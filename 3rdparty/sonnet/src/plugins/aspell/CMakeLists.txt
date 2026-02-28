qt_add_plugin(sonnet_aspell CLASS_NAME ASpellClient)

target_sources(sonnet_aspell PRIVATE
    aspellclient.cpp
    aspelldict.cpp
)

target_include_directories(sonnet_aspell PRIVATE ${ASPELL_INCLUDE_DIR})

ecm_qt_declare_logging_category(sonnet_aspell
    HEADER aspell_debug.h
    IDENTIFIER SONNET_LOG_ASPELL
    CATEGORY_NAME kf.sonnet.clients.aspell
    OLD_CATEGORY_NAMES sonnet.plugins.aspell
    DESCRIPTION "Sonnet Aspell plugin"
    EXPORT SONNET
)

target_link_libraries(sonnet_aspell PRIVATE KF6::SonnetCore ${ASPELL_LIBRARIES})

install(TARGETS sonnet_aspell  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/sonnet/)

set(SONNET_BACKEND_FOUND TRUE PARENT_SCOPE)
