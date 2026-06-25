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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "LegacyDataMigrationDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        title: "Scrite Settings Migration"
        titleBarCloseButtonVisible: false

        width: Math.min(560, Scrite.window.width * 0.85)
        height: Math.min(400, Scrite.window.height * 0.85)

        content: Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                VclLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Your Scrite settings, recent files, and vault have been copied to a new location on this computer. You can look up the exact location by searching for \"Settings Folder\" in the Command Center."
                }

                VclLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Please note: if you downgrade to an older version of Scrite, they will continue to use settings, vault and recent files from the old location."
                    font.bold: true
                }
            }
        }

        bottomBar: Component {
            Item {
                height: _okButton.height + 20

                VclButton {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 16

                    text: "Show New Location"
                    onClicked: {
                        const fileInfo = File.info(Platform.settingsPath)
                        File.revealOnDesktop(fileInfo.absolutePath)
                    }
                }

                VclButton {
                    id: _okButton

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 16

                    text: "OK"

                    onClicked: {
                        Scrite.app.acknowledgeLegacyDataMigration()
                        _dialog.accept()
                    }
                }
            }
        }
    }
}
