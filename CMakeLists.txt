cmake_minimum_required(VERSION 3.24)
project(kcast VERSION 0.1 LANGUAGES CXX)

set(QT_MIN_VERSION "6.5.0")
set(KF6_MIN_VERSION "6.0.0")

find_package (ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMQmlModule)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 DBus)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS Kirigami I18n CoreAddons NetworkManagerQt ItemModels)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_subdirectory(src)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)