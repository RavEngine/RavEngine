# SPDX-License-Identifier: BSD-2-Clause

# This code is part of the sfizz library and is licensed under a BSD 2-clause
# license. You should have receive a LICENSE.md file along with the code.
# If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

# Macro: option_ex(OPTION DOC [CONDITION])
#    Defines an option, with these characteristics:
#      - A suffix [default: ON/OFF] is appended to the documentation
#      - The value is interpreted like a conditional expression
macro(option_ex option doc)
    if(${ARGN})
        set(_value ON)
    else()
        set(_value OFF)
    endif()
    option("${option}" "${doc} [default: ${_value}]" "${_value}")
    unset(_value)
endmacro()
