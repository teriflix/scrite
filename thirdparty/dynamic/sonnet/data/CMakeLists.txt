include(ECMMarkNonGuiExecutable)

if(NOT CMAKE_CROSSCOMPILING)
    add_executable(parsetrigrams parsetrigrams.cpp)
    set_target_properties(parsetrigrams PROPERTIES OUTPUT_NAME parsetrigrams6)
    add_executable(gentrigrams gentrigrams.cpp)

    # Mark it as non-gui so we won't create an app bundle on Mac OS X
    ecm_mark_nongui_executable(parsetrigrams)
    ecm_mark_nongui_executable(gentrigrams)

    TARGET_LINK_LIBRARIES(parsetrigrams PUBLIC Qt6::Core)
    TARGET_LINK_LIBRARIES(gentrigrams PUBLIC Qt6::Core KF6::SonnetCore)
    INSTALL(TARGETS parsetrigrams ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

    add_executable(KF6::parsetrigrams ALIAS parsetrigrams)
else()
    if (NOT PARSETRIGRAMS_EXECUTABLE AND KF6_HOST_TOOLING)
        find_program(PARSETRIGRAMS_EXECUTABLE parsetrigrams6 PATHS ${KF6_HOST_TOOLING}/${KDE_INSTALL_BINDIR} NO_DEFAULT_PATH)
    endif()
    if (PARSETRIGRAMS_EXECUTABLE)
        add_executable(KF6::parsetrigrams IMPORTED GLOBAL)
        set_target_properties(KF6::parsetrigrams PROPERTIES IMPORTED_LOCATION ${PARSETRIGRAMS_EXECUTABLE})
    endif()
endif()
