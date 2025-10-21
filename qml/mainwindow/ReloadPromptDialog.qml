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
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/"
import "qrc:/qml/tasks"
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notifications"

VclDialog {
    id: root

    width: Math.min(500, Scrite.window.width * 0.5)
    height: 275

    title: "Reload Required"
    titleBarButtons: null

    content: Item {
        property bool autoSaveFlag: false

        property real preferredHeight: _layout.height + 40

        Component.onCompleted: {
            autoSaveFlag = Scrite.document.autoSave
            Scrite.document.autoSave = false
        }

        Component.onDestruction: {
            Scrite.document.autoSave = autoSaveFlag
        }

        ColumnLayout {
            id: _layout

            anchors.fill: parent
            anchors.margins: 20

            spacing: 10

            VclLabel {
                Layout.fillWidth: true

                text: "Current file was modified by another process in the background. Do you want to reload?"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter

                spacing: 20

                VclButton {
                    text: "Yes"
                    onClicked: {
                        Scrite.document.reload()
                        root.close()
                    }
                }

                VclButton {
                    text: "No"
                    onClicked: root.close()
                }
            }

            ColumnLayout {
                spacing: 2

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Runtime.colors.primary.borderColor
                }

                RowLayout {
                    VclCheckBox {
                        Layout.fillWidth: true

                        text: "Don't show this again."
                        checked: false
                        onToggled: Runtime.applicationSettings.reloadPrompt = !checked
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }

                    Link {
                        text: "More Info"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/version-1-2-released/#chapter6_reload_prompt")
                    }
                }
            }
        }
    }

    Connections {
        target: Scrite.document

        function onJustReset() {
            Runtime.firstSwitchToStructureTab = true
            if(_private.reloadTimer)
                _private.reloadTimer.stop()
            Runtime.execLater(Runtime.screenplayAdapter, 250, () => {
                                  Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
                              })
        }

        function onJustLoaded() {
            Runtime.firstSwitchToStructureTab = true
        }

        function onOpenedAnonymously(filePath) {
            MessageBox.question("Anonymous Open",
                                "The file you just opened is a backup of another file, and is being opened anonymously in <b>read-only</b> mode.<br/><br/>" +
                                "<b>NOTE:</b> In order to edit the file, you will need to first Save-As.",
                                ["Save As", "View Read Only"],
                                (answer) => {
                                    if(answer === "Save As")
                                    SaveFileTask.saveAs()
                                })
        }

        function onRequiresReload() {
            if(Runtime.applicationSettings.reloadPrompt)
                _private.reloadTimer = Runtime.execLater(root, Runtime.stdAnimationDuration, root.open)
        }
    }

    QtObject {
        id: _private

        property Timer reloadTimer
    }
}

