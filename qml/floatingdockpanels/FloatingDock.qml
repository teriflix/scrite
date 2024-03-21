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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    // Public properties
    property string title: "Floating Dock"
    property real titleBarHeight: titleBar.height
    property Component content: Item { }

    // Public signals
    signal closeRequest()

    // Public methods
    function open() { visible = true }
    function close() { visible = false }
    function toggle() { visible = !visible }
    function adjustedX(xVal) {
        const availableSpace = parent ? parent.width : Scrite.window.width
        return Utils.bounded(_private.margin, xVal, availableSpace-width-_private.margin)
    }
    function adjustedY(yVal) {
        const availableSpace = parent ? parent.height : Scrite.window.height
        return Utils.bounded(_private.margin, yVal, availableSpace-height-_private.margin)
    }

    // Implementation
    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    parent: FloatingDockLayer.item
    x: 20
    y: 20
    width: 200
    height: 100

    // Shadow Effect
    BoxShadow {
        anchors.fill: parent
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: Runtime.colors.primary.c100.background
    }

    // Titlebar
    Rectangle {
        id: titleBar
        width: root.width
        height: dialogHeaderLayout.height

        color: Runtime.colors.accent.c600.background

        MouseArea {
            id: dragArea
            anchors.fill: parent
            drag.target: root
            drag.axis: Drag.XAndYAxis
            onPressed: _private.raise()
        }

        RowLayout {
            id: dialogHeaderLayout

            width: parent.width

            spacing: 2

            VclText {
                id: titleText

                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                color: Runtime.colors.accent.c600.text
                padding: 4
                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize

                text: root.title
            }

            Image {
                Layout.alignment: Qt.AlignVCenter

                Layout.preferredWidth: Runtime.idealFontMetrics.height + titleText.padding*2
                Layout.preferredHeight: Runtime.idealFontMetrics.height + titleText.padding*2

                source: "qrc:/icons/action/dialog_close_button.png"
                smooth: true; mipmap: true

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.scale = 1.2
                    onExited: parent.scale = 1
                    onClicked: {
                        root.closeRequest()
                        Qt.callLater(root.close)
                    }
                }
            }
        }
    }

    // Content
    Loader {
        width: root.width
        height: root.height - titleBar.height
        y: titleBar.height
        sourceComponent: content
        active: root.visible
    }

    // Private section
    Component.onCompleted: {
        if(!FloatingDockLayer.valid)
            Scrite.app.log("FloatingDockLayer is not initialized!")
    }

    QtObject {
        id: _private

        readonly property real margin: 20

        function raise() {
            var docks = root.parent.children
            for(var i=0; i<docks.length; i++)
                docks[i].z = 0
            root.z = 1
        }
    }
}
