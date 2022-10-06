#
# A build file to help using sfizz as a static library
# ----------------------------------------------------

SFIZZ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SFIZZ_BUILD_DIR := $(SFIZZ_DIR)/generic-build
SFIZZ_PKG_CONFIG ?= $(PKG_CONFIG)
include common.mk

###

SFIZZ_TARGET_MACHINE := $(shell $(CC) -dumpmachine)
SFIZZ_TARGET_PROCESSOR := $(firstword $(subst -, ,$(SFIZZ_TARGET_MACHINE)))

ifneq (,$(filter i%86,$(SFIZZ_TARGET_PROCESSOR)))
SFIZZ_CPU_I386=1
SFIZZ_CPU_I386_OR_X86_64=1
endif
ifneq (,$(filter x86_64,$(SFIZZ_TARGET_PROCESSOR)))
SFIZZ_CPU_X86_64=1
SFIZZ_CPU_I386_OR_X86_64=1
endif

###

SFIZZ_TARGET := $(SFIZZ_BUILD_DIR)/libsfizz.a

###

all: lib

lib: $(SFIZZ_TARGET)

clean:
	rm -rf $(SFIZZ_BUILD_DIR)

.PHONY: all lib clean

###

SFIZZ_OBJECTS = $(SFIZZ_SOURCES:%=$(SFIZZ_BUILD_DIR)/%.o)

$(SFIZZ_BUILD_DIR)/libsfizz.a: $(SFIZZ_OBJECTS)
	-@mkdir -p $(dir $@)
	$(AR) crs $@ $^

###

ifeq ($(SFIZZ_CPU_I386_OR_X86_64),1)

$(SFIZZ_BUILD_DIR)/%SSE.cpp.o: $(SFIZZ_DIR)/%SSE.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -msse -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%AVX.cpp.o: $(SFIZZ_DIR)/%AVX.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -mavx -c -o $@ $<

endif

###

$(SFIZZ_BUILD_DIR)/%.cpp.o: $(SFIZZ_DIR)/%.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%.cc.o: $(SFIZZ_DIR)/%.cc
	-@mkdir -p $(dir $@)
	$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%.c.o: $(SFIZZ_DIR)/%.c
	-@mkdir -p $(dir $@)
	$(CC) $(BUILD_C_FLAGS) $(SFIZZ_C_FLAGS) -c -o $@ $<

-include $(SFIZZ_OBJECTS:%.o=%.d)
