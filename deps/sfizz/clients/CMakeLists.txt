string(TIMESTAMP MAN_TODAY "%Y-%m-%d")

if(SFIZZ_JACK)
    add_executable(sfizz_jack MidiHelpers.h jack_client.cpp)
    target_link_libraries(sfizz_jack PRIVATE sfizz::import sfizz::sfizz sfizz::jack sfizz::spin_mutex absl::flags_parse)
    sfizz_enable_lto_if_needed(sfizz_jack)
    configure_file(sfizz_jack.man.in sfizz_jack.man @ONLY)
    install(TARGETS sfizz_jack DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT "jack" OPTIONAL)
    install(FILES ${PROJECT_BINARY_DIR}/clients/sfizz_jack.man DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
        RENAME sfizz_jack COMPONENT "jack" OPTIONAL)
endif()

if(SFIZZ_RENDER)
    add_executable(sfizz_render MidiHelpers.h sfizz_render.cpp)
    target_link_libraries(sfizz_render PRIVATE sfizz::internal sfizz::fmidi sfizz::cxxopts st_audiofile_formats)
    sfizz_enable_lto_if_needed(sfizz_render)
    configure_file(sfizz_render.man.in sfizz_render.man @ONLY)
    install(TARGETS sfizz_render DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT "render" OPTIONAL)
    install(FILES ${PROJECT_BINARY_DIR}/clients/sfizz_render.man DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
        RENAME sfizz_render COMPONENT "render" OPTIONAL)
endif()
