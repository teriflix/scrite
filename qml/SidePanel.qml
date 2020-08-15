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

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    id: sidePanel

    property real buttonY: 0
    property string buttonText: ""
    property alias buttonColor: expandCollapseButton.color
    property alias backgroundColor: panelBackground.color
    property color borderColor: primaryColors.borderColor

    property bool expanded: false
    property alias content: contentLoader.sourceComponent
    property alias contentInstance: contentLoader.item

    width: expanded ? maxPanelWidth : minPanelWidth

    property real buttonSize: {
        if(textLabel.text !== "")
            Math.min((textLabel.contentWidth + iconImage.width + 20) * 1.25, height)
        return Math.min(100, height)
    }
    readonly property real minPanelWidth: 25
    property real maxPanelWidth: 450
    Behavior on width {
        enabled: screenplayEditorSettings.enableAnimations
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
        visible: contentLoader.opacity === 1
    }

    Rectangle {
        id: panelBackground
        anchors.fill: parent
        color: "white"
        opacity: contentLoader.opacity
        border.color: borderColor
        border.width: 1
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

        Loader {
            id: contentLoader
            height: parent.height
            width: sidePanel.maxPanelWidth - expandCollapseButton.width
            visible: opacity > 0
            opacity: sidePanel.expanded ? 1 : 0
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
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
            enabled: screenplayEditorSettings.enableAnimations
            NumberAnimation { duration: 50 }
        }

        Item {
            width: parent.width
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: sidePanel.expanded ? 0 : parent.radius/2

            Text {
                id: textLabel
                text: sidePanel.buttonText
                rotation: -90
                font.pixelSize: parent.width * 0.45
                transformOrigin: Item.Center
                anchors.centerIn: parent
            }

            Image {
                id: iconImage
                width: parent.width
                height: width
                anchors.centerIn: parent
                anchors.verticalCenterOffset: textLabel.text === "" ? 0 : (textLabel.contentWidth/2 + 10)
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
