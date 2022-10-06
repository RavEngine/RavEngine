
#
# A build file to help using sfizz with the DISTRHO Plugin Framework (DPF)
# ------------------------------------------------------------------------
#
# Usage notes:
#
# 1. Start with the template DPF project
#
#    (for example https://github.com/SpotlightKid/cookiecutter-dpf-effect)
#
# 2. In the top directory of the DPF project, edit `Makefile`
#
#    Add the following line, after the `all` rule definition:
#
#        include <path-to-sfizz>/dpf.mk
#
#    Add `sfizz-lib` to the `libs` rule definition:
#
#        libs: sfizz-lib
#
#    If you want you can add `sfizz-clean` to the `clean` rule.
#
#        clean: sfizz-clean
#
# 3. In the Makefile of your plugin folder, eg. `plugins/MyPlugin/Makefile`
#
#    Add the following line, after `include ../../dpf/Makefile.plugins.mk`
#
#        include <path-to-sfizz>/dpf.mk
#        BUILD_C_FLAGS += $(SFIZZ_C_FLAGS)
#        BUILD_CXX_FLAGS += $(SFIZZ_CXX_FLAGS)
#        LINK_FLAGS += $(SFIZZ_LINK_FLAGS)
#
# 4. Build using `make` from the top folder.
#

SFIZZ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SFIZZ_BUILD_DIR := $(SFIZZ_DIR)/dpf-build
SFIZZ_LINK_FLAGS = $(SFIZZ_BUILD_DIR)/libsfizz.a
SFIZZ_PKG_CONFIG ?= $(PKG_CONFIG)
include $(SFIZZ_DIR)/common.mk

sfizz-all: sfizz-lib

sfizz-lib: $(SFIZZ_BUILD_DIR)/libsfizz.a

sfizz-clean:
	rm -rf $(SFIZZ_BUILD_DIR)

.PHONY: sfizz-all sfizz-lib sfizz-clean

###

SFIZZ_OBJECTS = $(SFIZZ_SOURCES:%=$(SFIZZ_BUILD_DIR)/%.o)

$(SFIZZ_BUILD_DIR)/libsfizz.a: $(SFIZZ_OBJECTS)
	-@mkdir -p $(dir $@)
	@echo "Creating $@"
	$(SILENT)$(AR) crs $@ $^

###

ifeq ($(CPU_I386_OR_X86_64),true)

$(SFIZZ_BUILD_DIR)/%SSE.cpp.o: $(SFIZZ_DIR)/%SSE.cpp
	-@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(SILENT)$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -msse -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%AVX.cpp.o: $(SFIZZ_DIR)/%AVX.cpp
	-@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(SILENT)$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -mavx -c -o $@ $<

endif

###

$(SFIZZ_BUILD_DIR)/%.cpp.o: $(SFIZZ_DIR)/%.cpp
	-@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(SILENT)$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%.cc.o: $(SFIZZ_DIR)/%.cc
	-@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(SILENT)$(CXX) $(BUILD_CXX_FLAGS) $(SFIZZ_CXX_FLAGS) -c -o $@ $<

$(SFIZZ_BUILD_DIR)/%.c.o: $(SFIZZ_DIR)/%.c
	-@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(SILENT)$(CC) $(BUILD_C_FLAGS) $(SFIZZ_C_FLAGS) -c -o $@ $<

-include $(SFIZZ_OBJECTS:%.o=%.d)
