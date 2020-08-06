project(zziplib)

set(zziplib_SOURCES
  zzip/autoconf.h
  zzip/conf.h
  zzip/_config.h
  zzip/__debug.h
  zzip/dir.c
  zzip/__dirent.h
  zzip/err.c
  zzip/fetch.c
  zzip/fetch.h
  zzip/file.c
  zzip/file.h
  zzip/__fnmatch.h
  zzip/format.h
  zzip/fseeko.c
  zzip/fseeko.h
  zzip/__hints.h
  zzip/info.c
  zzip/info.h
  zzip/lib.h
  zzip/memdisk.c
  zzip/memdisk.h
  zzip/__mmap.h
  zzip/mmapped.c
  zzip/mmapped.h
  zzip/_msvc.h
  zzip/plugin.c
  zzip/plugin.h
  zzip/stat.c
  zzip/stdint.h
  zzip/types.h
  zzip/write.c
  zzip/write.h
  zzip/zip.c
  zzip/zzip32.h
  zzip/zzip.h
)

# need zlib includes
include_directories(${OGREDEPS_SOURCE_DIR}/src/zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32 AND BUILD_SHARED_LIBS)
  add_definitions(-DZZIP_DLL)
  link_libraries(zlib)
endif ()

add_library(zziplib STATIC ${zziplib_SOURCES})
install_dep(zziplib include/zzip zzip/_config.h zzip/conf.h zzip/types.h zzip/zzip.h zzip/plugin.h zzip/_msvc.h)
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET zziplib PROPERTY FOLDER Dependencies)
endif ()

if (APPLE)
 set_target_properties(zziplib PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")

 if(OGRE_BUILD_PLATFORM_APPLE_IOS)
  set_target_properties(zziplib PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
 endif()
endif()
set(ZZip_INCLUDE_DIR "${zziplib_SOURCE_DIR}" CACHE PATH "" FORCE)
set(ZZip_LIBRARY_DBG "zziplib" CACHE STRING "" FORCE)
set(ZZip_LIBRARY_REL "zziplib" CACHE STRING "" FORCE)
mark_as_advanced(ZZip_INCLUDE_DIR ZZip_LIBRARY_DBG ZZip_LIBRARY_REL)
