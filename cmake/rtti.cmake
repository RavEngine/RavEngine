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
			if (NOT MSVC)
				target_compile_options(${TG} PRIVATE "-frtti")
			else()
				target_compile_options(${TG} PRIVATE "/GR")
			endif()
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