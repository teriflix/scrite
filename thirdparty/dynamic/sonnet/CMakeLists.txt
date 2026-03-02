cmake_minimum_required(VERSION 3.27)

set(KF_VERSION "6.23.0") # handled by release scripts
project(Sonnet VERSION ${KF_VERSION})

include(FeatureSummary)
include(CMakeDependentOption)
find_package(ECM 6.23.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGenerateQmlTypes)
include(ECMFindQmlModule)
include(ECMDeprecationSettings)
include(ECMPoQmTools)
include(CMakeDependentOption)
include(ECMGenerateQDoc)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(REQUIRED_QT_VERSION 6.8.0)
option(SONNET_USE_WIDGETS "Build components using Qt6Widgets" ON)
option(SONNET_USE_QML "Build components using Qt6Quick" ON)
option(SONNET_NO_BACKENDS "Do not fail when no backends are built (to be used for cross-build host tooling)" OFF)

if(SONNET_USE_WIDGETS)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
endif()

if(SONNET_USE_QML)
    include(ECMQmlModule)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Quick)
endif()

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core)

ecm_setup_version(PROJECT VARIABLE_PREFIX SONNET
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/sonnet_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6SonnetConfigVersion.cmake"
                        SOVERSION 6)

if(SONNET_USE_WIDGETS)
    cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
    add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")
endif()

ecm_install_po_files_as_qm(poqm)

ecm_set_disabled_deprecation_versions(
    QT 6.11.0
)

if (NOT BUILD_SHARED_LIBS)
    add_definitions(-DSONNET_STATIC)
endif()

add_subdirectory(data)
add_subdirectory(src)

if (BUILD_TESTING)
     add_subdirectory(autotests)
endif()

cmake_dependent_option(BUILD_EXAMPLES "Whether to build the examples" OFF "SONNET_USE_WIDGETS" OFF)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Sonnet")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6SonnetConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/KF6SonnetConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KF6SonnetConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KF6SonnetConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  COMPONENT Devel )

install(EXPORT KF6SonnetTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6SonnetTargets.cmake NAMESPACE KF6:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sonnet_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Sonnet COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
