cmake_minimum_required(VERSION 3.13)

project(GameNetworkingSocketsFull C CXX)

# dependencies
#add_subdirectory(libsodium-cmake)
add_subdirectory(openssl-cmake)
set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
set(protobuf_MODULE_COMPATIBLE ON CACHE INTERNAL "")
add_subdirectory(protobuf/cmake)

option(LTO "Enable Link-Time Optimization" ON)
option(USE_STEAMWEBRTC "Build Google's WebRTC library to get ICE support for P2P" OFF)
option(Protobuf_USE_STATIC_LIBS "Link with protobuf statically" ON)
option(LIGHT_TESTS "Use smaller/shorter tests for simple integration testing (e.g. Travis)" OFF)
option(GAMENETWORKINGSOCKETS_BUILD_EXAMPLES "Build the included example chat program" OFF)
option(GAMENETWORKINGSOCKETS_BUILD_TESTS "Build crypto, pki and network connection tests" OFF)

set(USE_CRYPTO "OpenSSL" CACHE INTERNAL "")
set(USE_CRYPTO25519 "OpenSSL" CACHE INTERNAL "")

find_package(Threads)

add_definitions( -DVALVE_CRYPTO_ENABLE_25519 )
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
		-D_CRT_NONSTDC_NO_WARNINGS
		)
endif()

# Ported from protobuf for protobuf_generate_cpp
function(protobuf_generate)
  include(CMakeParseArguments)

  set(_options APPEND_PATH)
  set(_singleargs LANGUAGE OUT_VAR EXPORT_MACRO PROTOC_OUT_DIR PLUGIN)
  if(COMMAND target_sources)
    list(APPEND _singleargs TARGET)
  endif()
  set(_multiargs PROTOS IMPORT_DIRS GENERATE_EXTENSIONS)

  cmake_parse_arguments(protobuf_generate "${_options}" "${_singleargs}" "${_multiargs}" "${ARGN}")

  if(NOT protobuf_generate_PROTOS AND NOT protobuf_generate_TARGET)
    message(SEND_ERROR "Error: protobuf_generate called without any targets or source files")
    return()
  endif()

  if(NOT protobuf_generate_OUT_VAR AND NOT protobuf_generate_TARGET)
    message(SEND_ERROR "Error: protobuf_generate called without a target or output variable")
    return()
  endif()

  if(NOT protobuf_generate_LANGUAGE)
    set(protobuf_generate_LANGUAGE cpp)
  endif()
  string(TOLOWER ${protobuf_generate_LANGUAGE} protobuf_generate_LANGUAGE)

  if(NOT protobuf_generate_PROTOC_OUT_DIR)
    set(protobuf_generate_PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if(protobuf_generate_EXPORT_MACRO AND protobuf_generate_LANGUAGE STREQUAL cpp)
    set(_dll_export_decl "dllexport_decl=${protobuf_generate_EXPORT_MACRO}:")
  endif()
  
  if(protobuf_generate_PLUGIN)
      set(_plugin "--plugin=${protobuf_generate_PLUGIN}")
  endif()

  if(NOT protobuf_generate_GENERATE_EXTENSIONS)
    if(protobuf_generate_LANGUAGE STREQUAL cpp)
      set(protobuf_generate_GENERATE_EXTENSIONS .pb.h .pb.cc)
    elseif(protobuf_generate_LANGUAGE STREQUAL python)
      set(protobuf_generate_GENERATE_EXTENSIONS _pb2.py)
    else()
      message(SEND_ERROR "Error: protobuf_generate given unknown Language ${LANGUAGE}, please provide a value for GENERATE_EXTENSIONS")
      return()
    endif()
  endif()

  if(protobuf_generate_TARGET)
    get_target_property(_source_list ${protobuf_generate_TARGET} SOURCES)
    foreach(_file ${_source_list})
      if(_file MATCHES "proto$")
        list(APPEND protobuf_generate_PROTOS ${_file})
      endif()
    endforeach()
  endif()

  if(NOT protobuf_generate_PROTOS)
    message(SEND_ERROR "Error: protobuf_generate could not find any .proto files")
    return()
  endif()

  #workaround
  set(protobuf_generate_APPEND_PATH ON CACHE INTERNAL "")

  if(protobuf_generate_APPEND_PATH)
    # Create an include path for each file specified
    foreach(_file ${protobuf_generate_PROTOS})
      get_filename_component(_abs_file ${_file} ABSOLUTE)
      get_filename_component(_abs_path ${_abs_file} PATH)
      list(FIND _protobuf_include_path ${_abs_path} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${_abs_path})
      endif()
    endforeach()
  endif()

  foreach(DIR ${protobuf_generate_IMPORT_DIRS})
    get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
    list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
    if(${_contains_already} EQUAL -1)
        list(APPEND _protobuf_include_path -I ${ABS_PATH})
    endif()
  endforeach()

  if(NOT _protobuf_include_path)
    set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  set(_generated_srcs_all)
  foreach(_proto ${protobuf_generate_PROTOS})
    get_filename_component(_abs_file ${_proto} ABSOLUTE)
    get_filename_component(_abs_dir ${_abs_file} DIRECTORY)

    get_filename_component(_file_full_name ${_proto} NAME)
    string(FIND "${_file_full_name}" "." _file_last_ext_pos REVERSE)
    string(SUBSTRING "${_file_full_name}" 0 ${_file_last_ext_pos} _basename)

    set(_suitable_include_found FALSE)
    foreach(DIR ${_protobuf_include_path})
      if(NOT DIR STREQUAL "-I")
        file(RELATIVE_PATH _rel_dir ${DIR} ${_abs_dir})
        string(FIND "${_rel_dir}" "../" _is_in_parent_folder)
        if (NOT ${_is_in_parent_folder} EQUAL 0)
          set(_suitable_include_found TRUE)
          break()
        endif()
      endif()
    endforeach()

    if(NOT _suitable_include_found)
      message(SEND_ERROR "Error: protobuf_generate could not find any correct proto include directory.")
      return()
    endif()

    set(_generated_srcs)
    foreach(_ext ${protobuf_generate_GENERATE_EXTENSIONS})
      list(APPEND _generated_srcs "${protobuf_generate_PROTOC_OUT_DIR}/${_rel_dir}/${_basename}${_ext}")
    endforeach()
    list(APPEND _generated_srcs_all ${_generated_srcs})

    add_custom_command(
      OUTPUT ${_generated_srcs}
      COMMAND  protobuf::protoc
      ARGS --${protobuf_generate_LANGUAGE}_out ${_dll_export_decl}${protobuf_generate_PROTOC_OUT_DIR} ${_plugin} ${_protobuf_include_path} ${_abs_file}
      DEPENDS ${_abs_file} protobuf::protoc
      COMMENT "Running ${protobuf_generate_LANGUAGE} protocol buffer compiler on ${_proto}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${_generated_srcs_all} PROPERTIES GENERATED TRUE)
  if(protobuf_generate_OUT_VAR)
    set(${protobuf_generate_OUT_VAR} ${_generated_srcs_all} PARENT_SCOPE)
  endif()
  if(protobuf_generate_TARGET)
    target_sources(${protobuf_generate_TARGET} PRIVATE ${_generated_srcs_all})
  endif()

endfunction()

# Ported from protobuf for GameNetworkingSockets
function(PROTOBUF_GENERATE_CPP SRCS HDRS)
  cmake_parse_arguments(protobuf_generate_cpp "" "EXPORT_MACRO" "" ${ARGN})

  set(_proto_files "${protobuf_generate_cpp_UNPARSED_ARGUMENTS}")
  if(NOT _proto_files)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif()

  if(PROTOBUF_GENERATE_CPP_APPEND_PATH)
    set(_append_arg APPEND_PATH)
  endif()

  if(DEFINED Protobuf_IMPORT_DIRS)
    set(_import_arg IMPORT_DIRS ${Protobuf_IMPORT_DIRS})
  endif()

  set(_outvar)
  protobuf_generate(${_append_arg} LANGUAGE cpp EXPORT_MACRO ${protobuf_generate_cpp_EXPORT_MACRO} OUT_VAR _outvar ${_import_arg} PROTOS ${_proto_files})

  set(${SRCS})
  set(${HDRS})
  foreach(_file ${_outvar})
    if(_file MATCHES "cc$")
      list(APPEND ${SRCS} ${_file})
    else()
      list(APPEND ${HDRS} ${_file})
    endif()
  endforeach()
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()


function(set_target_common_gns_properties TGT)
	target_compile_definitions( ${TGT} PRIVATE GOOGLE_PROTOBUF_NO_RTTI )

	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		# Reduce binary size by allowing for a pseudo-"function-level linking" analog
		target_compile_options(${TGT} PRIVATE -ffunction-sections -fdata-sections ${TARGET_ARCH_FLAGS})
	endif()

	if(CMAKE_SYSTEM_NAME MATCHES Linux)
		target_compile_definitions(${TGT} PUBLIC POSIX LINUX)
	elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
		target_compile_definitions(${TGT} PUBLIC POSIX OSX)
	elseif(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
		target_compile_definitions(${TGT} PUBLIC POSIX FREEBSD)
	elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
		target_compile_definitions(${TGT} PUBLIC _WINDOWS)
		if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(NOT Protobuf_USE_STATIC_LIBS)
				target_compile_definitions(${TGT} PRIVATE PROTOBUF_USE_DLLS)
			endif()
			target_compile_options(${TGT} PRIVATE
				/EHs-c-   # Disable C++ exceptions

				# Below are warnings we can't fix and don't want to see (mostly from protobuf, some from MSVC standard library)
				/wd4146   # include/google/protobuf/wire_format_lite.h(863): warning C4146: unary minus operator applied to unsigned type, result still unsigned
				/wd4530   # .../xlocale(319): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
				/wd4244   # google/protobuf/wire_format_lite.h(935): warning C4244: 'argument': conversion from 'google::protobuf::uint64' to 'google::protobuf::uint32', possible loss of data
				/wd4251   # 'google::protobuf::io::CodedOutputStream::default_serialization_deterministic_': struct 'std::atomic<bool>' needs to have dll-interface to be used by clients of class 
				/wd4267   # google/protobuf/has_bits.h(73): warning C4267: 'argument': conversion from 'size_t' to 'int', possible loss of data
				)

			# Disable RTTI except in Debug, because we use dynamic_cast in assert_cast
			target_compile_options(${TGT} PRIVATE $<IF:$<CONFIG:Debug>,/GR,/GR->)
		else()
			target_compile_definitions(${TGT} PRIVATE
				__STDC_FORMAT_MACROS=1
				__USE_MINGW_ANSI_STDIO=0
				)
			target_compile_options(${TGT} PRIVATE -fno-stack-protector)
		endif()
	else()
		message(FATAL_ERROR "Could not identify your target operating system")
	endif()

	if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
		target_compile_options(${TGT} PRIVATE -fstack-protector-strong)
	endif()

	if(LTO)
		set_target_properties(${TGT} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()

	set_target_properties(${TGT} PROPERTIES
		CXX_STANDARD 11
	)
endfunction()

set(GNS_PROTOS
	"GameNetworkingSockets/src/common/steamnetworkingsockets_messages_certs.proto"
	"GameNetworkingSockets/src/common/steamnetworkingsockets_messages.proto"
	"GameNetworkingSockets/src/common/steamnetworkingsockets_messages_udp.proto"
	)

set(GNS_SRCS
	"GameNetworkingSockets/src/common/crypto.cpp"
	"GameNetworkingSockets/src/common/crypto_textencode.cpp"
	"GameNetworkingSockets/src/common/keypair.cpp"
	"GameNetworkingSockets/src/common/steamid.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/csteamnetworkingsockets.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/csteamnetworkingmessages.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_flat.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_connections.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_lowlevel.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_p2p.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_p2p_ice.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_snp.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_udp.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/steamnetworkingsockets_certs.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/steamnetworkingsockets_certstore.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/steamnetworkingsockets_shared.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/steamnetworkingsockets_stats.cpp"
	"GameNetworkingSockets/src/steamnetworkingsockets/steamnetworkingsockets_thinker.cpp"
	"GameNetworkingSockets/src/tier0/dbg.cpp"
	"GameNetworkingSockets/src/tier0/platformtime.cpp"
	"GameNetworkingSockets/src/tier1/netadr.cpp"
	"GameNetworkingSockets/src/tier1/utlbuffer.cpp"
	"GameNetworkingSockets/src/tier1/utlmemory.cpp"
	"GameNetworkingSockets/src/tier1/ipv6text.c"
	"GameNetworkingSockets/src/vstdlib/strtools.cpp"
	)

if(USE_CRYPTO STREQUAL "BCrypt")
	set(GNS_CRYPTO_DEFINES ${GNS_CRYPTO_DEFINES} STEAMNETWORKINGSOCKETS_CRYPTO_BCRYPT ED25519_HASH_BCRYPT)
	set(GNS_SRCS ${GNS_SRCS}
		"GameNetworkingSockets/src/common/crypto_bcrypt.cpp"
		)
endif()

if(USE_CRYPTO STREQUAL "OpenSSL")
	set(GNS_CRYPTO_DEFINES ${GNS_CRYPTO_DEFINES} STEAMNETWORKINGSOCKETS_CRYPTO_VALVEOPENSSL)
	set(GNS_SRCS ${GNS_SRCS}
		"GameNetworkingSockets/src/common/crypto_openssl.cpp"
		"GameNetworkingSockets/src/common/opensslwrapper.cpp"
		)
endif()

if(USE_CRYPTO STREQUAL "libsodium")
	set(GNS_CRYPTO_DEFINES ${GNS_CRYPTO_DEFINES} STEAMNETWORKINGSOCKETS_CRYPTO_LIBSODIUM)
	set(GNS_SRCS ${GNS_SRCS}
		"GameNetworkingSockets/src/common/crypto_libsodium.cpp")
endif()

# Use OpenSSL for 25519 if possible
if(USE_CRYPTO25519 STREQUAL "OpenSSL")
	set(GNS_CRYPTO_DEFINES  ${GNS_CRYPTO_DEFINES} STEAMNETWORKINGSOCKETS_CRYPTO_25519_OPENSSL)
	set(GNS_SRCS ${GNS_SRCS}
		"GameNetworkingSockets/src/common/crypto_25519_openssl.cpp"
		)
endif()

if(USE_CRYPTO25519 STREQUAL "libsodium")
	set(GNS_CRYPTO_DEFINES ${GNS_CRYPTO_DEFINES} STEAMNETWORKINGSOCKETS_CRYPTO_25519_LIBSODIUM)
	set(GNS_SRCS ${GNS_SRCS}
		"GameNetworkingSockets/src/common/crypto_25519_libsodium.cpp"
		)
endif()

# Use reference 25519 crypto implementation?
if(USE_CRYPTO25519 STREQUAL "Reference")
	set(GNS_CRYPTO_DEFINES ${GNS_CRYPTO_DEFINES} VALVE_CRYPTO_25519_DONNA)
	set(GNS_SRCS ${GNS_SRCS}
		"GameNetworkingSockets/src/common/crypto_25519_donna.cpp"
		"GameNetworkingSockets/src/external/curve25519-donna/curve25519.c"
		"GameNetworkingSockets/src/external/curve25519-donna/curve25519_VALVE_sse2.c"
		"GameNetworkingSockets/src/external/ed25519-donna/ed25519_VALVE.c"
		"GameNetworkingSockets/src/external/ed25519-donna/ed25519_VALVE_sse2.c"
		)
endif()

# If WebRTC enabled, build the thin wrapper library
if(USE_STEAMWEBRTC)
	add_subdirectory(external/steamwebrtc)
endif(USE_STEAMWEBRTC)

set(C99_FEATURES
	c_restrict
	c_function_prototypes)

set(CXX11_FEATURES
	cxx_constexpr
	cxx_auto_type)

set(GNS_COMMON_FLAGS
	-fvisibility=hidden
	-fno-strict-aliasing
	-Wall
	-Wno-unknown-pragmas
	-Wno-sign-compare
	-Wno-unused-local-typedef
	-Wno-unused-const-variable
	-Wno-nested-anon-types
	-Wno-format-truncation
	)

if(USE_CRYPTO25519 STREQUAL "Reference")
	# We don't use some of the 25519 functions with static linkage. Silence
	# -Wunused-function if we're including the reference ed25519/curve25519
	# stuff.
	set(GNS_COMMON_FLAGS ${GNS_COMMON_FLAGS} -Wno-unused-function)
endif()

if(WERROR)
	set(GNS_COMMON_FLAGS
		${GNS_COMMON_FLAGS}
		-Werror)
endif()

set(GNS_C_FLAGS
	-Wstrict-prototypes
	)

set(GNS_CXX_FLAGS
	-fvisibility-inlines-hidden
	-Wno-reorder
	-Wno-non-virtual-dtor
	-Wno-zero-as-null-pointer-constant
	-fno-exceptions
	)

if(NOT SANITIZE_UNDEFINED)
	set(GNS_CXX_FLAGS
		${GNS_CXX_FLAGS}
		-fno-rtti
		)
endif()

protobuf_generate_cpp(GNS_PROTO_SRCS GNS_PROTO_HDRS ${GNS_PROTOS})

macro(gamenetworkingsockets_common GNS_TARGET)

	target_sources(${GNS_TARGET} PRIVATE ${GNS_SRCS})
	target_sources(${GNS_TARGET} PRIVATE ${GNS_PROTO_SRCS})
	target_sources(${GNS_TARGET} PRIVATE ${GNS_PROTO_HDRS})

	set_target_common_gns_properties( ${GNS_TARGET} )

	target_include_directories(${GNS_TARGET} PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
		"$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/GameNetworkingSockets>"
		)

	target_include_directories(${GNS_TARGET} PRIVATE
		"GameNetworkingSockets/src/common"
		"GameNetworkingSockets/src/public"
		${CMAKE_CURRENT_BINARY_DIR}

		# Only necessary if we're not using protobuf::libprotobuf
		# But that's not supported until CMake 3.9
		${Protobuf_INCLUDE_DIRS}
		)

	target_link_libraries(${GNS_TARGET} PUBLIC
		# protobuf::libprotobuf
		${PROTOBUF_LIBRARIES}
		Threads::Threads
		)

	# Enable ICE?
	if(USE_STEAMWEBRTC)

		# Enable ICE
		target_compile_definitions(${GNS_TARGET} PRIVATE
			STEAMNETWORKINGSOCKETS_ENABLE_ICE )

		# Wrapper lib is always linked statically in the opensource code.
		# We might link dynamically in other environments.
		target_compile_definitions(${GNS_TARGET} PRIVATE
			STEAMWEBRTC_USE_STATIC_LIBS
			)
		target_link_libraries(${GNS_TARGET} PUBLIC
			steamwebrtc
			)
	endif()

	if(USE_CRYPTO STREQUAL "OpenSSL" OR USE_CRYPTO25519 STREQUAL "OpenSSL")
		target_link_libraries(${GNS_TARGET} PUBLIC
			crypto
			)
		if(WIN32 AND OPENSSL_USE_STATIC_LIBS)
			target_link_libraries(${GNS_TARGET} PUBLIC
				ws2_32
				crypt32
				)
		endif()
	endif()

	if(USE_CRYPTO STREQUAL "libsodium" OR USE_CRYPTO25519 STREQUAL "libsodium")
		target_link_libraries(${GNS_TARGET} PUBLIC
			sodium
			)
	endif()

	target_compile_definitions(${GNS_TARGET} PRIVATE
		STEAMNETWORKINGSOCKETS_FOREXPORT
		ENABLE_OPENSSLCONNECTION
		CRYPTO_DISABLE_ENCRYPT_WITH_PASSWORD
		${GNS_CRYPTO_DEFINES}
		)

	if(SANITIZE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_compile_definitions(${GNS_TARGET} PRIVATE ED25519_NO_INLINE_ASM)
	endif()


	if(CMAKE_SYSTEM_NAME MATCHES Linux)
	elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
	elseif(CMAKE_SYSTEM_NAME MATCHES FreeBSD)

	elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
		if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			get_target_property(TARGET_TYPE ${GNS_TARGET} TYPE)
			if(NOT TARGET_TYPE STREQUAL STATIC_LIBRARY)
				target_compile_options(${GNS_TARGET} PRIVATE
					/GL       # Enable link-time code generation
					)
				set_target_properties(${GNS_TARGET} PROPERTIES LINK_FLAGS "/LTCG /SUBSYSTEM:WINDOWS")
			endif()
		endif()
		target_link_libraries(${GNS_TARGET} PUBLIC ws2_32 crypt32)
		if(USE_CRYPTO STREQUAL "BCrypt")
			target_link_libraries(${GNS_TARGET} PUBLIC bcrypt)
		endif()
	else()
		message(FATAL_ERROR "Could not identify your target operating system")
	endif()

endmacro()

# add_library(GameNetworkingSockets SHARED "")
# add_library(GameNetworkingSockets::GameNetworkingSockets ALIAS GameNetworkingSockets)
# add_library(GameNetworkingSockets::shared ALIAS GameNetworkingSockets)
# gamenetworkingsockets_common(GameNetworkingSockets)

add_library(GameNetworkingSockets_s STATIC "")
add_library(GameNetworkingSockets::GameNetworkingSockets_s ALIAS GameNetworkingSockets_s)
add_library(GameNetworkingSockets::static ALIAS GameNetworkingSockets_s)
target_compile_definitions(GameNetworkingSockets_s INTERFACE STEAMNETWORKINGSOCKETS_STATIC_LINK)
gamenetworkingsockets_common(GameNetworkingSockets_s)

# Export doesn't work if we're using WebRTC and the Abseil dependency came from the submodule
if(NOT (USE_STEAMWEBRTC AND STEAMWEBRTC_ABSL_SOURCE STREQUAL submodule))

	include(CMakePackageConfigHelpers)

	# Ensure that variables used in GameNetworkingSocketsConfig.cmake.in have some value
	# rather than an empty string.
	if(NOT USE_CRYPTO)
		set(USE_CRYPTO USE_CRYPTO-NOTFOUND)
	endif()

	if(NOT STEAMWEBRTC_ABSL_SOURCE)
		set(STEAMWEBRTC_ABSL_SOURCE STEAMWEBRTC_ABSL_SOURCE-NOTFOUND)
	endif()

endif()

target_link_libraries(GameNetworkingSockets_s PUBLIC libprotobuf)
include_directories("${CMAKE_CURRENT_LIST_DIR}/GameNetworkingSockets/include")