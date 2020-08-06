project(freetype)

set(freetype_SOURCES
  #builds/win32/ftdebug.c
	include/freetype/config/ftconfig.h
	include/freetype/config/ftheader.h
	include/freetype/config/ftmodule.h
	include/freetype/config/ftoption.h
	include/freetype/config/ftstdlib.h
	include/freetype/freetype.h
	include/freetype/ftadvanc.h
	include/freetype/ftbbox.h
	include/freetype/ftbdf.h
	include/freetype/ftbitmap.h
    include/freetype/ftbzip2.h
	include/freetype/ftcache.h
	include/freetype/ftchapters.h
	include/freetype/ftcid.h
	include/freetype/fterrdef.h
	include/freetype/fterrors.h
	include/freetype/ftgasp.h
	include/freetype/ftglyph.h
	include/freetype/ftgxval.h
	include/freetype/ftgzip.h
	include/freetype/ftimage.h
	include/freetype/ftincrem.h
	include/freetype/ftlcdfil.h
	include/freetype/ftlist.h
	include/freetype/ftlzw.h
	include/freetype/ftmac.h
	include/freetype/ftmm.h
	include/freetype/ftmodapi.h
	include/freetype/ftmoderr.h
	include/freetype/ftotval.h
	include/freetype/ftoutln.h
	include/freetype/ftpfr.h
	include/freetype/ftrender.h
	include/freetype/ftsizes.h
	include/freetype/ftsnames.h
	include/freetype/ftstroke.h
	include/freetype/ftsynth.h
	include/freetype/ftsystem.h
	include/freetype/fttrigon.h
	include/freetype/fttypes.h
	include/freetype/ftwinfnt.h
	include/freetype/ftxf86.h
	include/freetype/t1tables.h
	include/freetype/ttnameid.h
	include/freetype/tttables.h
	include/freetype/tttags.h
	include/freetype/ttunpat.h
	include/ft2build.h
	src/autofit/autofit.c
	src/base/ftbase.c
	src/base/ftbbox.c
	src/base/ftbitmap.c
	src/base/ftfstype.c
	src/base/ftgasp.c
	src/base/ftglyph.c
	src/base/ftinit.c
	src/base/ftmm.c
	src/base/ftpfr.c
	src/base/ftstroke.c
	src/base/ftsynth.c
	src/base/ftsystem.c
	src/base/fttype1.c
	src/base/ftwinfnt.c
	src/bdf/bdf.c
    src/bzip2/ftbzip2.c
	src/cache/ftcache.c
	src/cff/cff.c
	src/cid/type1cid.c
	src/gzip/ftgzip.c
	src/lzw/ftlzw.c
	src/pcf/pcf.c
	src/pfr/pfr.c
	src/psaux/psaux.c
	src/pshinter/pshinter.c
	src/psnames/psmodule.c
	src/raster/raster.c
	src/sfnt/sfnt.c
	src/smooth/smooth.c
	src/truetype/truetype.c
	src/type1/type1.c
	src/type42/type42.c
	src/winfonts/winfnt.c
)

include_directories(include)
add_definitions(-DFT2_BUILD_LIBRARY)
add_library(freetype STATIC ${freetype_SOURCES})
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET freetype PROPERTY FOLDER Dependencies)
endif ()
install_dep(freetype include)
if (OGREDEPS_INSTALL_DEV)
  install(DIRECTORY include/ DESTINATION include)
endif ()

if (APPLE)
 set_target_properties(freetype PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")

 if(OGRE_BUILD_PLATFORM_APPLE_IOS)
  add_definitions(-DHAVE_QUICKDRAW_CARBON=0 -DHAVE_QUICKDRAW_TOOLBOX=0 -DHAVE_ATS=0 -DHAVE_FSREF=0 -DHAVE_FSSPEC=0 -DDARWIN_NO_CARBON=1 -DFT_CONFIG_OPTION_NO_ASSEMBLER=1)

  set_target_properties(freetype PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
 endif()
endif()

set(FREETYPE_FT2BUILD_INCLUDE_DIR "${freetype_SOURCE_DIR}/include" CACHE PATH "" FORCE)
set(FREETYPE_INCLUDE_DIR "${freetype_SOURCE_DIR}/include/freetype" CACHE PATH "" FORCE)
set(FREETYPE_LIBRARY_DBG "freetype" CACHE STRING "" FORCE)
set(FREETYPE_LIBRARY_REL "freetype" CACHE STRING "" FORCE)
mark_as_advanced(FREETYPE_FT2BUILD_INCLUDE_DIR FREETYPE_INCLUDE_DIR FREETYPE_LIBRARY_DBG FREETYPE_LIBRARY_REL)
