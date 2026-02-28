add_subdirectory(core)
add_subdirectory(plugins)

if(SONNET_USE_WIDGETS)
    add_subdirectory(ui)
endif()

if(SONNET_USE_QML)
    add_subdirectory(quick)
endif()

ecm_qt_install_logging_categories(
    EXPORT SONNET
    FILE sonnet.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
