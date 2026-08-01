import QtCore
import QtQml
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: app

    minimumWidth: Kirigami.Units.gridUnit * 34
    minimumHeight: Kirigami.Units.gridUnit* 27
    width: Kirigami.Units.gridUnit * 55
    height: Kirigami.Units.gridUnit * 39

    title: "KCast"

    pageStack.initialPage: Kirigami.Page {
        title: i18nc("@title", "KCast")

        ColumnLayout {
            anchors.fill: parent

            Controls.Label {
                text: p2p.statusMessage
                Layout.fillWidth: true
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: p2p.peers
                delegate: Controls.ItemDelegate {
                    width: ListView.view.width
                    text: model.hasWfd ? model.name  + i18nc("@item:inlistbox", " (WFD)") : model.name + i18nc("@item:inlistbox", " (P2P)")
                    onClicked: p2p.connectToPeer(model.mac)
                }
            }
        }

        actions: [
            Kirigami.Action {
                text: "Discover"
                icon.name: "view-refresh"
                onTriggered: p2p.startDiscovery()
            }
        ]
    }
}


