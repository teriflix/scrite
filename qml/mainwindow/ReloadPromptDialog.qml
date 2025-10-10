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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch() { return doLaunch() }

    function launchLater() { _timer.start() }
    function abortLaunchLater() { _timer.stop() }

    name: "ReloadPromptDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        width: Math.min(500, Scrite.window.width * 0.5)
        height: 275
        title: "Reload Required"

        titleBarButtons: null

        content: Item {
            property real preferredHeight: _layout.height + 40

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
                            _dialog.close()
                        }
                    }

                    VclButton {
                        text: "No"
                        onClicked: _dialog.close()
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

            property bool autoSaveFlag: false
            Component.onCompleted: {
                autoSaveFlag = Scrite.document.autoSave
                Scrite.document.autoSave = false
            }
            Component.onDestruction: Scrite.document.autoSave = autoSaveFlag
        }
    }

    Timer {
        id: _timer

        repeat: false
        interval: 500

        onTriggered: {
            if(Runtime.applicationSettings.reloadPrompt)
                root.launch()
        }
    }
}
