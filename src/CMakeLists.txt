add_executable(kbeam
        main.cpp
)

ecm_add_qml_module(kbeam URI org.kde.kbeam)
ecm_target_qml_sources(kbeam SOURCES ui/Main.qml)
ecm_finalize_qml_module(kbeam)

target_sources(kbeam PRIVATE
    p2pdiscovery.cpp
    screencastportal.cpp
    streampipeline.cpp
    wfdserver.cpp
    wfdclientsession.cpp
    wfdclientsession.h
    peermodel.cpp
)

target_link_libraries(kbeam PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
    KF6::NetworkManagerQt
    PkgConfig::GSTREAMER
    PkgConfig::GSTREAMER_RTSP
    KF6::ItemModels
    PkgConfig::GSTREAMER_RTSP_SERVER
)