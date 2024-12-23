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
		STRING(REPLACE "." "_" name_only ${name_only} )
		set(outname "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${name_only}.${extension}")
		set(finalrootpath "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${name_only}")
		if (${api} MATCHES "Metal")
			set(entrypoint "--entrypoint" "${name_only}")
		endif()
		add_custom_command(
			PRE_BUILD
			OUTPUT "${outname}"
			DEPENDS ${infile} GNS_Deps "${eng_dir}/shaders/ravengine_shader.glsl" "${eng_dir}/shaders/ravengine_shader_defs.h" "${eng_dir}/shaders/BRDF.glsl"
			COMMAND ${rglc_path} -f "${infile}" -o "${outname}" --api ${api} --stage ${stage} --include "${shader_inc_dir}" --debug ${binary} ${entrypoint}
		)
	endif()
endmacro()

macro(rvesc_compile_shader infile shaderfile api extension outputfilename extraflags)
	set(shader_inc_dir "${eng_dir}/shaders")	
	set(outname "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${outputfilename}.${extension}")
	
	get_filename_component(desc_dir "${infile}" DIRECTORY )
	
	set(shaderfilepath  "${desc_dir}/${shaderfile}")

	separate_arguments(sh_extraflags_sep UNIX_COMMAND ${extraflags})
	
	add_custom_command(
		PRE_BUILD
		OUTPUT "${outname}"
		DEPENDS "${infile}" GNS_Deps "${RVESC_PATH}" "${shaderfilepath}"
		COMMAND ${RVESC_PATH} -f "${infile}" -o "${outname}" --api ${api} --include "${shader_inc_dir}" ${sh_extraflags_sep} --debug 
	)

endmacro()

function(rvesc_compile descfile shader_target inshadername outputfilename extraflags)

	set(bindir "${shader_target}_ShaderIntermediate")	
		
	if(RGL_VK_AVAILABLE)
		rvesc_compile_shader("${descfile}" "${inshadername}" "Vulkan" "spv" "${outputfilename}" ${extraflags})
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${outname})
	endif()
	if (RGL_WEBGPU_AVAILABLE)
		rvesc_compile_shader("${descfile}" "${inshadername}" "WebGPU" "wgsl" "${outputfilename}" ${extraflags})
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${outname})
	endif()
	if(RGL_DX12_AVAILABLE)
		rvesc_compile_shader("${descfile}" "${inshadername}" "Direct3D12" "hlsl" "${outputfilename}" ${extraflags})
		string(JSON stage GET "${desc_STR}" stage)
		
		set(finalrootpath "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/Direct3D12/${outputfilename}")
		
		if (stage MATCHES "vertex")
			set(dxc_type "Vertex")
			set(dxc_profile "v")
		elseif (stage MATCHES "fragment")
			set(dxc_type "Pixel")
			set(dxc_profile "p")
		elseif (stage MATCHES "compute")
			set(dxc_type "Compute")
			set(dxc_profile "c")
		endif()
		target_sources(${shader_target} PUBLIC "${outname}")
		source_group("Generated" FILES "${outname}")
		set(dx_final_name "${finalrootpath}.cso")
		set_source_files_properties(${outname} PROPERTIES 
			GENERATED TRUE
			HEADER_FILE_ONLY ON
			VS_SHADER_MODEL "6.8"
		)
		set_source_files_properties(dx_final_name PROPERTIES GENERATED TRUE)
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${dx_final_name})
		# manually invoke dxc.exe
		add_custom_command(
			PRE_BUILD 
			OUTPUT "${dx_final_name}"
			DEPENDS "${outname}"
			COMMAND ${ST_DXC_EXE_PATH} $<$<CONFIG:Debug>:-Qembed_debug> $<$<CONFIG:Debug>:-Zi> $<$<CONFIG:Debug>:-Od> /enable-16bit-types -Fo \"${dx_final_name}\" -T ${dxc_profile}s_6_8 \"${outname}\"
		)
	endif()
	
	if(RGL_MTL_AVAILABLE)
		rvesc_compile_shader("${descfile}" "${inshadername}" "Metal" "metal" "${outputfilename}" ${extraflags})
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
	
	# add to the IDE project
	target_sources(${shader_target} PUBLIC "${shaderfilepath}")
	source_group("Shaders" FILES "${shaderfilepath}")
	set_source_files_properties("${shaderfilepath}" PROPERTIES 
		HEADER_FILE_ONLY ON
		XCODE_EXPLICIT_FILE_TYPE "sourcecode.glsl"
	)

endfunction()

function(rvesc_compile_meta infile shader_target)
	file(READ "${infile}" desc_STR)
	string(JSON inshadername GET "${desc_STR}" shader)

	get_filename_component(sh_name_only ${infile} NAME_WE)
	rvesc_compile("${infile}" "${shader_target}" "${inshadername}" "${sh_name_only}" " ")

	# extra cases
	# Depth-only variants: If type is mesh, and not transparent, and is a fragment shader
	string(JSON instage GET "${desc_STR}" stage)
	string(JSON intype GET "${desc_STR}" type)
	string(JSON inopacity ERROR_VARIABLE nofound GET "${desc_STR}" opacity)


	if (instage MATCHES "fragment" AND (intype MATCHES "mesh" OR intype MATCHES "quad") AND NOT (inopacity MATCHES "transparent"))
		rvesc_compile("${infile}" "${shader_target}" "${inshadername}" "${sh_name_only}_depthonly" "--define \"RVE_DEPTHONLY 1\"")
	endif()

endfunction()

function(declare_shader infile shader_target)
	if (RAVENGINE_SERVER)
		return()
	endif()

	get_filename_component(shader_ext ${infile} EXT)
	
	if (shader_ext MATCHES "json")
		rvesc_compile_meta("${infile}" ${shader_target})
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
			COMMAND ${ST_DXC_EXE_PATH} $<$<CONFIG:Debug>:-Qembed_debug> $<$<CONFIG:Debug>:-Zi> $<$<CONFIG:Debug>:-Od> /enable-16bit-types -Fo \"${dx_final_name}\" -T ${dxc_profile}s_6_8 \"${outname}\"
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