// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

Kirigami.ApplicationWindow {
    id: app

    minimumWidth: Kirigami.Units.gridUnit * 34
    minimumHeight: Kirigami.Units.gridUnit * 27
    width: Kirigami.Units.gridUnit * 55
    height: Kirigami.Units.gridUnit * 39

    title: i18nc("@title:window", "KBeam")

    pageStack.initialPage: Kirigami.ScrollablePage {
        id: findPage
        title: i18nc("@title", "KBeam")

        Component.onCompleted: {
            P2PDiscovery.startDiscovery();
        }

        readonly property string bannerText: {
            if (P2PDiscovery.state === P2PDiscovery.Connecting) {
                return P2PDiscovery.statusMessage;
            }
            if (P2PDiscovery.state === P2PDiscovery.Connected) {
                const name = P2PDiscovery.activePeerName.length > 0 ? P2PDiscovery.activePeerName : P2PDiscovery.activePeerMac;
                return i18nc("@info:status", "Connected to %1 · %2", name, P2PDiscovery.ipv4Address);
            }
            if (ScreencastPortal.statusMessage.length > 0) {
                return ScreencastPortal.statusMessage;
            }
            return P2PDiscovery.statusMessage;
        }

        header: Kirigami.InlineMessage {
            visible: findPage.bannerText.length > 0
            text: findPage.bannerText
            type: {
                if (P2PDiscovery.state === P2PDiscovery.Error) {
                    return Kirigami.MessageType.Error;
                }
                if (P2PDiscovery.state === P2PDiscovery.Connected) {
                    return Kirigami.MessageType.Positive;
                }
                return Kirigami.MessageType.Information;
            }
            showCloseButton: P2PDiscovery.state === P2PDiscovery.Error
            position: Kirigami.InlineMessage.Position.Header

            actions: [
                Kirigami.Action {
                    text: i18nc("@action:button", "Disconnect")
                    icon.name: "network-disconnect"
                    visible: P2PDiscovery.state === P2PDiscovery.Connected || P2PDiscovery.state === P2PDiscovery.Connecting
                    onTriggered: P2PDiscovery.disconnectPeer()
                }
            ]
        }

        KItemModels.KSortFilterProxyModel {
            id: wfdPeerModel
            sourceModel: P2PDiscovery.peers
            filterRoleName: "hasWfd"
            filterRowCallback: function (sourceRow, sourceParent) {
                const idx = sourceModel.index(sourceRow, 0, sourceParent);
                return sourceModel.data(idx, 260) === true;
            }
        }

        ListView {
            id: peerList
            clip: true
            reuseItems: true
            model: wfdPeerModel

            delegate: Controls.ItemDelegate {
                id: delegateItem
                width: ListView.view.width
                enabled: P2PDiscovery.state !== P2PDiscovery.Connecting
                highlighted: P2PDiscovery.activePeerMac === model.mac && (P2PDiscovery.state === P2PDiscovery.Connecting || P2PDiscovery.state === P2PDiscovery.Connected)

                onClicked: {
                    if (P2PDiscovery.activePeerMac === model.mac && P2PDiscovery.state === P2PDiscovery.Connected) {
                        P2PDiscovery.disconnectPeer();
                    } else {
                        P2PDiscovery.connectToPeer(model.uni);
                    }
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.mediumSpacing

                    Kirigami.Icon {
                        source: {
                            if (P2PDiscovery.activePeerMac === model.mac && P2PDiscovery.state === P2PDiscovery.Connected) {
                                return "network-wireless-connected";
                            }
                            return model.hasWfd ? "video-display" : "network-wireless";
                        }
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
                            font.bold: delegateItem.highlighted
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            text: {
                                if (P2PDiscovery.activePeerMac === model.mac) {
                                    if (P2PDiscovery.state === P2PDiscovery.Connecting) {
                                        return i18nc("@item:inlistbox subtitle", "Connecting...");
                                    }
                                    if (P2PDiscovery.state === P2PDiscovery.Connected) {
                                        return i18nc("@item:inlistbox subtitle", "Connected · %1", P2PDiscovery.ipv4Address);
                                    }
                                }
                                return model.hasWfd
                                    ? i18nc("@item:inlistbox subtitle", "Miracast · %1", model.mac)
                                    : i18nc("@item:inlistbox subtitle", "P2P only · %1", model.mac);
                            }
                            opacity: 0.7
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                    }

                    Controls.BusyIndicator {
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        running: P2PDiscovery.activePeerMac === model.mac && P2PDiscovery.state === P2PDiscovery.Connecting
                        visible: running
                    }

                    Controls.Button {
                        visible: P2PDiscovery.activePeerMac === model.mac && P2PDiscovery.state === P2PDiscovery.Connected
                        text: i18nc("@action:button", "Disconnect")
                        icon.name: "network-disconnect"
                        onClicked: P2PDiscovery.disconnectPeer()
                    }
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 4
                visible: peerList.count === 0
                icon.name: P2PDiscovery.state === P2PDiscovery.Discovering ? "network-wireless" : "dialog-information"
                text: P2PDiscovery.state === P2PDiscovery.Discovering
                    ? i18nc("@info:placeholder", "Searching for displays...")
                    : i18nc("@info:placeholder", "No devices found")
                explanation: P2PDiscovery.state === P2PDiscovery.Discovering
                    ? i18nc("@info:placeholder", "Ensure the target display has wireless projection enabled.")
                    : i18nc("@info:placeholder", "Press Discover to search for nearby displays.")
            }
        }

        actions: [
            Kirigami.Action {
                text: P2PDiscovery.state === P2PDiscovery.Discovering
                    ? i18nc("@action:button", "Restart Discovery")
                    : i18nc("@action:button", "Discover")
                icon.name: "view-refresh"
                enabled: P2PDiscovery.state !== P2PDiscovery.Connecting
                onTriggered: P2PDiscovery.startDiscovery()
            },
            Kirigami.Action {
                text: i18nc("@action:button", "Cast")
                icon.name: "video-display"
                onTriggered: ScreencastPortal.start()
            },
            Kirigami.Action {
                text: i18nc("@action:button", "Disconnect")
                icon.name: "network-disconnect"
                visible: P2PDiscovery.state === P2PDiscovery.Connected || P2PDiscovery.state === P2PDiscovery.Connecting
                onTriggered: P2PDiscovery.disconnectPeer()
            }
        ]
    }
}
