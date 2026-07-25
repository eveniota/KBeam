import QtCore
import QtQml
import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: app

    minimumWidth: Kirigami.Units.gridUnit * 34
    minimumHeight: Kirigami.Units.gridUnit* 27
    width: Kirigami.Units.gridUnit * 55
    height: Kirigami.Units.gridUnit * 39

    title: "KCast"

    pageStack.initialPage: Kirigami.Page {
        Controls.Label {
            text: "Hello KCast"
        }
    }
}


