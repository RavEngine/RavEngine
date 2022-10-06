if(TARGET Qt5::Widgets AND TARGET sfizz::jack)
    add_executable(sfizz_demo_filters DemoFilters.cpp)
    target_link_libraries(sfizz_demo_filters PRIVATE sfizz::internal sfizz::jack Qt5::Widgets)
    set_target_properties(sfizz_demo_filters PROPERTIES AUTOUIC ON)

    add_executable(sfizz_demo_smooth DemoSmooth.cpp)
    target_link_libraries(sfizz_demo_smooth PRIVATE sfizz::internal sfizz::jack Qt5::Widgets)
    set_target_properties(sfizz_demo_smooth PROPERTIES AUTOUIC ON)

    add_executable(sfizz_demo_stereo DemoStereo.cpp)
    target_link_libraries(sfizz_demo_stereo PRIVATE sfizz::internal sfizz::jack Qt5::Widgets)
    set_target_properties(sfizz_demo_stereo PROPERTIES AUTOUIC ON)

    add_executable(sfizz_demo_wavetables DemoWavetables.cpp)
    target_link_libraries(sfizz_demo_wavetables PRIVATE sfizz::internal sfizz::jack Qt5::Widgets)
    set_target_properties(sfizz_demo_wavetables PROPERTIES AUTOUIC ON)
endif()

if(TARGET Qt5::Widgets)
    add_executable(sfizz_demo_parser DemoParser.cpp)
    target_link_libraries(sfizz_demo_parser PRIVATE sfizz::parser Qt5::Widgets)
    set_target_properties(sfizz_demo_parser PROPERTIES AUTOUIC ON)

    add_executable(sfizz_demo_stretch_tuning DemoStretchTuning.cpp)
    target_link_libraries(sfizz_demo_stretch_tuning PRIVATE sfizz::internal Qt5::Widgets)
    set_target_properties(sfizz_demo_stretch_tuning PROPERTIES AUTOUIC ON)
endif()

add_executable(eq_apply EQ.cpp)
target_link_libraries(eq_apply PRIVATE sfizz::internal sfizz::sndfile sfizz::cxxopts sfizz::filesystem)

add_executable(filter_apply Filter.cpp)
target_link_libraries(filter_apply PRIVATE sfizz::internal sfizz::sndfile sfizz::cxxopts sfizz::filesystem)

add_executable(sfizz_plot_curve PlotCurve.cpp)
target_link_libraries(sfizz_plot_curve PRIVATE sfizz::internal)

add_executable(sfizz_plot_wavetables PlotWavetables.cpp)
target_link_libraries(sfizz_plot_wavetables PRIVATE sfizz::internal)

add_executable(sfizz_plot_lfo PlotLFO.cpp)
target_link_libraries(sfizz_plot_lfo PRIVATE sfizz::internal sfizz::sndfile sfizz::cxxopts)

add_executable(sfizz_file_instrument FileInstrument.cpp)
target_link_libraries(sfizz_file_instrument PRIVATE sfizz::internal sfizz::sndfile)

add_executable(sfizz_file_wavetable FileWavetable.cpp)
target_link_libraries(sfizz_file_wavetable PRIVATE sfizz::internal)

add_executable(sfizz_tuning Tuning.cpp)
target_link_libraries(sfizz_tuning PRIVATE sfizz::internal sfizz::cxxopts)
