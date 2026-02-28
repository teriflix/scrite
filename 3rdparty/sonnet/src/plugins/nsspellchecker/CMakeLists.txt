qt_add_plugin(sonnet_nsspellchecker CLASS_NAME NSSpellCheckerClient)

target_sources(sonnet_nsspellchecker PRIVATE
    nsspellcheckerclient.mm
    nsspellcheckerdict.mm
)

ecm_qt_declare_logging_category(sonnet_nsspellchecker
    HEADER nsspellcheckerdebug.h
    IDENTIFIER SONNET_NSSPELLCHECKER
    CATEGORY_NAME kf.sonnet.clients.nsspellchecker
    OLD_CATEGORY_NAMES sonnet.plugins.nsspellchecker
    DESCRIPTION "Sonnet NSSpellChecker plugin"
    EXPORT SONNET
)

target_link_libraries(sonnet_nsspellchecker PRIVATE KF6::SonnetCore "-framework AppKit")

install(TARGETS sonnet_nsspellchecker  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/sonnet/)

set(SONNET_BACKEND_FOUND TRUE PARENT_SCOPE)
