/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

QtObject {
    id: root

    function show(text) {
        if(!OverlaysLayer.valid) {
            console.log("Overlays layer not initialized. Cannot create AnimatedTextOverlay for '" + text + "'")
            return null
        }

        var olay = overlayComponent.createObject(OverlaysLayer.item, {"text": text})
        if(olay) {
            olay.done.connect(olay.destroy)
            olay.visible = true
            return olay
        }

        console.log("Could not create AnimatedTextOverlay for '" + text + "'")
        return null
    }

    readonly property Component overlayComponent: Item {
        id: overlay

        required property string text

        signal done()

        anchors.fill: parent

        VclText {
            id: textItem
            anchors.centerIn: parent

            font.pixelSize: parent.height * 0.075

            text: parent.text

            property real t: Runtime.applicationSettings.enableAnimations ? 0 : 1
            scale: 0.5 + t/1.0
            opacity: Runtime.applicationSettings.enableAnimations ? (1.0 - t*0.75) : 0.8
        }

        SequentialAnimation {
            running: true

            NumberAnimation {
                target: textItem
                properties: "t"
                from: 0
                to: 1
                duration: Runtime.applicationSettings.enableAnimations ? 250 : 0
                easing.type: Easing.OutQuint
            }

            PauseAnimation {
                duration: Runtime.applicationSettings.enableAnimations ? 0 : 250
            }

            ScriptAction {
                script: overlay.done()
            }
        }
    }
}
