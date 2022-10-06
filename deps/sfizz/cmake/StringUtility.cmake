# SPDX-License-Identifier: BSD-2-Clause

function(string_left_pad VAR INPUT LENGTH FILLCHAR)
    set(_output "${INPUT}")
    string(LENGTH "${_output}" _length)
    while(_length LESS "${LENGTH}")
        set(_output "${FILLCHAR}${_output}")
        string(LENGTH "${_output}" _length)
    endwhile()
    set("${VAR}" "${_output}" PARENT_SCOPE)
endfunction()
