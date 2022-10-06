# This option is for MIDI CC support in absence of host midi:binding support
option(SFIZZ_LV2_PSA "Enable plugin-side MIDI automations" ON)

# Configuration for this plugin
# TODO: generate version from git
set(LV2PLUGIN_VERSION_MINOR   10)
set(LV2PLUGIN_VERSION_MICRO   0)
set(LV2PLUGIN_NAME            "sfizz")
set(LV2PLUGIN_COMMENT         "SFZ sampler")
set(LV2PLUGIN_URI             "http://sfztools.github.io/sfizz")
set(LV2PLUGIN_REPOSITORY      SFIZZ_REPOSITORY)
set(LV2PLUGIN_AUTHOR          "SFZTools")
set(LV2PLUGIN_EMAIL           "paul@ferrand.cc")
if(SFIZZ_USE_VCPKG)
    set(LV2PLUGIN_SPDX_LICENSE_ID "LGPL-3.0-only")
else()
    set(LV2PLUGIN_SPDX_LICENSE_ID "ISC")
endif()

if(SFIZZ_LV2_UI)
    set(LV2PLUGIN_IF_ENABLE_UI "")
else()
    set(LV2PLUGIN_IF_ENABLE_UI "#")
endif()

if(WIN32)
    set(LV2_UI_TYPE "WindowsUI")
elseif(APPLE)
    set(LV2_UI_TYPE "CocoaUI")
elseif(HAIKU)
    set(LV2_UI_TYPE "BeUI")
else()
    set(LV2_UI_TYPE "X11UI")
endif()

if(APPLE)
    set(LV2PLUGIN_INSTALL_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/LV2" CACHE STRING
    "Install destination for LV2 bundle [default: $ENV{HOME}/Library/Audio/Plug-Ins/LV2]")
elseif(MSVC)
    set(LV2PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lv2" CACHE STRING
    "Install destination for LV2 bundle [default: ${CMAKE_INSTALL_PREFIX}/lv2]")
else()
    set(LV2PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/lv2" CACHE STRING
    "Install destination for LV2 bundle [default: ${CMAKE_INSTALL_PREFIX}/lib/lv2]")
endif()

include(StringUtility)

function(sfizz_lv2_generate_controllers_ttl FILE)
    file(WRITE "${FILE}" "# LV2 parameters for SFZ controllers
@prefix atom:   <http://lv2plug.in/ns/ext/atom#> .
@prefix lv2:    <http://lv2plug.in/ns/lv2core#> .
@prefix midi:   <http://lv2plug.in/ns/ext/midi#> .
@prefix patch:  <http://lv2plug.in/ns/ext/patch#> .
@prefix rdfs:   <http://www.w3.org/2000/01/rdf-schema#> .
@prefix sfizz:  <${LV2PLUGIN_URI}#> .
")
    math(EXPR _j "${SFIZZ_NUM_CCS}-1")
    foreach(_i RANGE "${_j}")
        if(_i LESS 128 AND SFIZZ_LV2_PSA)
            continue() # Don't generate automation parameters for CCs with plugin-side automation
        endif()

        string_left_pad(_i "${_i}" 3 0)
        file(APPEND "${FILE}" "
sfizz:cc${_i}
  a lv2:Parameter ;
  rdfs:label \"Controller ${_i}\" ;
  rdfs:range atom:Float ;
  lv2:minimum 0.0 ;
  lv2:maximum 1.0")

        if(_i LESS 128 AND NOT SFIZZ_LV2_PSA)
            math(EXPR _digit1 "${_i}>>4")
            math(EXPR _digit2 "${_i}&15")
            string(SUBSTRING "0123456789ABCDEF" "${_digit1}" 1 _digit1)
            string(SUBSTRING "0123456789ABCDEF" "${_digit2}" 1 _digit2)
            file(APPEND "${FILE}" " ;
  midi:binding \"B0${_digit1}${_digit2}00\"^^midi:MidiEvent .
")
        else()
            file(APPEND "${FILE}" " .
")
        endif()
    endforeach()

    file(APPEND "${FILE}" "
<${LV2PLUGIN_URI}>
  a lv2:Plugin ;
")

    file(APPEND "${FILE}" "  patch:readable sfizz:cc000")
    foreach(_i RANGE 1 "${_j}")
        string_left_pad(_i "${_i}" 3 0)
        file(APPEND "${FILE}" ", sfizz:cc${_i}")
    endforeach()
    file(APPEND "${FILE}" " ;
")

    file(APPEND "${FILE}" "  patch:writable sfizz:cc000")
    foreach(_i RANGE 1 "${_j}")
        string_left_pad(_i "${_i}" 3 0)
        file(APPEND "${FILE}" ", sfizz:cc${_i}")
    endforeach()
    file(APPEND "${FILE}" " .
")
endfunction()
