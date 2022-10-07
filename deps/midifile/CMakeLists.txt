project(midifile)

file(GLOB midifile_srcs "src/*.cpp" "include/*.h")
add_library(${PROJECT_NAME} ${midifile_srcs})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)