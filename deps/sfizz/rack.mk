
#
# A build file to help using sfizz with the VCV Rack SDK
# ------------------------------------------------------
#
# Usage notes:
#
# 1. In the `dep` subfolder of your plugin folder,
#
#    Check out the sfizz source code as a submodule
#
#        git submodule add https://github.com/sfztools/sfizz.git
#
# 2. At the root of your plugin folder,
#
#    Add the following lines, at the bottom of `Makefile`:
#
#        # Include the sfizz library
#        include dep/sfizz/rack.mk
#        CFLAGS += $(SFIZZ_C_FLAGS)
#        CXXFLAGS += $(SFIZZ_CXX_FLAGS)
#        LDFLAGS += $(SFIZZ_LINK_FLAGS)
#        $(TARGET): $(SFIZZ_TARGET)
#
# 3. In the file `Makefile`,
#
#    Above the line `include dep/sfizz/rack.mk`, some configuration variables
#    may be customized.
#
#        SFIZZ_RACK_PLUGIN_DIR = <the root directory of the Rack plugin>
#        SFIZZ_PKG_CONFIG = <a custom pkg-config command>
#        SFIZZ_USE_SNDFILE = <0 disabled, 1 enabled (default)>
#        SFIZZ_SNDFILE_C_FLAGS = <compiler flags of sndfile for C>
#        SFIZZ_SNDFILE_CXX_FLAGS = <compiler flags of sndfile for C++>
#        SFIZZ_SNDFILE_LINK_FLAGS = <linker flags of sndfile>

ifndef RACK_DIR
$(error sfizz: We are not invoked from the Rack SDK)
endif

SFIZZ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SFIZZ_RACK_PLUGIN_DIR ?= .
ifneq ($(shell test -f $(SFIZZ_RACK_PLUGIN_DIR)/plugin.json && echo 1),1)
$(error sfizz: This is not a Rack plugin directory)
endif
SFIZZ_BUILD_DIR := $(SFIZZ_RACK_PLUGIN_DIR)/build/sfizz
include $(SFIZZ_DIR)/common.mk

###

SFIZZ_TARGET := $(SFIZZ_BUILD_DIR)/libsfizz.a

###

SFIZZ_OBJECTS = $(SFIZZ_SOURCES:%=$(SFIZZ_BUILD_DIR)/%.o)

$(SFIZZ_BUILD_DIR)/libsfizz.a: $(SFIZZ_OBJECTS)
	-@mkdir -p $(dir $@)
	$(AR) crs $@ $^

###

ifeq ($(SFIZZ_CPU_I386_OR_X86_64),1)

$(SFIZZ_BUILD_DIR)/%SSE.cpp.o: $(SFIZZ_DIR)/%SSE.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS) -msse2 -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%AVX.cpp.o: $(SFIZZ_DIR)/%AVX.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS) -mavx -c -o $@ $<

endif

###

$(SFIZZ_BUILD_DIR)/%.cpp.o: $(SFIZZ_DIR)/%.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS) -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%.cc.o: $(SFIZZ_DIR)/%.cc
	-@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS) -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%.c.o: $(SFIZZ_DIR)/%.c
	-@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS) -c -o $@ $<

-include $(SFIZZ_OBJECTS:%.o=%.d)
