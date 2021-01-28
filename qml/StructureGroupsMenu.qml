/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

Menu2 {
    id: structureGroupsMenu

    property SceneGroup sceneGroup: null

    title: "Tag Groups"

    width: 450
    height: 500

    MenuItem2 {
        width: structureGroupsMenu.width
        height: structureGroupsMenu.height

        background: Item { }
        contentItem: Item {
            Rectangle {
                anchors.fill: parent
                anchors.bottomMargin: 10
                border.width: 1
                border.color: primaryColors.borderColor
                enabled: sceneGroup.sceneCount > 0
                opacity: enabled ? 1 : 0.5

                ListView {
                    id: groupsView
                    anchors.fill: parent
                    anchors.margins: 5
                    clip: true
                    model: sceneGroup
                    section.property: "category"
                    section.criteria: ViewSection.FullString
                    section.delegate: Rectangle {
                        width: groupsView.width - (groupsView.scrollBarVisible ? 20 : 1)
                        height: categoryLabel.height
                        color: primaryColors.windowColor
                        Text {
                            id: categoryLabel
                            text: section
                            topPadding: 5
                            bottomPadding: 5
                            anchors.centerIn: parent
                            color: primaryColors.button.text
                            font.pointSize: app.idealFontPointSize
                        }
                    }
                    property bool scrollBarVisible: groupsView.height < groupsView.contentHeight
                    ScrollBar.vertical: ScrollBar {
                        policy: groupsView.scrollBarVisible ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                    }
                    delegate: Rectangle {
                        width: groupsView.width - (groupsView.scrollBarVisible ? 20 : 1)
                        height: 30
                        color: groupItemMouseArea.containsMouse ? primaryColors.button.background : Qt.rgba(0,0,0,0)

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 5

                            Image {
                                opacity: {
                                    switch(arrayItem.checked) {
                                    case "no": return 0
                                    case "partial": return 0.5
                                    case "yes": return 1
                                    }
                                    return 0
                                }
                                source: "../icons/navigation/check.png"
                                anchors.verticalCenter: parent.verticalCenter
                                width: 24; height: 24
                            }

                            Text {
                                text: arrayItem.label
                                width: parent.width - parent.spacing - 24
                                anchors.verticalCenter: parent.verticalCenter
                                font.pointSize: app.idealFontPointSize
                                leftPadding: arrayItem.type > 0 ? 20 : 0
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            id: groupItemMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: sceneGroup.toggle(index)
                        }
                    }
                }
            }
        }
    }
}
