import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: app

    minimumWidth: Kirigami.Units.gridUnit * 34
    minimumHeight: Kirigami.Units.gridUnit * 27
    width: Kirigami.Units.gridUnit * 55
    height: Kirigami.Units.gridUnit * 39

    title: i18nc("@title:window", "KCast")

    pageStack.initialPage: Kirigami.ScrollablePage {
        id: findPage
        title: i18nc("@title", "KCast")

        header: Kirigami.InlineMessage {
            visible: P2PDiscovery.statusMessage.length > 0 && pageStack.depth === 1
            text: P2PDiscovery.statusMessage
            type: P2PDiscovery.state === P2PDiscovery.Error
                ? Kirigami.MessageType.Error
                : Kirigami.MessageType.Information
            position: Kirigami.InlineMessage.Position.Header
        }

        ListView {
            id: peerList
            clip: true
            reuseItems: true
            model: P2PDiscovery.peers

            delegate: Controls.ItemDelegate {
                width: ListView.view.width
                onClicked: P2PDiscovery.connectToPeer(model.mac)

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: model.hasWfd ? "video-display" : "network-wireless"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Controls.Label {
                            Layout.fillWidth: true
                            text: model.name
                            elide: Text.ElideRight
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            text: model.hasWfd
                                ? i18nc("@item:inlistbox subtitle", "Miracast · %1", model.mac)
                                : i18nc("@item:inlistbox subtitle", "P2P only · %1", model.mac)
                            opacity: 0.7
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 4
                visible: peerList.count === 0
                icon.name: "network-wireless"
                text: i18nc("@info:placeholder", "No devices found")
                explanation: i18nc("@info:placeholder", "Press Discover to search for nearby displays.")
            }
        }

        actions: [
            Kirigami.Action {
                text: i18nc("@action:button", "Discover")
                icon.name: "view-refresh"
                enabled: P2PDiscovery.state !== P2PDiscovery.Connecting
                onTriggered: P2PDiscovery.startDiscovery()
            }
        ]
    }

    Component {
        id: connectPageComponent
        Kirigami.Page {
            id: connectPage
            title: P2PDiscovery.state === P2PDiscovery.Connected
                ? i18nc("@title", "Connected")
                : P2PDiscovery.state === P2PDiscovery.Error
                    ? i18nc("@title", "Connection failed")
                    : i18nc("@title", "Connecting")

            ColumnLayout {
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 4
                spacing: Kirigami.Units.largeSpacing

                Controls.BusyIndicator {
                    Layout.alignment: Qt.AlignHCenter
                    running: P2PDiscovery.state === P2PDiscovery.Connecting
                    visible: running
                }

                Kirigami.Icon {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                    Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                    visible: P2PDiscovery.state !== P2PDiscovery.Connecting
                    source: P2PDiscovery.state === P2PDiscovery.Connected
                        ? "network-wireless-connected"
                        : "dialog-error"
                }

                Controls.Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: P2PDiscovery.statusMessage
                }
            }
        }
    }

    Connections {
        target: P2PDiscovery
        function onStateChanged() {
            if (P2PDiscovery.state === P2PDiscovery.Connecting) {
                if (pageStack.depth === 1) {
                    pageStack.push(connectPageComponent)
                }
            } else if (P2PDiscovery.state === P2PDiscovery.Connected) {
            } else if (P2PDiscovery.state === P2PDiscovery.Error) {
            } else if (P2PDiscovery.state === P2PDiscovery.Idle
                    || P2PDiscovery.state === P2PDiscovery.Discovering) {
                if (pageStack.depth > 1) {
                    pageStack.pop()
                }
            }
        }
    }
}
