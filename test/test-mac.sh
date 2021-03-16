mkdir -p ../build && cd ../build
cmake -G "Xcode" -DRAVENGINE_BUILD_TESTS=ON .. && cmake --build . --config debug --target RavEngine_TestBasics && ctest -C debug
