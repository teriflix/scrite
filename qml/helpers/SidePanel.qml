/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    property bool expanded: false

    property real buttonY: 0
    property real borderWidth: 1
    property real maxPanelWidth: 450
    property real buttonSize: Math.min(100, height)

    property color borderColor: Runtime.colors.primary.borderColor

    property string label: ""

    property alias content: _contentLoader.sourceComponent
    property alias buttonColor: _expandCollapseButton.color
    property alias contentData: _contentLoader.contentData
    property alias backgroundColor: _background.color
    property alias cornerInstance: _cornerLoader.item
    property alias contentInstance: _contentLoader.item
    property alias cornerComponent: _cornerLoader.sourceComponent

    readonly property real minPanelWidth: 25

    width: expanded ? maxPanelWidth : minPanelWidth

    Behavior on width {
        enabled: Runtime.applicationSettings.enableAnimations
        NumberAnimation { duration: 50 }
    }

    BorderImage {
        anchors.fill: _background
        anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }

        border { left: 21; top: 21; right: 21; bottom: 21 }

        source: "qrc:/icons/content/shadow.png"
        opacity: 0.25
        visible: _contentLoader.visible
        verticalTileMode: BorderImage.Stretch
        horizontalTileMode: BorderImage.Stretch
    }

    Rectangle {
        id: _background

        anchors.fill: parent

        color: Runtime.colors.primary.c50.background
        visible: _contentLoader.visible
        opacity: _contentLoader.opacity
        border.color: borderColor
        border.width: borderWidth
    }

    Item {
        id: _contentArea

        anchors.top: parent.top
        anchors.left: _expandCollapseButton.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        anchors.leftMargin: 0

        clip: true

        Rectangle {
            id: _labelBackgroud

            anchors.fill: _label

            color: Qt.darker(_expandCollapseButton.color)
            visible: _label.visible
            opacity: _label.opacity
        }

        VclText {
            id: _label

            anchors.top: parent.top

            width: parent.width

            text: root.label
            elide: Text.ElideRight
            color: Scrite.app.isLightColor(_labelBackgroud.color) ? "black" : "white"
            opacity: _contentLoader.opacity
            visible: _contentLoader.visible && text !== ""
            horizontalAlignment: Text.AlignHCenter

            topPadding: 8
            leftPadding: 5
            rightPadding: 5
            bottomPadding: 8

            font.bold: true
            font.pixelSize: _expandCollapseButton.width * 0.45
            font.capitalization: Font.AllUppercase
        }

        Loader {
            id: _contentLoader

            anchors.top: _label.visible ? _label.bottom : parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: _label.visible ? 2 : 0

            active: opacity > 0
            visible: opacity > 0
            opacity: root.expanded ? 1 : 0

            property var contentData
            Behavior on opacity {
                enabled: Runtime.applicationSettings.enableAnimations
                NumberAnimation { duration: 50 }
            }
        }
    }

    Rectangle {
        id: _expandCollapseButton

        x: root.expanded ? 4 : -radius
        y: root.expanded ? 4 : parent.buttonY
        width: root.minPanelWidth
        height: root.expanded ? parent.height-8 : root.buttonSize

        color: Runtime.colors.primary.button.background
        radius: (1.0-_contentLoader.opacity) * 6
        border.width: _contentLoader.visible ? 0 : 1
        border.color: root.expanded ? Runtime.colors.primary.windowColor : borderColor

        Behavior on height {
            enabled: Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: 50 }
        }

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: root.expanded ? 0 : parent.radius/2

            width: parent.width
            height: parent.height

            Image {
                id: _icon

                anchors.centerIn: parent

                width: parent.width
                height: width

                source: root.expanded ? "qrc:/icons/navigation/arrow_left.png" : "qrc:/icons/navigation/arrow_right.png"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent

                onClicked: root.expanded = !root.expanded
            }

            Loader {
                id: _cornerLoader

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: _icon.top
                // anchors.topMargin: 2
                anchors.leftMargin: -2
                anchors.bottomMargin: 2
            }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: Math.abs(parent.x)

            width: 1

            color: borderColor
            visible: parent.x < 0
        }
    }
}
