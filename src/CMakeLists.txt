add_executable(kcast
        main.cpp)

target_sources(kcast PRIVATE
    main.cpp
)

target_link_libraries(kcast PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
)