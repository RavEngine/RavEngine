@echo OFF

mkdir ..\build && cd ..\build && cmake -DRAVENGINE_BUILD_TESTS=ON .. ; cmake -DRAVENGINE_BUILD_TESTS=ON .. && cmake --build . --config debug --target RavEngine_TestBasics && ctest -C debug
