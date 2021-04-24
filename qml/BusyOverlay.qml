import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

Rectangle {
    id: busyOverlay
    anchors.fill: parent
    color: primaryColors.windowColor
    opacity: 0.9
    visible: false

    property string busyMessage: "Busy Doing Something..."

    Rectangle {
        anchors.fill: busyOverlayNotice
        anchors.margins: -30
        radius: 4
        color: primaryColors.c600.background
    }

    Column {
        id: busyOverlayNotice
        spacing: 10
        width: parent.width * 0.8
        anchors.centerIn: parent

        BusyIndicator {
            running: busyOverlay.visible
            anchors.horizontalCenter: parent.horizontalCenter
            Material.accent: primaryColors.c600.text
        }

        Text {
            width: parent.width
            font.pointSize: app.idealFontPointSize
            horizontalAlignment: Text.AlignHCenter
            text: busyMessage
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            color: primaryColors.c600.text
        }
    }

    MouseArea {
        anchors.fill: parent
    }
}
