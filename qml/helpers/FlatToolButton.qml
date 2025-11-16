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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

/**
ToolButton from QtQuick.Controls is what we normally use in Scrite.
ToolButton2 customises the built-in ToolButton to provide a standard behaviour
across all tool-buttons in Scrite.

This component (ToolButton3) is for use specifically in the new main-window toolbar.
This new toolbar design is based on UI/UX suggestions from Surya Vasishta.
*/

Item {
    id: root

    property int menuArrow: Qt.RightArrow

    property real suggestedWidth: 42
    property real suggestedHeight: suggestedWidth

    property alias hovered: _mouseArea.containsMouse
    property alias margins: _icon.anchorMargins
    property alias shortcut: _shortcut.sequence
    property alias toolButtonImage: _icon
    property alias downIndicatorColor: _downIndicator.color

    property string text
    property string iconSource: ""
    property string shortcutText: _shortcut.portableText
    property string toolTipText: shortcutText === "" ? text : (text + "\t(" + Gui.nativeShortcut(shortcutText) + ")")

    property bool down: _mouseArea.pressed
    property bool hasMenu: false
    property bool checked: false
    property bool checkable: false
    property bool autoRepeat: false
    property bool toolTipVisible: toolTipText !== "" && (_mouseArea.containsMouse && !down)

    readonly property alias containsMouse: _mouseArea.containsMouse

    signal toggled()
    signal clicked()

    width: suggestedWidth
    height: suggestedHeight

    implicitWidth: suggestedWidth
    implicitHeight: suggestedHeight

    opacity: enabled ? 1 : 0.5

    Rectangle {
        id: _downIndicator

        anchors.fill: parent

        color: Qt.rgba(0,0,0,0.15)
        visible: parent.checkable && parent.checked || parent.down
    }

    Image {
        id: _icon

        property real anchorMargins: {
            const am = _mouseArea.containsMouse ? 8 : 10
            return parent.width-2*am < 16 ? (parent.width*0.15) : am
        }

        anchors.fill: parent
        anchors.margins: anchorMargins

        z: 1
        source: parent.iconSource
        smooth: true
        mipmap: true
        opacity: enabled ? (_mouseArea.containsMouse ? 1 : 0.9) : 0.45
        fillMode: Image.PreserveAspectFit

        Behavior on anchorMargins {
            enabled: _icon.anchorMargins > 0 && Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: Runtime.stdAnimationDuration }
        }
    }

    Image {
        anchors.right: menuArrow === Qt.RightArrow ? parent.right : undefined
        anchors.rightMargin: menuArrow === Qt.RightArrow ? -parent.width/10 : 0
        anchors.verticalCenter: menuArrow === Qt.RightArrow ? parent.verticalCenter : undefined

        anchors.bottom: menuArrow === Qt.DownArrow ? parent.bottom : undefined
        anchors.bottomMargin:  menuArrow === Qt.DownArrow ? -parent.height/10 : 0
        anchors.horizontalCenter: menuArrow === Qt.DownArrow ? parent.horizontalCenter : undefined

        width: parent.width/2.5
        height: parent.height/2.5

        visible: hasMenu

        source: menuArrow === Qt.RightArrow ? "qrc:/icons/navigation/arrow_right.png" : "qrc:/icons/navigation/arrow_down.png"
        opacity: _icon.opacity
        fillMode: Image.PreserveAspectFit
    }

    Shortcut {
        id: _shortcut

        context: Qt.ApplicationShortcut
        enabled: Runtime.allowAppUsage

        onActivated: root.click()
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: root.click()
    }

    ToolTipPopup {
        container: root
        text: root.toolTipText
        visible: text !== "" && root.toolTipVisible
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
