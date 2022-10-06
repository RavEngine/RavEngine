# Generate the Git Build ID information into a C source file
# This does not overwrite the file if the contents are up-to-date.
#
# Arguments:
#   SOURCE_DIR   The root directory of the project, expected to be a Git repo
#   OUTPUT_FILE  The file which gets written

get_filename_component(OUTPUT_NAME "${OUTPUT_FILE}" NAME)
get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)

message("(Git Build ID) Generating ${OUTPUT_NAME}")

find_package(Git QUIET)

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_ID
        OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(GIT_COMMIT_ID "")
    message("(Git Build ID) Error: could not find Git")
endif()

file(WRITE "${OUTPUT_FILE}.temp" "const char* GitBuildId = \"${GIT_COMMIT_ID}\";\n")
execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "copy_if_different" "${OUTPUT_FILE}.temp" "${OUTPUT_FILE}")
file(REMOVE "${OUTPUT_FILE}.temp")
