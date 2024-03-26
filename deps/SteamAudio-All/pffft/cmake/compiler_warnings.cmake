
function(target_activate_cxx_compiler_warnings target)
    target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>)
    target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)
endfunction()

function(target_activate_c_compiler_warnings target)
    target_compile_options(${target} PRIVATE $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>)
    target_compile_options(${target} PRIVATE $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)
endfunction()

