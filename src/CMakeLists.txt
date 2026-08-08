add_executable(kcast
        main.cpp
        peermodel.cpp
        streampipeline.cpp
        streampipeline.h
)

ecm_add_qml_module(kcast URI org.kde.kcast)
ecm_target_qml_sources(kcast SOURCES ui/Main.qml)
ecm_finalize_qml_module(kcast)

target_sources(kcast PRIVATE
    p2pdiscovery.cpp
    screencastportal.cpp
)

target_link_libraries(kcast PRIVATE
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
)