find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "https://www.zlib.net"
                       TYPE REQUIRED
                       PURPOSE "Required by the hspell sonnet plugin"
                       )

qt_add_plugin(sonnet_hspell CLASS_NAME HSpellClient)

target_sources(sonnet_hspell PRIVATE
    hspellclient.cpp
    hspelldict.cpp
)

target_include_directories(sonnet_hspell PRIVATE
    ${HSPELL_INCLUDE_DIR}
)

ecm_qt_declare_logging_category(sonnet_hspell
    HEADER hspell_debug.h
    IDENTIFIER SONNET_LOG_HSPELL
    CATEGORY_NAME kf.sonnet.clients.hspell
    OLD_CATEGORY_NAMES sonnet.plugins.hspell
    DESCRIPTION "Sonnet Hspell plugin"
    EXPORT SONNET
)

target_link_libraries(sonnet_hspell PRIVATE KF6::SonnetCore ${HSPELL_LIBRARIES} ZLIB::ZLIB)

install(TARGETS sonnet_hspell  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/sonnet/)

set(SONNET_BACKEND_FOUND TRUE PARENT_SCOPE)
