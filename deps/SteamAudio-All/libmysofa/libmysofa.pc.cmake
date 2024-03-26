Name: @PROJECT_NAME@
Description: @CPACK_PACKAGE_DESCRIPTION@
Version: @PROJECT_VERSION@
Requires: @PKG_CONFIG_REQUIRES@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
Libs: -L${libdir} -lmysofa
Cflags: -I${includedir}

Libs.private: @PKG_CONFIG_PRIVATELIBS@
#Requires.private:
