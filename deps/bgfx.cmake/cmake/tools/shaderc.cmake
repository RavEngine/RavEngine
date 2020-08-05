# bgfx.cmake - bgfx building in cmake
# Written in 2017 by Joshua Brookover <joshua.al.brookover@gmail.com>

# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. This software is distributed without any warranty.

# You should have received a copy of the CC0 Public Domain Dedication along with
# this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

include( CMakeParseArguments )

include( cmake/3rdparty/fcpp.cmake )
include( cmake/3rdparty/glsl-optimizer.cmake )
include( cmake/3rdparty/glslang.cmake )
include( cmake/3rdparty/spirv-cross.cmake )
include( cmake/3rdparty/spirv-tools.cmake )
include( cmake/3rdparty/webgpu.cmake )

add_executable( shaderc ${BGFX_DIR}/tools/shaderc/shaderc.cpp ${BGFX_DIR}/tools/shaderc/shaderc.h ${BGFX_DIR}/tools/shaderc/shaderc_glsl.cpp ${BGFX_DIR}/tools/shaderc/shaderc_hlsl.cpp ${BGFX_DIR}/tools/shaderc/shaderc_pssl.cpp ${BGFX_DIR}/tools/shaderc/shaderc_spirv.cpp ${BGFX_DIR}/tools/shaderc/shaderc_metal.cpp )
target_compile_definitions( shaderc PRIVATE "-D_CRT_SECURE_NO_WARNINGS" )
set_target_properties( shaderc PROPERTIES FOLDER "bgfx/tools" )
target_link_libraries(shaderc PRIVATE bx bimg bgfx-vertexlayout bgfx-shader-spirv fcpp glsl-optimizer glslang spirv-cross spirv-tools webgpu)

if( BGFX_CUSTOM_TARGETS )
	add_dependencies( tools shaderc )
endif()

if (ANDROID)
    target_link_libraries(shaderc PRIVATE log)
elseif (IOS)
	set_target_properties(shaderc PROPERTIES MACOSX_BUNDLE ON
											 MACOSX_BUNDLE_GUI_IDENTIFIER shaderc)
endif()

function( add_shader ARG_FILE )
	# Parse arguments
	cmake_parse_arguments( ARG "FRAGMENT;VERTEX;COMPUTE" "OUTPUT;GLSL_VERSION;DX9_MODEL;DX11_MODEL" "PLATFORMS" ${ARGN} )

	# Get filename
	get_filename_component( FILENAME "${ARG_FILE}" NAME_WE )

	# Determine if fragment or vertex or compute
	if( ARG_FRAGMENT AND ARG_VERTEX AND ARG_COMPUTE )
		message( SEND_ERROR "add_shader cannot be called with all FRAGMENT and VERTEX and COMPUTE." )
		return()
	elseif( ARG_FRAGMENT AND ARG_VERTEX )
		message( SEND_ERROR "add_shader cannot be called with both FRAGMENT and VERTEX." )
		return()
	elseif( ARG_FRAGMENT AND ARG_COMPUTE )
		message( SEND_ERROR "add_shader cannot be called with both FRAGMENT and COMPUTE." )
		return()
	elseif( ARG_VERTEX AND ARG_COMPUTE )
		message( SEND_ERROR "add_shader cannot be called with both VERTEX and COMPUTE." )
		return()
	endif()

	if( ARG_FRAGMENT )
		set( TYPE "FRAGMENT" )
		set( D3D_PREFIX "ps" )
	elseif( ARG_VERTEX )
		set( TYPE "VERTEX" )
		set( D3D_PREFIX "vs" )
	elseif( ARG_COMPUTE )
		set( TYPE "COMPUTE" )
		set( D3D_PREFIX "cs" )
	else()
		message( SEND_ERROR "add_shader must be called with either FRAGMENT or VERTEX or COMPUTE." )
 		return()
	endif()

	# Determine compatible platforms
	if( ARG_PLATFORMS )
		set( PLATFORMS ${ARG_PLATFORMS} )
	else()
		if( MSVC )
			set( PLATFORMS dx9 dx11 glsl essl asm.js spirv )
		elseif( APPLE )
			set( PLATFORMS metal glsl essl asm.js spirv )
		else()
			set( PLATFORMS glsl essl asm.js spirv )
		endif()
	endif()

	# Build options
	set( BASE_OPTIONS
		FILE ${ARG_FILE}
		${TYPE}
		INCLUDES ${BGFX_DIR}/src
	)

	# Parse profiles
	set( DX9_PROFILE PROFILE ${D3D_PREFIX}_3_0 )
	if( ARG_DX9_MODEL )
		set( DX9_PROFILE PROFILE ${D3D_PREFIX}_${ARG_DX9_MODEL} )
	endif()
	set( DX11_PROFILE PROFILE ${D3D_PREFIX}_5_0 )
	if( ARG_DX11_MODEL )
		set( DX11_PROFILE PROFILE ${D3D_PREFIX}_${ARG_DX11_MODEL} )
	endif()
	set( GLSL_PROFILE PROFILE 120 )
	if( ARG_COMPUTE )
		set( GLSL_PROFILE PROFILE 430 )
	endif()
	if( ARG_GLSL_VERSION )
		set( GLSL_PROFILE PROFILE ${ARG_GLSL_VERSION} )
	endif()
	set( SPIRV_PROFILE PROFILE spirv )

	# Add commands
	set( OUTPUTS "" )
	set( COMMANDS "" )
	foreach( PLATFORM ${PLATFORMS} )
		set( OPTIONS ${BASE_OPTIONS} )
		set( OUTPUT "${ARG_OUTPUT}/${PLATFORM}/${FILENAME}.bin" )
		get_filename_component( OUTPUT "${OUTPUT}" ABSOLUTE )

		if( "${PLATFORM}" STREQUAL "dx9" )
			list( APPEND OPTIONS
				WINDOWS
				${DX9_PROFILE}
				OUTPUT ${OUTPUT}
			)
		elseif( "${PLATFORM}" STREQUAL "dx11" )
			list( APPEND OPTIONS
				WINDOWS
				${DX11_PROFILE}
				OUTPUT ${OUTPUT}
			)
		elseif( "${PLATFORM}" STREQUAL "metal" )
			list( APPEND OPTIONS
				OSX
				PROFILE metal
				OUTPUT ${OUTPUT}
			)
		elseif( "${PLATFORM}" STREQUAL "glsl" )
			list( APPEND OPTIONS
				LINUX
				${GLSL_PROFILE}
				OUTPUT ${OUTPUT}
			)
		elseif( "${PLATFORM}" STREQUAL "essl" )
			list( APPEND OPTIONS
				ANDROID
				OUTPUT ${OUTPUT}
			)
		elseif( "${PLATFORM}" STREQUAL "asm.js" )
			list( APPEND OPTIONS
				ASM_JS
				OUTPUT ${OUTPUT}
			)
		elseif( "${PLATFORM}" STREQUAL "spirv" )
			list( APPEND OPTIONS
				LINUX
				${SPIRV_PROFILE}
				OUTPUT ${OUTPUT}
			)
		else()
			message( SEND_ERROR "add_shader given bad platform: ${PLATFORM}" )
			return()
		endif()

		list( APPEND OUTPUTS ${OUTPUT} )
		shaderc_parse( CMD ${OPTIONS} )
		list( APPEND COMMANDS COMMAND "${CMAKE_COMMAND}" -E make_directory "${ARG_OUTPUT}/${PLATFORM}" )
		list( APPEND COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${CMD} )
	endforeach()

	# Add command
	add_custom_command(
		MAIN_DEPENDENCY
		${ARG_FILE}
		OUTPUT
		${OUTPUTS}
		${COMMANDS}
		COMMENT "Compiling shader ${ARG_FILE}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	# Add to custom filter
	source_group( "Shader Files" FILES ${ARG_FILE} )
endfunction()

# shaderc( FILE file OUTPUT file ... )
# See shaderc_parse() below for inputs
function( shaderc )
	cmake_parse_arguments( ARG "" "FILE;OUTPUT;LABEL" "" ${ARGN} )
	set( LABEL "" )
	if( ARG_LABEL )
		set( LABEL " (${ARG_LABEL})" )
	endif()
	shaderc_parse( CLI FILE ${ARG_FILE} OUTPUT ${ARG_OUTPUT} ${ARG_UNPARSED_ARGUMENTS} )
	get_filename_component( OUTDIR "${ARG_OUTPUT}" ABSOLUTE )
	get_filename_component( OUTDIR "${OUTDIR}" DIRECTORY )
	add_custom_command( OUTPUT ${ARG_OUTPUT}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTDIR}"
		COMMAND "$<TARGET_FILE:shaderc>" ${CLI}
		MAIN_DEPENDENCY ${ARG_FILE}
		COMMENT "Compiling shader ${ARG_FILE}${LABEL}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
endfunction()

# shaderc_parse(
#	FILE filename
#	OUTPUT filename
#	FRAGMENT|VERTEX|COMPUTE
#	ANDROID|ASM_JS|IOS|LINUX|NACL|OSX|WINDOWS
#	[PROFILE profile]
#	[O 0|1|2|3]
#	[VARYINGDEF filename]
#	[BIN2C filename]
#	[INCLUDES include;include]
#	[DEFINES include;include]
#	[DEPENDS]
#	[PREPROCESS]
#	[RAW]
#	[VERBOSE]
#	[DEBUG]
#	[DISASM]
#	[WERROR]
# )
function( shaderc_parse ARG_OUT )
	cmake_parse_arguments( ARG "DEPENDS;ANDROID;ASM_JS;IOS;LINUX;NACL;OSX;WINDOWS;PREPROCESS;RAW;FRAGMENT;VERTEX;COMPUTE;VERBOSE;DEBUG;DISASM;WERROR" "FILE;OUTPUT;VARYINGDEF;BIN2C;PROFILE;O" "INCLUDES;DEFINES" ${ARGN} )
	set( CLI "" )

	# -f
	if( ARG_FILE )
		list( APPEND CLI "-f" "${ARG_FILE}" )
	endif()

	# -i
	if( ARG_INCLUDES )
		foreach( INCLUDE ${ARG_INCLUDES} )
			list( APPEND CLI "-i" )
			list( APPEND CLI "${INCLUDE}" )			
		endforeach()
	endif()

	# -o
	if( ARG_OUTPUT )
		list( APPEND CLI "-o" "${ARG_OUTPUT}" )
	endif()

	# --bin2c
	if( ARG_BIN2C )
		list( APPEND CLI "--bin2c" "${ARG_BIN2C}" )
	endif()

	# --depends
	if( ARG_DEPENDS )
		list( APPEND CLI "--depends" )
	endif()

	# --platform
	set( PLATFORM "" )
	set( PLATFORMS "ANDROID;ASM_JS;IOS;LINUX;NACL;OSX;WINDOWS" )
	foreach( P ${PLATFORMS} )
		if( ARG_${P} )
			if( PLATFORM )
				message( SEND_ERROR "Call to shaderc_parse() cannot have both flags ${PLATFORM} and ${P}." )
				return()
			endif()
			set( PLATFORM "${P}" )
		endif()
	endforeach()
	if( "${PLATFORM}" STREQUAL "" )
		message( SEND_ERROR "Call to shaderc_parse() must have a platform flag: ${PLATFORMS}" )
		return()
	elseif( "${PLATFORM}" STREQUAL "ANDROID" )
		list( APPEND CLI "--platform" "android" )
	elseif( "${PLATFORM}" STREQUAL "ASM_JS" )
		list( APPEND CLI "--platform" "asm.js" )
	elseif( "${PLATFORM}" STREQUAL "IOS" )
		list( APPEND CLI "--platform" "ios" )
	elseif( "${PLATFORM}" STREQUAL "LINUX" )
		list( APPEND CLI "--platform" "linux" )
	elseif( "${PLATFORM}" STREQUAL "NACL" )
		list( APPEND CLI "--platform" "nacl" )
	elseif( "${PLATFORM}" STREQUAL "OSX" )
		list( APPEND CLI "--platform" "osx" )
	elseif( "${PLATFORM}" STREQUAL "WINDOWS" )
		list( APPEND CLI "--platform" "windows" )
	endif()

	# --preprocess
	if( ARG_PREPROCESS )
		list( APPEND CLI "--preprocess" )
	endif()

	# --define
	if( ARG_DEFINES )
		list( APPEND CLI "--defines" )
		set( DEFINES "" )
		foreach( DEFINE ${ARG_DEFINES} )
			if( NOT "${DEFINES}" STREQUAL "" )
				set( DEFINES "${DEFINES}\\\\;${DEFINE}" )
			else()
				set( DEFINES "${DEFINE}" )
			endif()
		endforeach()
		list( APPEND CLI "${DEFINES}" )
	endif()

	# --raw
	if( ARG_RAW )
		list( APPEND CLI "--raw" )
	endif()

	# --type
	set( TYPE "" )
	set( TYPES "FRAGMENT;VERTEX;COMPUTE" )
	foreach( T ${TYPES} )
		if( ARG_${T} )
			if( TYPE )
				message( SEND_ERROR "Call to shaderc_parse() cannot have both flags ${TYPE} and ${T}." )
				return()
			endif()
			set( TYPE "${T}" )
		endif()
	endforeach()
	if( "${TYPE}" STREQUAL "" )
		message( SEND_ERROR "Call to shaderc_parse() must have a type flag: ${TYPES}" )
		return()
	elseif( "${TYPE}" STREQUAL "FRAGMENT" )
		list( APPEND CLI "--type" "fragment" )
	elseif( "${TYPE}" STREQUAL "VERTEX" )
		list( APPEND CLI "--type" "vertex" )
	elseif( "${TYPE}" STREQUAL "COMPUTE" )
		list( APPEND CLI "--type" "compute" )
	endif()

	# --varyingdef
	if( ARG_VARYINGDEF )
		list( APPEND CLI "--varyingdef" "${ARG_VARYINGDEF}" )
	endif()

	# --verbose
	if( ARG_VERBOSE )
		list( APPEND CLI "--verbose" )
	endif()

	# --debug
	if( ARG_DEBUG )
		list( APPEND CLI "--debug" )
	endif()

	# --disasm
	if( ARG_DISASM )
		list( APPEND CLI "--disasm" )
	endif()

	# --profile
	if( ARG_PROFILE )
		list( APPEND CLI "--profile" "${ARG_PROFILE}" )
	endif()

	# -O
	if( ARG_O )
		list( APPEND CLI "-O" "${ARG_O}" )
	endif()

	# --Werror
	if( ARG_WERROR )
		list( APPEND CLI "--Werror" )
	endif()

	set( ${ARG_OUT} ${CLI} PARENT_SCOPE )
endfunction()
