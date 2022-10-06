# Common definitions for builds based on GNU Make

ifndef SFIZZ_DIR
$(error sfizz: The source directory must be set before including)
endif

### Options

SFIZZ_USE_SNDFILE ?= 0

###

SFIZZ_MACHINE := $(shell $(CC) -dumpmachine)
SFIZZ_PROCESSOR := $(firstword $(subst -, ,$(SFIZZ_MACHINE)))

ifneq (,$(filter i%86,$(SFIZZ_PROCESSOR)))
SFIZZ_CPU_I386 := 1
SFIZZ_CPU_I386_OR_X86_64 := 1
endif
ifneq (,$(filter x86_64,$(SFIZZ_PROCESSOR)))
SFIZZ_CPU_X86_64 := 1
SFIZZ_CPU_I386_OR_X86_64 := 1
endif
ifneq (,$(filter arm%,$(SFIZZ_PROCESSOR)))
SFIZZ_CPU_ARM := 1
SFIZZ_CPU_ARM_OR_AARCH64 := 1
endif
ifneq (,$(filter aarch64%,$(SFIZZ_PROCESSOR)))
SFIZZ_CPU_AARCH64 := 1
SFIZZ_CPU_ARM_OR_AARCH64 := 1
endif

ifneq (,$(findstring linux,$(SFIZZ_MACHINE)))
SFIZZ_OS_LINUX := 1
endif
ifneq (,$(findstring apple,$(SFIZZ_MACHINE)))
SFIZZ_OS_APPLE := 1
endif
ifneq (,$(findstring mingw,$(SFIZZ_MACHINE)))
SFIZZ_OS_WINDOWS := 1
endif

###

SFIZZ_C_FLAGS = -I$(SFIZZ_DIR)/src
SFIZZ_CXX_FLAGS = $(SFIZZ_C_FLAGS)

SFIZZ_SOURCES = \
	src/sfizz/ADSREnvelope.cpp \
	src/sfizz/AudioReader.cpp \
	src/sfizz/BeatClock.cpp \
	src/sfizz/Curve.cpp \
	src/sfizz/Defaults.cpp \
	src/sfizz/effects/Apan.cpp \
	src/sfizz/Effects.cpp \
	src/sfizz/modulations/ModId.cpp \
	src/sfizz/modulations/ModKey.cpp \
	src/sfizz/modulations/ModKeyHash.cpp \
	src/sfizz/modulations/ModMatrix.cpp \
	src/sfizz/modulations/sources/ADSREnvelope.cpp \
	src/sfizz/modulations/sources/ChannelAftertouch.cpp \
	src/sfizz/modulations/sources/PolyAftertouch.cpp \
	src/sfizz/modulations/sources/Controller.cpp \
	src/sfizz/modulations/sources/FlexEnvelope.cpp \
	src/sfizz/modulations/sources/LFO.cpp \
	src/sfizz/effects/Compressor.cpp \
	src/sfizz/effects/Disto.cpp \
	src/sfizz/effects/Eq.cpp \
	src/sfizz/effects/Filter.cpp \
	src/sfizz/effects/Fverb.cpp \
	src/sfizz/effects/Gain.cpp \
	src/sfizz/effects/Gate.cpp \
	src/sfizz/effects/impl/ResonantArrayAVX.cpp \
	src/sfizz/effects/impl/ResonantArray.cpp \
	src/sfizz/effects/impl/ResonantArraySSE.cpp \
	src/sfizz/effects/impl/ResonantStringAVX.cpp \
	src/sfizz/effects/impl/ResonantString.cpp \
	src/sfizz/effects/impl/ResonantStringSSE.cpp \
	src/sfizz/effects/Limiter.cpp \
	src/sfizz/effects/Lofi.cpp \
	src/sfizz/effects/Nothing.cpp \
	src/sfizz/effects/Rectify.cpp \
	src/sfizz/effects/Strings.cpp \
	src/sfizz/effects/Width.cpp \
	src/sfizz/EQPool.cpp \
	src/sfizz/FileId.cpp \
	src/sfizz/FileMetadata.cpp \
	src/sfizz/FilePool.cpp \
	src/sfizz/FilterPool.cpp \
	src/sfizz/FlexEGDescription.cpp \
	src/sfizz/FlexEnvelope.cpp \
	src/sfizz/Interpolators.cpp \
	src/sfizz/Layer.cpp \
	src/sfizz/LFO.cpp \
	src/sfizz/LFODescription.cpp \
	src/sfizz/Messaging.cpp \
	src/sfizz/Metronome.cpp \
	src/sfizz/MidiState.cpp \
	src/sfizz/OpcodeCleanup.cpp \
	src/sfizz/Opcode.cpp \
	src/sfizz/Oversampler.cpp \
	src/sfizz/Panning.cpp \
	src/sfizz/parser/Parser.cpp \
	src/sfizz/parser/ParserPrivate.cpp \
	src/sfizz/PolyphonyGroup.cpp \
	src/sfizz/PowerFollower.cpp \
	src/sfizz/Region.cpp \
	src/sfizz/RegionSet.cpp \
	src/sfizz/RegionStateful.cpp \
	src/sfizz/Resources.cpp \
	src/sfizz/RTSemaphore.cpp \
	src/sfizz/ScopedFTZ.cpp \
	src/sfizz/sfizz.cpp \
	src/sfizz/sfizz_wrapper.cpp \
	src/sfizz/SfzFilter.cpp \
	src/sfizz/SIMDHelpers.cpp \
	src/sfizz/simd/HelpersSSE.cpp \
	src/sfizz/simd/HelpersAVX.cpp \
	src/sfizz/Smoothers.cpp \
	src/sfizz/Synth.cpp \
	src/sfizz/SynthMessaging.cpp \
	src/sfizz/Tuning.cpp \
	src/sfizz/utility/spin_mutex/SpinMutex.cpp \
	src/sfizz/Voice.cpp \
	src/sfizz/VoiceManager.cpp \
	src/sfizz/VoiceStealing.cpp \
	src/sfizz/Wavetables.cpp \
	src/sfizz/WindowedSinc.cpp

### Other internal

SFIZZ_C_FLAGS += \
    -I$(SFIZZ_DIR)/src/sfizz \
    -I$(SFIZZ_DIR)/src/sfizz/utility/bit_array \
    -I$(SFIZZ_DIR)/src/sfizz/utility/spin_mutex

# Pkg-config dependency

SFIZZ_PKG_CONFIG ?= pkg-config

# Sndfile dependency

ifeq ($(SFIZZ_USE_SNDFILE),1)
SFIZZ_SNDFILE_C_FLAGS ?= $(shell $(SFIZZ_PKG_CONFIG) --cflags sndfile)
SFIZZ_SNDFILE_CXX_FLAGS ?= $(SFIZZ_SNDFILE_C_FLAGS)
SFIZZ_SNDFILE_LINK_FLAGS ?= $(shell $(SFIZZ_PKG_CONFIG) --libs sndfile)

SFIZZ_C_FLAGS += -DSFIZZ_USE_SNDFILE=1
SFIZZ_CXX_FLAGS += -DSFIZZ_USE_SNDFILE=1
endif

# st_audiofile dependency

SFIZZ_SOURCES += \
	external/st_audiofile/src/st_audiofile.c \
	external/st_audiofile/src/st_audiofile_common.c \
	external/st_audiofile/src/st_audiofile_libs.c \
	external/st_audiofile/src/st_audiofile_sndfile.c

ifneq ($(SFIZZ_USE_SNDFILE),1)
SFIZZ_SOURCES += \
	external/st_audiofile/src/st_audiofile_libs.c
endif

SFIZZ_C_FLAGS += \
	-I$(SFIZZ_DIR)/external/st_audiofile/src \
	-I$(SFIZZ_DIR)/external/st_audiofile/thirdparty/dr_libs \
	-I$(SFIZZ_DIR)/external/st_audiofile/thirdparty/stb_vorbis
SFIZZ_CXX_FLAGS += \
	-I$(SFIZZ_DIR)/external/st_audiofile/src \
	-I$(SFIZZ_DIR)/external/st_audiofile/thirdparty/dr_libs \
	-I$(SFIZZ_DIR)/external/st_audiofile/thirdparty/stb_vorbis

ifeq ($(SFIZZ_USE_SNDFILE),1)
SFIZZ_C_FLAGS += $(SFIZZ_SNDFILE_C_FLAGS) -DST_AUDIO_FILE_USE_SNDFILE=1
SFIZZ_CXX_FLAGS += $(SFIZZ_SNDFILE_CXX_FLAGS) -DST_AUDIO_FILE_USE_SNDFILE=1
SFIZZ_LINK_FLAGS += $(SFIZZ_SNDFILE_LINK_FLAGS)
endif

# libaiff dependency

ifneq ($(SFIZZ_USE_SNDFILE),1)
SFIZZ_SOURCES += \
	external/st_audiofile/thirdparty/libaiff/libaiff.all.c
SFIZZ_C_FLAGS += \
	-I$(SFIZZ_DIR)/external/st_audiofile/thirdparty/libaiff
SFIZZ_CXX_FLAGS += \
	-I$(SFIZZ_DIR)/external/st_audiofile/thirdparty/libaiff
endif

# hiir dependency

SFIZZ_CXX_FLAGS += -I$(SFIZZ_DIR)/src/external/hiir

# threadpool dependency

SFIZZ_CXX_FLAGS += -I$(SFIZZ_DIR)/external/threadpool

# atomic_queue dependency

SFIZZ_CXX_FLAGS += -I$(SFIZZ_DIR)/external/atomic_queue/include

# ghc::filesystem dependency

SFIZZ_CXX_FLAGS += -I$(SFIZZ_DIR)/external/filesystem/include

### Abseil dependency

SFIZZ_C_FLAGS += -I$(SFIZZ_DIR)/external/abseil-cpp
# absl::base
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/base/internal/cycleclock.cc \
	external/abseil-cpp/absl/base/internal/spinlock.cc \
	external/abseil-cpp/absl/base/internal/sysinfo.cc \
	external/abseil-cpp/absl/base/internal/thread_identity.cc \
	external/abseil-cpp/absl/base/internal/unscaledcycleclock.cc
# absl::malloc_internal
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/base/internal/low_level_alloc.cc
# absl::raw_logging_internal
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/base/internal/raw_logging.cc
# absl::spinlock_wait
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/base/internal/spinlock_wait.cc
# absl::throw_delegate
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/base/internal/throw_delegate.cc
# absl::strings
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/strings/ascii.cc \
	external/abseil-cpp/absl/strings/charconv.cc \
	external/abseil-cpp/absl/strings/escaping.cc \
	external/abseil-cpp/absl/strings/internal/charconv_bigint.cc \
	external/abseil-cpp/absl/strings/internal/charconv_parse.cc \
	external/abseil-cpp/absl/strings/internal/memutil.cc \
	external/abseil-cpp/absl/strings/match.cc \
	external/abseil-cpp/absl/strings/numbers.cc \
	external/abseil-cpp/absl/strings/str_cat.cc \
	external/abseil-cpp/absl/strings/str_replace.cc \
	external/abseil-cpp/absl/strings/str_split.cc \
	external/abseil-cpp/absl/strings/string_view.cc \
	external/abseil-cpp/absl/strings/substitute.cc
# absl::hashtablez_sampler
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/container/internal/hashtablez_sampler.cc \
	external/abseil-cpp/absl/container/internal/hashtablez_sampler_force_weak_definition.cc
# absl::raw_hash_set
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/container/internal/raw_hash_set.cc
# absl::synchronization
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/synchronization/barrier.cc \
	external/abseil-cpp/absl/synchronization/blocking_counter.cc \
	external/abseil-cpp/absl/synchronization/internal/create_thread_identity.cc \
	external/abseil-cpp/absl/synchronization/internal/per_thread_sem.cc \
	external/abseil-cpp/absl/synchronization/internal/waiter.cc \
	external/abseil-cpp/absl/synchronization/notification.cc \
	external/abseil-cpp/absl/synchronization/mutex.cc
# absl::graphcycles_internal
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/synchronization/internal/graphcycles.cc
# absl::time
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/time/civil_time.cc \
	external/abseil-cpp/absl/time/clock.cc \
	external/abseil-cpp/absl/time/duration.cc \
	external/abseil-cpp/absl/time/format.cc \
	external/abseil-cpp/absl/time/time.cc
# absl::time_zone
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_fixed.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_format.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_if.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_impl.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_info.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_libc.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_lookup.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/time_zone_posix.cc \
	external/abseil-cpp/absl/time/internal/cctz/src/zone_info_source.cc
# absl::stacktrace
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/debugging/stacktrace.cc
# absl::symbolize
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/debugging/symbolize.cc
# absl::demangle_internal
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/debugging/internal/demangle.cc
# absl::debugging_internal
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/debugging/internal/address_is_readable.cc \
	external/abseil-cpp/absl/debugging/internal/elf_mem_image.cc \
	external/abseil-cpp/absl/debugging/internal/vdso_support.cc
# absl::hash
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/hash/internal/hash.cc
# absl::city
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/hash/internal/city.cc
# absl::low_level_hash
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/hash/internal/low_level_hash.cc
# absl::int128
SFIZZ_SOURCES += \
	external/abseil-cpp/absl/numeric/int128.cc

ifdef SFIZZ_OS_WINDOWS
SFIZZ_LINK_FLAGS += -ldbghelp
endif

### Spline dependency

SFIZZ_C_FLAGS += -I$(SFIZZ_DIR)/src/external/spline
SFIZZ_SOURCES += src/external/spline/spline/spline.cpp

### Cpuid dependency

SFIZZ_C_FLAGS += \
	-I$(SFIZZ_DIR)/src/external/cpuid/src \
	-I$(SFIZZ_DIR)/src/external/cpuid/platform/src
SFIZZ_SOURCES += \
	src/external/cpuid/src/cpuid/cpuinfo.cpp \
	src/external/cpuid/src/cpuid/version.cpp

### simde dependency
SFIZZ_C_FLAGS += \
	-I$(SFIZZ_DIR)/external/simde

### Pugixml dependency

SFIZZ_C_FLAGS += -I$(SFIZZ_DIR)/src/external/pugixml/src
SFIZZ_SOURCES += src/external/pugixml/src/pugixml.cpp

### Kissfft dependency

SFIZZ_C_FLAGS += \
	-I$(SFIZZ_DIR)/src/external/kiss_fft
SFIZZ_SOURCES += \
	src/external/kiss_fft/kiss_fft.c \
	src/external/kiss_fft/kiss_fftr.c

### Surge tuning library dependency

SFIZZ_CXX_FLAGS += \
	-I$(SFIZZ_DIR)/src/external/tunings/include
SFIZZ_SOURCES += \
	src/external/tunings/src/Tunings.cpp

### jsl dependency

SFIZZ_CXX_FLAGS += \
	-I$(SFIZZ_DIR)/external/jsl/include

### cephes dependency

SFIZZ_SOURCES += \
	external/cephes/src/chbevl.c \
	external/cephes/src/i0.c

### math dependency

ifdef SFIZZ_OS_LINUX
SFIZZ_LINK_FLAGS += -lm
endif

### pthread dependency

ifdef SFIZZ_OS_LINUX
SFIZZ_C_FLAGS += -pthread
SFIZZ_CXX_FLAGS += -pthread
SFIZZ_LINK_FLAGS += -pthread
endif

### OpenMP dependency

SFIZZ_C_FLAGS += -fopenmp
SFIZZ_CXX_FLAGS += -fopenmp
SFIZZ_LINK_FLAGS += -fopenmp
