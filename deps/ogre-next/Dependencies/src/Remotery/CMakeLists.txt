project(Remotery)

set( Remotery_SOURCES src/Remotery.c src/Remotery.h src/RemoteryConfig.h )

if( APPLE )
	set( Remotery_SOURCES ${Remotery_SOURCES} src/RemoteryMetal.mm )
endif()

if( OGRE_BUILD_PLATFORM_APPLE_IOS )
	add_library( Remotery STATIC ${Remotery_SOURCES} )
else()
	add_library( Remotery SHARED ${Remotery_SOURCES} )
endif()

set( RMT_USE_OPENGL 0 )
set( RMT_USE_D3D11 0 )
set( RMT_USE_METAL 0 )
if( UNIX OR WIN32 )
	set( RMT_USE_OPENGL 1 )
endif()
if( WIN32 )
	set( RMT_USE_D3D11 1 )
endif()
if( APPLE )
	set( RMT_USE_METAL 1 )
endif()
configure_file( RemoteryConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteryConfig.h )

install_dep( Remotery include src/Remotery.h src/RemoteryConfig.h )
