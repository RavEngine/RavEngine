check_include_file(sys/times.h HAVE_SYS_TIMES_H)

check_function_exists(alarm HAVE_ALARM)
check_function_exists(signal HAVE_SIGNAL)

check_symbol_exists(SIGALRM signal.h HAVE_SIGALRM)

find_package(FFTW3)
set(HAVE_FFTW3 ${FFTW3_FOUND} PARENT_SCOPE)

add_executable(misc_test misc_test.c util.c util.h)
target_link_libraries(misc_test PRIVATE samplerate)
add_test(NAME misc_test COMMAND misc_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(termination_test termination_test.c util.c util.h)
target_link_libraries(termination_test PRIVATE samplerate)
add_test(NAME termination_test COMMAND termination_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(callback_hang_test callback_hang_test.c util.c util.h)
target_link_libraries(callback_hang_test PRIVATE samplerate)
add_test(NAME callback_hang_test COMMAND callback_hang_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(downsample_test downsample_test.c util.c util.h)
target_link_libraries(downsample_test PRIVATE samplerate)
add_test(NAME downsample_test COMMAND downsample_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(simple_test simple_test.c util.c util.h)
target_link_libraries(simple_test PRIVATE samplerate)
add_test(NAME simple_test COMMAND simple_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(callback_test callback_test.c util.c util.h)
target_link_libraries(callback_test PRIVATE samplerate)
add_test(NAME callback_test COMMAND callback_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(reset_test reset_test.c util.c util.h)
target_link_libraries(reset_test PRIVATE samplerate)
add_test(NAME reset_test COMMAND reset_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(clone_test clone_test.c util.c util.h)
target_link_libraries(clone_test PRIVATE samplerate)
add_test(NAME clone_test COMMAND clone_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(nullptr_test nullptr_test.c util.c util.h)
target_link_libraries(nullptr_test PRIVATE samplerate)
add_test(NAME nullptr_test COMMAND nullptr_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(multi_channel_test multi_channel_test.c calc_snr.c util.c util.h)
target_link_libraries(multi_channel_test
  PRIVATE
    samplerate
    $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
   )
add_test(NAME multi_channel_test COMMAND multi_channel_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(varispeed_test varispeed_test.c calc_snr.c util.c util.h)
target_link_libraries(varispeed_test
  PRIVATE samplerate
    $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
   )
add_test(NAME varispeed_test COMMAND varispeed_test util.c util.h WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(float_short_test float_short_test.c)
target_link_libraries(float_short_test PRIVATE samplerate)
add_test(NAME float_short_test COMMAND float_short_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(snr_bw_test snr_bw_test.c calc_snr.c util.c util.h)
target_link_libraries(snr_bw_test
  PRIVATE
    samplerate
    $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
   )
add_test(NAME snr_bw_test COMMAND snr_bw_test util.c util.h WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

add_executable(throughput_test throughput_test.c util.c util.h)
target_link_libraries(throughput_test PRIVATE samplerate)

add_executable(multichan_throughput_test multichan_throughput_test.c util.c util.h)
target_link_libraries(multichan_throughput_test PRIVATE samplerate)

add_executable(src-evaluate src-evaluate.c util.c util.h calc_snr.c)
target_link_libraries(src-evaluate
  PRIVATE
    samplerate
    $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
    $<$<BOOL:${SndFile_FOUND}>:SndFile::sndfile>
   )
