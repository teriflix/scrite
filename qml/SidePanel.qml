/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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

Item {
    id: sidePanel

    property real buttonY: 0
    property string label: ""
    property alias buttonColor: expandCollapseButton.color
    property alias backgroundColor: panelBackground.color
    property color borderColor: primaryColors.borderColor
    property real borderWidth: 1

    property bool expanded: false
    property alias content: contentLoader.sourceComponent
    property alias contentInstance: contentLoader.item

    width: expanded ? maxPanelWidth : minPanelWidth

    property real buttonSize: Math.min(100, height)
    readonly property real minPanelWidth: 25
    property real maxPanelWidth: 450
    Behavior on width {
        enabled: applicationSettings.enableAnimations
        NumberAnimation { duration: 50 }
    }

    BorderImage {
        source: "../icons/content/shadow.png"
        anchors.fill: panelBackground
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
        border { left: 21; top: 21; right: 21; bottom: 21 }
        opacity: 0.25
        visible: contentLoader.visible
    }

    Rectangle {
        id: panelBackground
        anchors.fill: parent
        color: "white"
        visible: contentLoader.visible
        opacity: contentLoader.opacity
        border.color: borderColor
        border.width: borderWidth
    }

    Item {
        id: contentLoaderArea
        anchors.top: parent.top
        anchors.left: expandCollapseButton.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        anchors.leftMargin: 0
        clip: true

        Rectangle {
            id: textLabelBackground
            anchors.fill: textLabel
            color: Qt.darker(expandCollapseButton.color)
            visible: textLabel.visible
            opacity: textLabel.opacity
        }

        Text {
            id: textLabel
            anchors.top: parent.top
            text: sidePanel.label
            leftPadding: 5; rightPadding: 5
            topPadding: 8; bottomPadding: 8
            font.bold: true
            font.pixelSize: expandCollapseButton.width * 0.45
            font.capitalization: Font.AllUppercase
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
            elide: Text.ElideRight
            opacity: contentLoader.opacity
            visible: contentLoader.visible && text !== ""
            color: Scrite.app.isLightColor(textLabelBackground.color) ? "black" : "white"
        }

        Loader {
            id: contentLoader
            anchors.top: textLabel.visible ? textLabel.bottom : parent.top
            anchors.bottom: parent.bottom
            width: sidePanel.maxPanelWidth - expandCollapseButton.width
            visible: opacity > 0
            opacity: sidePanel.expanded ? 1 : 0
            Behavior on opacity {
                enabled: applicationSettings.enableAnimations
                NumberAnimation { duration: 50 }
            }
            active: opacity > 0
        }
    }

    Rectangle {
        id: expandCollapseButton
        x: sidePanel.expanded ? 4 : -radius
        y: sidePanel.expanded ? 4 : parent.buttonY
        color: primaryColors.button.background
        width: parent.minPanelWidth
        height: sidePanel.expanded ? parent.height-8 : sidePanel.buttonSize
        radius: (1.0-contentLoader.opacity) * 6
        border.width: contentLoader.visible ? 0 : 1
        border.color: sidePanel.expanded ? primaryColors.windowColor : borderColor

        Behavior on height {
            enabled: applicationSettings.enableAnimations
            NumberAnimation { duration: 50 }
        }

        Item {
            width: parent.width
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: sidePanel.expanded ? 0 : parent.radius/2

            Image {
                id: iconImage
                width: parent.width
                height: width
                anchors.centerIn: parent
                source: sidePanel.expanded ? "../icons/navigation/arrow_left.png" : "../icons/navigation/arrow_right.png"
                fillMode: Image.PreserveAspectFit
            }
        }

        MouseArea {
            onClicked: sidePanel.expanded = !sidePanel.expanded
            anchors.fill: parent
        }

        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: Math.abs(parent.x)
            width: 1
            color: borderColor
            visible: parent.x < 0
        }
    }
}
