qt_add_plugin(sonnet_voikko CLASS_NAME VoikkoClient)

target_sources(sonnet_voikko PRIVATE
    voikkoclient.cpp
    voikkodict.cpp
)

ecm_qt_declare_logging_category(sonnet_voikko
    HEADER voikkodebug.h
    IDENTIFIER SONNET_VOIKKO
    CATEGORY_NAME kf.sonnet.clients.voikko
    OLD_CATEGORY_NAMES sonnet.plugins.voikko
    DESCRIPTION "Sonnet Voikko plugin"
    EXPORT SONNET
)

target_include_directories(sonnet_voikko PRIVATE ${VOIKKO_INCLUDE_DIR})

target_link_libraries(sonnet_voikko PRIVATE KF6::SonnetCore ${VOIKKO_LIBRARIES})

install(TARGETS sonnet_voikko  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/sonnet/)

set(SONNET_BACKEND_FOUND TRUE PARENT_SCOPE)
