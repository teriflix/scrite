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

import io.scrite.components 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

/**
  ToolButton from QtQuick.Controls is what we normally use in Scrite.
  ToolButton2 customises the built-in ToolButton to provide a standard behaviour
    across all tool-buttons in Scrite.

  This component (ToolButton3) is for use specifically in the new main-window toolbar.
  This new toolbar design is based on UI/UX suggestions from Surya Vasishta.
  */

Item {
    id: toolButton

    property string iconSource: ""
    property real suggestedWidth: 42
    property real suggestedHeight: suggestedWidth
    property alias shortcut: toolButtonShortcut.sequence
    property string shortcutText: toolButtonShortcut.portableText
    property bool down: toolButtonMouseArea.pressed
    property bool checkable: false
    property bool checked: false
    property alias hovered: toolButtonMouseArea.containsMouse
    property string text
    property bool autoRepeat: false
    property alias toolButtonImage: iconImage
    property bool hasMenu: false
    property int menuArrow: Qt.RightArrow

    signal toggled()
    signal clicked()

    width: suggestedWidth
    height: suggestedHeight

    Rectangle {
        id: downIndicator
        anchors.fill: parent
        color: Qt.rgba(0,0,0,0.15)
        visible: parent.checkable && parent.checked || parent.down
    }

    Image {
        id: iconImage
        z: 1
        anchors.fill: parent
        anchors.margins: anchorMargins
        source: parent.iconSource
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        opacity: enabled ? (toolButtonMouseArea.containsMouse ? 1 : 0.9) : 0.45
        property real anchorMargins: {
            var am = toolButtonMouseArea.containsMouse ? 8 : 10
            return parent.width-2*am < 16 ? (parent.width*0.15) : am
        }
        Behavior on anchorMargins {
            enabled: iconImage.anchorMargins > 0 && applicationSettings.enableAnimations
            NumberAnimation {
                duration: 250
            }
        }
    }

    Image {
        visible: hasMenu
        width: parent.width/2.5
        height: parent.height/2.5

        anchors.verticalCenter: menuArrow === Qt.RightArrow ? parent.verticalCenter : undefined
        anchors.right: menuArrow === Qt.RightArrow ? parent.right : undefined
        anchors.rightMargin: menuArrow === Qt.RightArrow ? -parent.width/10 : 0

        anchors.horizontalCenter: menuArrow === Qt.DownArrow ? parent.horizontalCenter : undefined
        anchors.bottom: menuArrow === Qt.DownArrow ? parent.bottom : undefined
        anchors.bottomMargin:  menuArrow === Qt.DownArrow ? -parent.height/10 : 0

        fillMode: Image.PreserveAspectFit
        opacity: iconImage.opacity
        source: menuArrow === Qt.RightArrow ? "../icons/navigation/arrow_right.png" : "../icons/navigation/arrow_down.png"
    }

    Shortcut {
        id: toolButtonShortcut
        context: Qt.ApplicationShortcut
        enabled: !modalDialog.active
        onActivated: toolButton.click()
    }

    ToolTip.text: shortcutText === "" ? text : (text + "\t(" + Scrite.app.polishShortcutTextForDisplay(shortcutText) + ")")
    ToolTip.visible: ToolTip.text === "" ? false : (toolButtonMouseArea.containsMouse && !down)
    ToolTip.delay: 500

    property bool containsMouse: toolButtonMouseArea.containsMouse

    MouseArea {
        id: toolButtonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: toolButton.click()
    }

    function click() {
        if(!enabled)
            return
        if(checkable) {
            checked = !checked
            toggled()
        } else {
            clicked()
        }
    }
}
