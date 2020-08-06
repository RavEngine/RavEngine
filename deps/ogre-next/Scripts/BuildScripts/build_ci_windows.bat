
@echo off
SETLOCAL

set OGRE_BRANCH_NAME=v2-2
set GENERATOR="Visual Studio 14 2015"
set PLATFORM=x64
set CONFIGURATION=Debug
set BUILD_FOLDER=%APPVEYOR_BUILD_FOLDER%

set CMAKE_BIN_x86="C:\Program Files (x86)\CMake\bin\cmake.exe"
set CMAKE_BIN_x64="C:\Program Files\CMake\bin\cmake.exe"
IF EXIST %CMAKE_BIN_x64% (
	echo CMake 64-bit detected
	set CMAKE_BIN=%CMAKE_BIN_x64%
) ELSE (
	IF EXIST %CMAKE_BIN_x86% (
		echo CMake 32-bit detected
		set CMAKE_BIN=%CMAKE_BIN_x86%
	) ELSE (
		echo Cannot detect either %CMAKE_BIN_x86% or
		echo %CMAKE_BIN_x64% make sure CMake is installed
		EXIT /B 1
	)
)
echo Using CMake at %CMAKE_BIN%

IF NOT EXIST %BUILD_FOLDER%\..\ogredeps (
	mkdir %BUILD_FOLDER%\..\ogredeps
	echo --- Cloning Ogredeps ---
	hg clone https://bitbucket.org/cabalistic/ogredeps %BUILD_FOLDER%\..\ogredeps
) ELSE (
	echo --- Ogredeps repo detected. Cloning skipped ---
)
mkdir %BUILD_FOLDER%\..\ogredeps\build
cd %BUILD_FOLDER%\..\ogredeps\build
echo --- Building Ogredeps ---
%CMAKE_BIN% -G %GENERATOR% -A %PLATFORM% %BUILD_FOLDER%\..\ogredeps
%CMAKE_BIN% --build . --config %CONFIGURATION%
%CMAKE_BIN% --build . --target install --config %CONFIGURATION%

cd %BUILD_FOLDER%
mkdir %BUILD_FOLDER%\build
cd %BUILD_FOLDER%\build
echo --- Running CMake configure ---
%CMAKE_BIN% -D OGRE_UNITY_BUILD=1 -D OGRE_USE_BOOST=0 -D OGRE_CONFIG_THREAD_PROVIDER=0 -D OGRE_CONFIG_THREADS=0 -D OGRE_BUILD_COMPONENT_SCENE_FORMAT=1 -D OGRE_BUILD_SAMPLES2=1 -D OGRE_BUILD_TESTS=1 -D OGRE_DEPENDENCIES_DIR=%BUILD_FOLDER%\..\ogredeps\build\ogredeps -G %GENERATOR% -A %PLATFORM% %BUILD_FOLDER%
echo --- Building Ogre ---
%CMAKE_BIN% --build . --config %CONFIGURATION%
%CMAKE_BIN% --build . --target install --config %CONFIGURATION%

echo Done!

ENDLOCAL
