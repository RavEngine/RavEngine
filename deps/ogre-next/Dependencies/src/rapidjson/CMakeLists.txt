project(rapidjson)

set(rapidjson_HEADERS
    include/error/en.h
    include/error/error.h
    include/internal/biginteger.h
    include/internal/diyfp.h
    include/internal/dtoa.h
    include/internal/ieee754.h
    include/internal/itoa.h
    include/internal/meta.h
    include/internal/pow10.h
    include/internal/stack.h
    include/internal/strfunc.h
    include/internal/strtod.h
    include/internal/swap.h
    include/msinttypes/inttypes.h
    include/msinttypes/stdint.h
    include/allocators.h
    include/document.h
    include/encodings.h
    include/filereadstream.h
    include/memorystream.h
    include/pointer.h
    include/rapidjson.h
    include/stringbuffer.h
    include/encodedstream.h
    include/filewritestream.h
    include/memorybuffer.h
    include/prettywriter.h
    include/reader.h
    include/writer.h
)

#add_library( rapidjson ${rapidjson_HEADERS} )
#set_target_properties( rapidjson PROPERTIES LINKER_LANGUAGE CXX )
#install_dep(rapidjson include/rapidjson ${rapidjson_HEADERS})

if (OGREDEPS_INSTALL_DEV)
    install( DIRECTORY include/ DESTINATION include/rapidjson )
endif()

set(rapidjson_INCLUDE_DIR "${rapidjson_SOURCE_DIR}" CACHE PATH "" FORCE)
mark_as_advanced(rapidjson_INCLUDE_DIR)
