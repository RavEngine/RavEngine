OGRE-next DEPENDENCIES SOURCE PACKAGE
==================================

This package is provided as a quick route to compile the core
dependencies of [OGRE-next](https://github.com/OGRECave/ogre-next) on most supported
platforms. For a list of the included libraries and their 
versions, see versions.txt

COMPILATION
=============

You need CMake (http://www.cmake.org). In a console, type:

```
cd /path/to/ogredeps
mkdir build
cd build
cmake ..
make
make install
```

If you are on a Windows system or prefer graphical interfaces,
launch cmake-gui instead. Enter as the source code path the
path where you extracted ogredeps (i.e. where this readme 
resides). Select any directory to build the binaries. Hit
'Configure', choose your compiler set and click Ok. Click
'Generate' twice. CMake will generate a set of project files
for your IDE (e.g. Visual Studio or XCode) in the chosen build
directory. Open them, compile the target 'BUILD_ALL'. Also build
the target 'install'.

USAGE
=======

When compilation was successful and the install target was built,
you should find a new directory 'ogredeps' in your build path.
This contains the final include and lib files needed. Copy it
to your Ogre source or build directory, then rerun CMake for your
Ogre build. It should pick up the dependencies automatically.
