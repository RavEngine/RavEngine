mkdir -p ../build && cd ../build
cmake -G "Ninja" -DRAVENGINE_BUILD_TESTS=ON .. && cmake --build . --config debug --target RavEngine_TestBasics && ctest -C debug
