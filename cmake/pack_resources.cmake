# pack resources
function(pack_resources)
	set(optional )
	set(args TARGET OUTPUT_FILE STREAMING_INPUT_ROOT)
	set(list_args SHADERS MESHES OBJECTS SKELETONS ANIMATIONS TEXTURES UIS FONTS SOUNDS STREAMING_ASSETS)
	cmake_parse_arguments(
		PARSE_ARGV 0
		ARGS
		"${optional}"
		"${args}"
		"${list_args}"
	)

	if(${ARGS_UNPARSED_ARGUMENTS})
		message(WARNING "Unparsed arguments: ${ARGS_UNPARSED_ARGUMENTS}")
	endif()

	if(APPLE)
		set_property(TARGET ${ARGS_TARGET} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
		set_target_properties(${ARGS_TARGET} PROPERTIES 
			XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "$<$<NOT:$<CONFIG:RELEASE>>:INCLUDE_SOURCE>"	# enable shader debugging in all but release mode
			XCODE_ATTRIBUTE_MTL_OPTIMIZATION_LEVEL "$<$<CONFIG:RELEASE>:default>"				# optimize shaders in release builds
			XCODE_ATTRIBUTE_MTL_LANGUAGE_REVISION "Metal30"										# use Metal 3
			)
	endif()	

	get_property(eng_dir GLOBAL PROPERTY ENG_DIR)

	# add polygon primitives provided by engine
	file(GLOB ENG_MESHES "${eng_dir}/meshes/*.json")

	# add engine-provided shaders
	file(GLOB ENG_SHADERS "${eng_dir}/shaders/*.vsh" "${eng_dir}/shaders/*.fsh" "${eng_dir}/shaders/*.csh" "${eng_dir}/materials/*.json")

	# add engine-provided fonts
	file(GLOB ENG_FONTS "${eng_dir}/fonts/*.ttf")

	file(GLOB ENG_UIS "${eng_dir}/ui/*.rcss" "${eng_dir}/ui/*.rml")

	# clear copy-depends
	set_property(GLOBAL PROPERTY COPY_DEPENDS "")

	function(copy_helper_impl FILE_LIST output_dir root_dir)
		foreach(FILE ${FILE_LIST})
			# copy objects pre-build if they are changed
			get_filename_component(output_name "${FILE}" NAME)
			set(outname "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}/${root_dir}/${output_name}")
			add_custom_command(PRE_BUILD 
				OUTPUT "${outname}" 
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
				DEPENDS ${FILE}
				)
			set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS ${outname})
		endforeach()
	endfunction()

	# helper for copying to staging directory
	function(copy_helper FILE_LIST output_dir)
		copy_helper_impl("${FILE_LIST}" ${ARGS_TARGET} "${output_dir}")
	endfunction()

	function(copy_streaming_helper FILE_LIST output_root input_root)
		foreach(FILE ${FILE_LIST})
			if (APPLE)
				# put it in the resources bundle
				file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
				get_filename_component(relpath "${relpath}" DIRECTORY)
				target_sources(${ARGS_TARGET} PRIVATE ${FILE})
				set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${ARGS_TARGET}_Streaming/${relpath}")
			elseif(NOT EMSCRIPTEN)
				# put it next to the executable
				file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
				if (WINDOWS_STORE)
					set(UWP_APPX "AppX")
				endif()
				set(asset_relpath "${ARGS_TARGET}_Streaming/${output_root}/${relpath}")
				set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${UWP_APPX}/${asset_relpath}")
				add_custom_command(PRE_BUILD 
					OUTPUT "${outname}" 
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
					DEPENDS ${FILE}
				)
				target_sources("${ARGS_TARGET}" PRIVATE "${FILE}")
				source_group("Streaming Assets" FILES ${FILE})
				set_source_files_properties(${FILE} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "archive.ar")
				set_property(SOURCE "${FILE}" PROPERTY VS_DEPLOYMENT_CONTENT 1)
				set_property(SOURCE "${FILE}" PROPERTY VS_DEPLOYMENT_LOCATION "${asset_relpath}")
				set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS ${outname})
			endif()
		endforeach()
	endfunction()

	copy_helper("${ARGS_OBJECTS}" "objects")
	copy_helper("${ARGS_TEXTURES}" "textures")
	copy_helper("${ARGS_UIS}" "uis")
	copy_helper("${ENG_UIS}" "uis")
	copy_helper("${ARGS_FONTS}" "fonts")
	copy_helper("${ENG_FONTS}" "fonts")
	copy_helper("${ARGS_SOUNDS}" "sounds")
	
	copy_streaming_helper("${ARGS_STREAMING_ASSETS}" "" "${ARGS_STREAMING_INPUT_ROOT}")

	target_sources(${ARGS_TARGET} PUBLIC ${ARGS_OBJECTS} ${ARGS_TEXTURES} ${ARGS_SOUNDS} ${ARGS_MESHES})
	set_source_files_properties(${ARGS_OBJECTS} ${ARGS_TEXTURES} ${ARGS_SOUNDS} PROPERTIES HEADER_FILE_ONLY ON)
	
	source_group("Objects" FILES ${ARGS_OBJECTS})
	source_group("Textures" FILES ${ARGS_TEXTURES})
	source_group("Sounds" FILES ${ARGS_SOUNDS})
	source_group("UI" FILES ${ARGS_UIS})
	source_group("Streaming" FILES ${ARGS_STREAMING_ASSETS})
	source_group("Meshes" FILES ${ARGS_MESHES})

	# import Meshes
	foreach(MESHCONF ${ARGS_MESHES} ${ENG_MESHES})
		file(READ "${MESHCONF}" desc_STR)
		string(JSON inmeshfile GET "${desc_STR}" file)
		
		set(outdir "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}/meshes/")
		get_filename_component(outname "${MESHCONF}" NAME_WE)
		get_filename_component(indir "${MESHCONF}" DIRECTORY)
		set(outfilename "${outdir}/${outname}.rvem")
		add_custom_command(PRE_BUILD 
			OUTPUT "${outfilename}"
			COMMAND ${RVEMC_PATH} -f "${MESHCONF}" -o "${outdir}"
			DEPENDS "${MESHCONF}" "${indir}/${inmeshfile}" "${RVEMC_PATH}"
			COMMENT "Importing Mesh ${MESHCONF}"
		)
		set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS "${outfilename}")
		target_sources(${ARGS_TARGET} PUBLIC "${indir}/${inmeshfile}")
		source_group("Meshes" FILES  "${indir}/${inmeshfile}")
		set_source_files_properties("${indir}/${inmeshfile}" PROPERTIES XCODE_EXPLICIT_FILE_TYPE "archive.ar")
	endforeach()

	# import Skeletons
	foreach(SKELCONF ${ARGS_SKELETONS})
		file(READ "${SKELCONF}" desc_STR)
		string(JSON inmeshfile GET "${desc_STR}" file)
		
		set(outdir "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}/skeletons/")
		get_filename_component(outname "${SKELCONF}" NAME_WE)
		get_filename_component(indir "${SKELCONF}" DIRECTORY)
		set(outfilename "${outdir}/${outname}.rves")
		add_custom_command(PRE_BUILD 
			OUTPUT "${outfilename}"
			COMMAND ${RVESKC_PATH} -f "${SKELCONF}" -o "${outdir}"
			DEPENDS "${SKELCONF}" "${indir}/${inmeshfile}" "${RVESKC_PATH}"
			COMMENT "Importing Skeleton ${SKELCONF}"
		)
		set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS "${outfilename}")
	endforeach()

	# import Animations
	foreach(ANIMCONF ${ARGS_ANIMATIONS})
		file(READ "${ANIMCONF}" desc_STR)
		string(JSON inmeshfile GET "${desc_STR}" file)
		
		set(outdir "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}/animations/")
		get_filename_component(outname "${ANIMCONF}" NAME_WE)
		get_filename_component(indir "${ANIMCONF}" DIRECTORY)
		set(outfilename "${outdir}/${outname}.rvea")
		add_custom_command(PRE_BUILD 
			OUTPUT "${outfilename}"
			COMMAND ${RVEAC_PATH} -f "${ANIMCONF}" -o "${outdir}"
			DEPENDS "${ANIMCONF}" "${indir}/${inmeshfile}" "${RVEAC_PATH}"
			COMMENT "Importing Animation ${ANIMCONF}"
		)
		set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS "${outfilename}")
	endforeach()

	# get dependency outputs
	get_property(copy_depends GLOBAL PROPERTY COPY_DEPENDS)

	# clear global shaders property
	set_property(GLOBAL PROPERTY ALL_SHADERS "")

	# setup shader compiler
	foreach(SHADER ${ENG_SHADERS} ${ARGS_SHADERS})
		declare_shader("${SHADER}" "${ARGS_TARGET}")
	endforeach()

	set_property(GLOBAL PROPERTY ALL_SHADER_SOURCES "")
	foreach(SHADER ${ARGS_SHADERS})
		list(APPEND all_shader_sources "${SHADER}")
	endforeach()

	get_property(sc_comp_name GLOBAL PROPERTY SC_COMP_NAME)
	get_property(sc_include_dir GLOBAL PROPERTY SC_INCLUDE_DIR)

	#track all the shaders for compilation
	get_property(all_shaders_property GLOBAL PROPERTY ALL_SHADERS)
	add_custom_target("${ARGS_TARGET}_CompileShaders" ALL DEPENDS ${all_shaders_property})
	add_dependencies("${ARGS_TARGET}" "${ARGS_TARGET}_CompileShaders" "RavEngine")

	if(NOT APPLE)
		set(rve_cmrc_resource_name "${ARGS_TARGET}_ShaderResources")
		cmrc_add_resource_library("${rve_cmrc_resource_name}" WHENCE "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}_ShaderIntermediate/" ${all_shaders_property} )
		set_target_properties("${rve_cmrc_resource_name}" PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
		)
		target_link_libraries("${ARGS_TARGET}" PRIVATE "${rve_cmrc_resource_name}")
		set(cmrc_shader_libfn_cpp "${CMAKE_BINARY_DIR}/__cmrc_${rve_cmrc_resource_name}/lib_extern.cpp")
		file(GENERATE OUTPUT "${cmrc_shader_libfn_cpp}" CONTENT "
		#include <cmrc/cmrc.hpp>
		#include <span>
		#include <string_view>
		#include <string>
		CMRC_DECLARE(${rve_cmrc_resource_name});

		#define CMRC_NS cmrc::${rve_cmrc_resource_name}

		const std::span<const char> cmrc_get_file_data(const std::string_view& path){
			auto fs = CMRC_NS::get_filesystem();

			auto file = fs.open(std::string(path));


			return {file.begin(),file.size()};
		}
		")
		target_sources(${rve_cmrc_resource_name} PRIVATE "${cmrc_shader_libfn_cpp}")
		target_compile_features(${rve_cmrc_resource_name} PRIVATE cxx_std_20)
	endif()

	set(rve_externs_cpp "${CMAKE_BINARY_DIR}/${ARGS_TARGET}_ShaderIntermediate/externs.cpp")
	file(GENERATE OUTPUT "${rve_externs_cpp}" CONTENT "
	#include <string_view>
	const std::string_view RVE_VFS_get_name(){
		return \"${ARGS_TARGET}\";
	}
	")
	set(rve_externs_lib "${ARGS_TARGET}_Externs")
	add_library(${rve_externs_lib} "${rve_externs_cpp}")
	set_target_properties("${rve_externs_lib}" PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
	)
	target_compile_features(${rve_externs_lib} PRIVATE cxx_std_20)
	target_link_libraries("${ARGS_TARGET}" PRIVATE "${rve_externs_lib}")

	set_target_properties(${rve_cmrc_resource_name} "${ARGS_TARGET}_CompileShaders" ${rve_externs_lib} PROPERTIES
		FOLDER "RavEngine Auxillary"
	)

	# add files to IDE sidebar for convenience
	target_sources("${ARGS_TARGET}" PUBLIC ${ARGS_UIS} ${all_shader_sources})
	set_source_files_properties(${ARGS_UIS} ${all_shader_sources} PROPERTIES HEADER_FILE_ONLY TRUE)	# prevents visual studio from trying to build these
	set_source_files_properties(${all_shader_sources} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "sourcecode.glsl")
	set_source_files_properties(${ARGS_UIS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "text.html")
	set_source_files_properties(${ARGS_OBJECTS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "compiled")
	set_source_files_properties(${ARGS_SOUNDS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "audio.wav")
	source_group("Shaders" FILES ${all_shader_sources})

	set(outpack "${CMAKE_BINARY_DIR}/${ARGS_TARGET}.rvedata")

	# allow inserting into the mac / ios resource bundle
	set_target_properties(${ARGS_TARGET} PROPERTIES 
		MACOSX_BUNDLE TRUE
		#XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>,$<CONFIG:PROFILE>>:YES>
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
		XCODE_GENERATE_SCHEME ON	# create a scheme in Xcode
	)

	set(assets ${ARGS_OBJECTS} ${ARGS_TEXTURES} ${copy_depends})

	# the command to pack into a zip
	add_custom_command(
		POST_BUILD 
		OUTPUT "${outpack}"
		DEPENDS ${assets}
		COMMENT "Packing resources for ${ARGS_TARGET}"
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${outpack}" --format=zip "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}"
		VERBATIM
	)

	# make part of the target, and add to the resources folder if applicable
	target_sources("${ARGS_TARGET}" PRIVATE "${outpack}")
	set_source_files_properties("${outpack}" PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources
	)
	source_group("Resources" FILES ${outpack})
	set_source_files_properties(${outpack} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "archive.ar")

	# Set the assets zip location on UWP
	set_property(SOURCE "${outpack}" PROPERTY VS_DEPLOYMENT_CONTENT 1)
	set_property(SOURCE "${outpack}" PROPERTY VS_DEPLOYMENT_LOCATION "")	# tells the deployment to put the assets zip in the AppX root directory
	
	# copy to target dir on Win
	if((WIN32 AND NOT WINDOWS_STORE) OR LINUX)
		get_filename_component(outfile ${outpack} NAME)
		SET(outfile "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${outfile}")
		add_custom_command(
			POST_BUILD
			TARGET "${ARGS_TARGET}"
			DEPENDS "${outpack}"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${outpack}" "${outfile}"
			COMMENT "Copying assets package to executable directory"
		)
	endif()

	set(${ARGS_OUTPUT_FILE} ${outpack} CACHE INTERNAL "")
endfunction()