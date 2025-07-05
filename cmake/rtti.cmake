function(rve_disable_rtti TG)
	if (NOT MSVC)
		target_compile_options(${TG} PRIVATE "-fno-rtti")
	else()
		target_compile_options(${TG} PRIVATE "/GR-")
	endif()
endfunction()

function(disable_rtti_in_dir DIR)
	get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
	foreach(TG ${TGTS})
		if (${TG} STREQUAL "assimp")
			continue()	# assimp needs RTTI for the FBX importer
		endif()
		get_target_property(target_type ${TG} TYPE)
		if (
			${target_type} MATCHES "EXECUTABLE" OR 
			${target_type} MATCHES "STATIC_LIBRARY" OR 
			${target_type} MATCHES "SHARED_LIBRARY" OR
			${target_type} MATCHES "OBJECT_LIBRARY"
		)
		rve_disable_rtti(${TG})
		endif()
	endforeach()

	get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
	foreach(SUBDIR IN LISTS SUBDIRS)
		disable_rtti_in_dir("${SUBDIR}")
	endforeach()
endfunction()

function(set_iterator_debug_level DIR)
	get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
	foreach(TG ${TGTS})
		get_target_property(target_type ${TG} TYPE)
		if (
			${target_type} MATCHES "EXECUTABLE" OR 
			${target_type} MATCHES "STATIC_LIBRARY" OR 
			${target_type} MATCHES "SHARED_LIBRARY" OR
			${target_type} MATCHES "OBJECT_LIBRARY"
		)
		target_compile_definitions(${TG} PUBLIC 
			$<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
		)
		endif()
	endforeach()

	get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
	foreach(SUBDIR IN LISTS SUBDIRS)
		set_iterator_debug_level("${SUBDIR}")
	endforeach()
endfunction()

function(enable_debug_all DIR)
	get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
	foreach(TG ${TGTS})
		get_target_property(target_type ${TG} TYPE)
		if (
			${target_type} MATCHES "EXECUTABLE" OR 
			${target_type} MATCHES "STATIC_LIBRARY" OR 
			${target_type} MATCHES "SHARED_LIBRARY" OR
			${target_type} MATCHES "OBJECT_LIBRARY"
		)
		 target_link_options(${TG} PUBLIC /DEBUG:FULL /dynamicdeopt)
		endif()
	endforeach()

	get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
	foreach(SUBDIR IN LISTS SUBDIRS)
		enable_debug_all("${SUBDIR}")
	endforeach()
endfunction()