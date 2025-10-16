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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

VclDialog {
    id: root

    title: "Settings"
    width: Math.min(Scrite.window.width-80, 1049)
    height: Math.min(Scrite.window.height-80, 750)

    content: ColumnLayout {
        spacing: 0

        TabBar {
            id: settingsDialogTabBar

            Layout.fillWidth: true

            TabButton {
                text: "Application"
            }

            TabButton {
                text: "Structure"
            }

            TabButton {
                text: "Screenplay"
            }

            TabButton {
                text: "Notebook"
            }

            TabButton {
                text: "Language"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Runtime.colors.primary.c50.background

            Loader {
                id: settingsDialogContent
                anchors.fill: parent

                property real pageListWidth: (width/settingsDialogTabBar.count)

                source: "./" + settingsDialogTabBar.currentItem.text + "SettingsTab.qml"
                onItemChanged: {
                    if(item) {
                        item.pageListWidth = Qt.binding( () => { return pageListWidth } )

                        Runtime.showHelpTip("settings" + settingsDialogTabBar.currentItem.text)
                    }
                }
            }
        }
    }
}
