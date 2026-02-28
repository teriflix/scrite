
include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

set(ECM_TEST_NAME_PREFIX "sonnet-")

########### unittests ###############

set(sonnet_plugins)

if (NOT BUILD_SHARED_LIBS)
    set (sonnet_plugins sonnet_aspell sonnet_hunspell)
endif()


ecm_add_tests(
 test_filter.cpp
 test_core.cpp
 test_suggest.cpp
 test_settings.cpp
 test_autodetect.cpp
 LINK_LIBRARIES KF6::SonnetCore Qt6::Test ${sonnet_plugins}
)

ecm_add_tests(
 test_decorator.cpp
 test_highlighter.cpp
 LINK_LIBRARIES KF6::SonnetUi KF6::SonnetCore Qt6::Test ${sonnet_plugins}
)
