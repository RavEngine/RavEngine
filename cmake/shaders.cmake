# globals for managing state
define_property(GLOBAL PROPERTY ALL_SHADERS
		BRIEF_DOCS "Aggregate shader list"
		FULL_DOCS "GLOBAL shader list"
	)
set_property(GLOBAL PROPERTY ALL_SHADERS "")
define_property(GLOBAL PROPERTY ALL_SHADER_SOURCES
	BRIEF_DOCS "Aggregate shader source list"
	FULL_DOCS "GLOBAL shader source list"
)
set_property(GLOBAL PROPERTY ALL_SHADER_SOURCES "")

define_property(GLOBAL PROPERTY ENG_DIR
	BRIEF_DOCS "Engine Directory"
	FULL_DOCS "Engine Directory"
)
set_property(GLOBAL PROPERTY ENG_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

define_property(GLOBAL PROPERTY COPY_DEPENDS
	BRIEF_DOCS "Engine Directory"
	FULL_DOCS "Engine Directory"
)

macro(shader_compile infile stage api extension binary)
	if (NOT RAVENGINE_SERVER)
		set(bindir "${shader_target}_ShaderIntermediate")
		set(shader_inc_dir "${eng_dir}/shaders")

		get_filename_component(name_only ${infile} NAME)
		set(outname "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${name_only}.${extension}")
		set(finalrootpath "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${name_only}")
		if (${api} MATCHES "Metal")
			STRING(REPLACE "." "_" name_fixed ${name_only} )
			set(entrypoint "--entrypoint" "${name_fixed}")
		endif()
		add_custom_command(
			PRE_BUILD
			OUTPUT "${outname}"
			DEPENDS ${infile} GNS_Deps "${eng_dir}/shaders/ravengine_shader.glsl" "${eng_dir}/shaders/ravengine_fsh.h" "${eng_dir}/shaders/ravengine_vsh.h" "${eng_dir}/shaders/ravengine_shader_defs.h" "${eng_dir}/shaders/lightingbindings_shared.h" "${eng_dir}/shaders/BRDF.glsl" "${eng_dir}/shaders/lighting_preamble.glsl"
			COMMAND ${rglc_path} -f "${infile}" -o "${outname}" --api ${api} --stage ${stage} --include "${shader_inc_dir}" --debug ${binary} ${entrypoint}
		)
	endif()
endmacro()

function(rvesc_compile descfile shader_target)

	add_custom_command(
		PRE_BUILD
		OUTPUT "test"
		COMMAND ${RVESC_PATH}
	)

endfunction()

function(declare_shader infile shader_target)
	if (RAVENGINE_SERVER)
		return()
	endif()

	get_filename_component(shader_ext ${infile} EXT)
	
	if (shader_ext MATCHES "json")
		rvesc_compile(infile shader_target)
		return()
	endif()

	if (shader_ext MATCHES "vsh")
		set(stage "vertex")
		set(dxc_profile "v")
	elseif (shader_ext MATCHES "fsh")
		set(stage "fragment")
		set(dxc_profile "p")
	elseif (shader_ext MATCHES "csh")
		set(stage "compute")
		set(dxc_profile "c")
	endif()

	if(RGL_VK_AVAILABLE)
		shader_compile("${infile}" "${stage}" "Vulkan" "spv" "")
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${outname})
	endif()
	if (RGL_WEBGPU_AVAILABLE)
		shader_compile("${infile}" "${stage}" "WebGPU" "wgsl" "")
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${outname})
	endif()
	if(RGL_DX12_AVAILABLE)
		shader_compile("${infile}" "${stage}" "Direct3D12" "hlsl" "")

		if (stage MATCHES "vertex")
			set(dxc_type "Vertex")
		elseif (stage MATCHES "fragment")
			set(dxc_type "Pixel")
		elseif (stage MATCHES "compute")
			set(dxc_type "Compute")
		endif()
		target_sources(${shader_target} PUBLIC ${outname})
		source_group("Generated" FILES "${outname}")
		set(dx_final_name "${finalrootpath}.cso")
		set_source_files_properties(${outname} PROPERTIES 
			GENERATED TRUE
			HEADER_FILE_ONLY ON
			VS_SHADER_MODEL "6.4"
			VS_SHADER_TYPE ${dxc_type}
			VS_SHADER_ENABLE_DEBUG ON
			VS_SHADER_DISABLE_OPTIMIZATIONS ON
			# VS_SHADER_FLAGS "/Fd ${CMAKE_BINARY_DIR}/$<CONFIG>/"
			VS_SHADER_OBJECT_FILE_NAME ${dx_final_name}
		)
		set_source_files_properties(dx_final_name PROPERTIES GENERATED TRUE)
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${dx_final_name})
		# manually invoke dxc.exe
		add_custom_command(
			PRE_BUILD 
			OUTPUT "${dx_final_name}"
			DEPENDS "${outname}"
			COMMAND dxc.exe $<$<CONFIG:Debug>:-Qembed_debug> $<$<CONFIG:Debug>:-Zi> $<$<CONFIG:Debug>:-Od> -Fo \"${dx_final_name}\" -T ${dxc_profile}s_6_4 \"${outname}\"
		)
	endif()
	if(RGL_MTL_AVAILABLE)
		shader_compile("${infile}" "${stage}" "Metal" "metal" "")
		set_source_files_properties(${outname} PROPERTIES 
			GENERATED TRUE
			HEADER_FILE_ONLY OFF
			LANGUAGE METAL
			XCODE_EXPLICIT_FILE_TYPE "sourcecode.metal"
            COMPILE_FLAGS "-gline-tables-only -frecord-sources" # enable shader source debugging
		)
			# goes into the bundle Metallib, so we don't need to include the shader in the asset package
		target_sources(${shader_target} PUBLIC ${outname})
		source_group("Generated" FILES "${outname}")
	endif()
endfunction()