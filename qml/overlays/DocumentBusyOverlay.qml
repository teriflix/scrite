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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

QtObject {
    function init() {
        if(!OverlaysLayer.valid) {
            console.log("Overlays layer not initialized. Cannot initialize DocumentBusyOverlay.")
            return null
        }

        Scrite.document.busyChanged.connect( () => {
                                                if(!OverlaysLayer.valid) {
                                                    console.log("Overlays layer not initialized. Cannot create DocumentBusyOverlay.")
                                                    return null
                                                }

                                                if(Scrite.document.busy) {
                                                    var olay = overlayComponent.createObject(OverlaysLayer.item)
                                                    if(olay) {
                                                        olay.done.connect(olay.destroy)
                                                        olay.visible = true
                                                    }
                                                }
                                            } )
    }

    readonly property Component overlayComponent: Item {
        signal done()

        anchors.fill: parent
        z: 1

        Rectangle {
            anchors.fill: indication
            anchors.margins: -30
            radius: 4
            color: Runtime.colors.primary.c600.background
        }

        Row {
            id: indication
            anchors.centerIn: parent
            spacing: 20
            width: Math.min(parent.width * 0.4, implicitWidth)

            property real maxWidth: parent.width*0.4

            BusyIcon {
                id: busyIndicator
                anchors.verticalCenter: parent.verticalCenter
                running: true
                width: 50; height: 50
                forDarkBackground: true
            }

            VclLabel {
                width: Math.min(parent.maxWidth - busyIndicator.width - parent.spacing, contentWidth)
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: Scrite.document.busyMessage
                color: Runtime.colors.primary.c600.text
            }
        }

        // Swallow all mouse & keyboard events as long as the document is busy
        EventFilter.target: Scrite.app
        EventFilter.events: [
            EventFilter.MouseButtonRelease,
            EventFilter.MouseButtonDblClick,
            EventFilter.MouseMove,
            EventFilter.KeyPress,
            EventFilter.KeyRelease,
            EventFilter.Shortcut,
            EventFilter.ShortcutOverride,
            EventFilter.HoverEnter,
            EventFilter.HoverLeave,
            EventFilter.HoverMove,
            EventFilter.DragEnter,
            EventFilter.DragMove,
            EventFilter.DragLeave,
            EventFilter.Drop
        ]
        EventFilter.onFilter: {
            result.filter = true
        }

        // Private section
        property bool documentIsBusy: Scrite.document.busy
        onDocumentIsBusyChanged: {
            if(!documentIsBusy)
                done()
        }
    }
}
