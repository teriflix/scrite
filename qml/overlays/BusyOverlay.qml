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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    function init(_parent) { parent = _parent }
    function ref(message, source) { _private.messageStack.ref(message, source) }
    function deref(source) { _private.messageStack.deref(source) }
    function discard() { _private.messageStack.discard() }

    anchors.fill: parent

    parent: Scrite.window.contentItem

    Popup {
        id: _popup

        Material.primary: Runtime.colors.primary.key
        Material.accent: Runtime.colors.accent.key
        Material.theme: Runtime.colors.theme

        anchors.centerIn: parent

        parent: Overlay.overlay

        closePolicy: Popup.NoAutoClose
        focus: false
        modal: true
        padding: 40
        visible: _private.messageStack.count > 0

        contentItem: ColumnLayout {
            id: _contentLayout

            spacing: 20

            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter

                running: true
            }

            VclLabel {
                Layout.preferredWidth: Math.min(640, Scrite.window.width * 0.8)

                text: _private.messageStack.top()
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    QtObject {
        id: _private

        property ListModel messageStack: ListModel {
            function top() {
                return count > 0 ? get(0).message : ""
            }

            function ref(message, source) {
                const idx = indexOfSource(source)
                if(idx < 0) {
                    insert(0, {"message": message, "source": source})
                } else {
                    setProperty(idx, "message", message)
                }
            }

            function deref(source) {
                const idx = indexOfSource(source)
                if(idx >= 0)
                    remove(idx, 1)
            }

            function discard() {
                clear()
            }

            function indexOfSource(source) {
                if(source === undefined || source === null || source === "") {
                    return -1
                }

                for(let i=0; i<count; i++) {
                    if(get(i).source === source) {
                        return i
                    }
                }

                return -1
            }
        }
    }
}
