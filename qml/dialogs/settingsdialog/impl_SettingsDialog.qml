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

    property string activeTab

    title: "Settings"
    width: Math.min(Scrite.window.width-80, 1049)
    height: Math.min(Scrite.window.height-80, 750)

    content: ColumnLayout {
        spacing: 0

        Component.onCompleted: {
            if(root.activeTab !== "") {
                for(let i=0; i<_tabBar.count; i++) {
                    const tab = _tabBar.itemAt(i)
                    if(tab.text === root.activeTab) {
                        _tabBar.currentIndex = i
                        break
                    }
                }
            }
        }

        TabBar {
            id: _tabBar

            Material.primary: Runtime.colors.primary.key
            Material.accent: Runtime.colors.accent.key
            Material.theme: Runtime.colors.theme

            Layout.fillWidth: true

            TabButton {
                text: "Application"
                font: Runtime.idealFontMetrics.font
            }

            TabButton {
                text: "Structure"
                font: Runtime.idealFontMetrics.font
            }

            TabButton {
                text: "Screenplay"
                font: Runtime.idealFontMetrics.font
            }

            TabButton {
                text: "Notebook"
                font: Runtime.idealFontMetrics.font
            }

            TabButton {
                text: "Language"
                font: Runtime.idealFontMetrics.font
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Runtime.colors.primary.c50.background

            Loader {
                anchors.fill: parent

                property real pageListWidth: (width/_tabBar.count)

                source: "./" + _tabBar.currentItem.text + "SettingsTab.qml"
                onItemChanged: {
                    if(item) {
                        item.pageListWidth = Qt.binding( () => { return pageListWidth } )

                        Runtime.showHelpTip("settings" + _tabBar.currentItem.text)
                    }
                }
            }
        }
    }
}
