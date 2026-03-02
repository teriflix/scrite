ecm_add_qml_module(sonnetquickplugin URI "org.kde.sonnet" VERSION 1.0 GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)

target_sources(sonnetquickplugin PRIVATE
    types.h
    spellcheckhighlighter.cpp
    spellcheckhighlighter.h )

ecm_qt_declare_logging_category(sonnetquickplugin
    HEADER quick_debug.h
    IDENTIFIER SONNET_LOG_QUICK
    CATEGORY_NAME kf.sonnet.quick
    OLD_CATEGORY_NAMES sonnet.quick
    DESCRIPTION "Sonnet Quick bindings"
    EXPORT SONNET
)
target_link_libraries(sonnetquickplugin
    PUBLIC Qt6::Quick
    PRIVATE KF6SonnetCore
)

ecm_finalize_qml_module(sonnetquickplugin DESTINATION ${KDE_INSTALL_QMLDIR})

ecm_generate_qdoc(sonnetquickplugin sonnetquick.qdocconf)
