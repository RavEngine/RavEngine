## Process this file with automake to produce Makefile.in

DISTCHECK_CONFIGURE_FLAGS = --enable-werror

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = README.md autogen.sh libsamplerate.spec.in samplerate.pc.in  \
    Octave/generate_filter.m Octave/make_filter.m Octave/measure_filter.m \
    Octave/Readme.md Win32/libsamplerate-0.def

pkgconfig_DATA = samplerate.pc

# Some people prefer "make test" to "make check".
test:
	$(MAKE) check

########
# src/ #
########

lib_LTLIBRARIES = src/libsamplerate.la
include_HEADERS = include/samplerate.h

EXTRA_DIST += cmake CMakeLists.txt config.h.cmake docs/CMakeLists.txt   \
	examples/CMakeLists.txt src/CMakeLists.txt tests/CMakeLists.txt \
	src/config.h.in src/Version_script.in src/check_asm.sh \
	tests/streaming_test.c
CLEANFILES = src/src_sinc.s

# MinGW requires -no-undefined if a DLL is to be built.
src_libsamplerate_la_LDFLAGS = -no-undefined -version-info $(SHARED_VERSION_INFO) $(SHLIB_VERSION_ARG)
src_libsamplerate_la_SOURCES = src/samplerate.c src/src_sinc.c src/src_zoh.c src/src_linear.c \
	src/common.h src/fastest_coeffs.h src/mid_qual_coeffs.h src/high_qual_coeffs.h

#-------------------------------------------------------------------------------
# An extra check for bad asm.

check-asm: src/check_asm.sh src/src_sinc.s src/src_linear.s src/src_zoh.s
	@echo
	@echo
	$(top_srcdir)/src/check_asm.sh src/src_sinc.s
	$(top_srcdir)/src/check_asm.sh src/src_linear.s
	$(top_srcdir)/src/check_asm.sh src/src_zoh.s
	@echo
	@echo

.c.s:
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $(DEFAULT_INCLUDES) $< -o $@

# Disable autoheader.
AUTOHEADER=echo

########
# docs/ #
########

dist_doc_DATA = docs/SRC.png docs/SRC.css docs/index.md docs/license.md docs/history.md \
	docs/download.md docs/lists.md docs/quality.md docs/win32.md docs/faq.md docs/api.md \
	docs/api_simple.md docs/api_callback.md docs/api_full.md docs/api_misc.md docs/bugs.md

#############
# examples/ #
#############

if HAVE_LIBSNDFILE
noinst_PROGRAMS = examples/varispeed-play examples/timewarp-file

examples_varispeed_play_SOURCES = examples/varispeed-play.c examples/audio_out.c examples/audio_out.h
examples_varispeed_play_CFLAGS = $(SNDFILE_CFLAGS) $(AUDIO_CFLAGS)
examples_varispeed_play_LDADD = src/libsamplerate.la $(SNDFILE_LIBS) $(AUDIO_LIBS)

examples_timewarp_file_SOURCES = examples/timewarp-file.c
examples_timewarp_file_CFLAGS = $(SNDFILE_CFLAGS)
examples_timewarp_file_LDADD = src/libsamplerate.la $(SNDFILE_LIBS)
endif

##########
# tests/ #
##########

TESTS = \
	tests/callback_hang_test \
	tests/callback_test \
	tests/clone_test \
	tests/downsample_test \
	tests/float_short_test \
	tests/misc_test \
	tests/multi_channel_test \
	tests/nullptr_test \
	tests/reset_test \
	tests/simple_test \
	tests/snr_bw_test \
	tests/termination_test \
	tests/throughput_test \
	tests/varispeed_test

check_PROGRAMS = \
	$(TESTS) \
	tests/multichan_throughput_test \
	tests/src-evaluate

#===============================================================================

tests_misc_test_SOURCES = tests/misc_test.c tests/util.c tests/util.h
tests_misc_test_LDADD = src/libsamplerate.la

tests_termination_test_SOURCES = tests/termination_test.c tests/util.c tests/util.h
tests_termination_test_LDADD = src/libsamplerate.la

tests_callback_hang_test_SOURCES = tests/callback_hang_test.c tests/util.c tests/util.h
tests_callback_hang_test_LDADD = src/libsamplerate.la

tests_simple_test_SOURCES = tests/simple_test.c tests/util.c tests/util.h
tests_simple_test_LDADD = src/libsamplerate.la

tests_reset_test_SOURCES = tests/reset_test.c tests/util.c tests/util.h
tests_reset_test_LDADD = src/libsamplerate.la

tests_multi_channel_test_SOURCES = tests/multi_channel_test.c tests/util.c tests/calc_snr.c
tests_multi_channel_test_CFLAGS = $(FFTW3_CFLAGS)
tests_multi_channel_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_snr_bw_test_SOURCES = tests/snr_bw_test.c tests/calc_snr.c tests/util.c tests/util.h
tests_snr_bw_test_CFLAGS = $(FFTW3_CFLAGS)
tests_snr_bw_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_callback_test_SOURCES = tests/callback_test.c tests/util.c tests/util.h
tests_callback_test_LDADD = src/libsamplerate.la

tests_float_short_test_SOURCES = tests/float_short_test.c tests/util.c tests/util.h
tests_float_short_test_LDADD = src/libsamplerate.la

tests_downsample_test_SOURCES = tests/downsample_test.c tests/util.c tests/util.h
tests_downsample_test_LDADD = src/libsamplerate.la

tests_varispeed_test_SOURCES = tests/varispeed_test.c tests/util.c tests/util.h tests/calc_snr.c
tests_varispeed_test_CFLAGS = $(FFTW3_CFLAGS)
tests_varispeed_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_clone_test_SOURCES = tests/clone_test.c tests/util.c tests/util.h
tests_clone_test_LDADD = src/libsamplerate.la

tests_nullptr_test_SOURCES = tests/nullptr_test.c tests/util.c tests/util.h
tests_nullptr_test_LDADD = src/libsamplerate.la

# This program is for evaluating other sample rate converters.

tests_throughput_test_SOURCES = tests/throughput_test.c tests/util.c tests/calc_snr.c
tests_throughput_test_CFLAGS = $(FFTW3_CFLAGS)
tests_throughput_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_multichan_throughput_test_SOURCES = tests/multichan_throughput_test.c tests/util.c tests/calc_snr.c
tests_multichan_throughput_test_CFLAGS = $(FFTW3_CFLAGS)
tests_multichan_throughput_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_src_evaluate_SOURCES = tests/src-evaluate.c tests/calc_snr.c tests/util.c
tests_src_evaluate_CFLAGS = $(SNDFILE_CFLAGS) $(FFTW3_CFLAGS)
tests_src_evaluate_LDADD = $(SNDFILE_LIBS) $(FFTW3_LIBS)
