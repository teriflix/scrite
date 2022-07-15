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

import io.scrite.components 1.0

Menu2 {
    id: structureGroupsMenu

    property SceneGroup sceneGroup: null
    signal toggled(int row, string name)
    closePolicy: htn.Notification.active ? Popup.NoAutoClose : Popup.CloseOnEscape|Popup.CloseOnPressOutside
    enabled: !Scrite.document.readOnly

    title: "Tag Groups"
    property string innerTitle: ""

    width: 450
    height: 500

    HelpTipNotification {
        id: htn
        tipName: "story_beat_tagging"
        enabled: structureGroupsMenu.opened
    }

    MenuItem2 {
        width: structureGroupsMenu.width
        height: structureGroupsMenu.height

        background: Item { }
        contentItem: Item {
            Rectangle {
                anchors.fill: parent
                anchors.bottomMargin: structureGroupsMenu.bottomPadding
                border.width: 1
                border.color: primaryColors.borderColor
                enabled: structureAppFeature.enabled && sceneGroup.sceneCount > 0
                opacity: enabled ? 1 : 0.5

                Rectangle {
                    anchors.fill: innerTitleText
                    color: primaryColors.c700.background
                    visible: innerTitleText.visible
                }

                Text {
                    id: innerTitleText
                    width: parent.width - 8
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.margins: 3
                    wrapMode: Text.WordWrap
                    text: {
                        var ret = innerTitle
                        if(sceneGroup.hasSceneStackIds) {
                            if(ret !== "")
                                ret += "<br/>"
                            ret += "<font size=\"-2\"><i>All scenes in the selected stack(s) are going to be tagged.</i></font>"
                        }
                        return ret;
                    }
                    font.pointSize: Scrite.app.idealFontPointSize
                    visible: text !== ""
                    horizontalAlignment: Text.AlignHCenter
                    padding: 5
                    color: primaryColors.c700.text
                    font.bold: true
                }

                ListView {
                    id: groupsView
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    anchors.left: parent.left
                    anchors.top: innerTitleText.visible ? innerTitleText.bottom : parent.top
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 5
                    clip: true
                    model: sceneGroup
                    keyNavigationEnabled: false
                    section.property: "category"
                    section.criteria: ViewSection.FullString
                    section.delegate: Rectangle {
                        width: groupsView.width - (groupsView.scrollBarVisible ? 20 : 1)
                        height: 30
                        color: primaryColors.windowColor
                        Text {
                            id: categoryLabel
                            text: section
                            topPadding: 5
                            bottomPadding: 5
                            anchors.centerIn: parent
                            color: primaryColors.button.text
                            font.pointSize: Scrite.app.idealFontPointSize
                        }
                    }
                    property bool scrollBarVisible: groupsView.height < groupsView.contentHeight
                    ScrollBar.vertical: ScrollBar2 { flickable: groupsView }
                    property bool showingFilteredItems: sceneGroup.hasSceneActs && sceneGroup.hasGroupActs
                    onShowingFilteredItemsChanged: adjustScrollingLater()

                    function adjustScrolling() {
                        if(!showingFilteredItems) {
                            positionViewAtBeginning()
                            return
                        }

                        var prefCategory = Scrite.document.structure.preferredGroupCategory

                        var acts = sceneGroup.sceneActs
                        var index = -1
                        for(var i=0; i<sceneGroup.count; i++) {
                            var item = sceneGroup.at(i)

                            if(item.category.toUpperCase() !== prefCategory)
                                continue

                            if( item.act === "" || acts.indexOf(item.act) >= 0) {
                                positionViewAtIndex(i, ListView.Beginning)
                                return
                            }
                        }
                    }

                    function adjustScrollingLater() {
                        Scrite.app.execLater(groupsView, 50, adjustScrolling)
                    }

                    delegate: Rectangle {
                        width: groupsView.width - (groupsView.scrollBarVisible ? 20 : 1)
                        height: 30
                        color: groupItemMouseArea.containsMouse ? primaryColors.button.background : Qt.rgba(0,0,0,0)
                        opacity: groupsView.showingFilteredItems ? (filtered ? 1 : 0.5) : 1
                        property bool doesNotBelongToAnyAct: arrayItem.act === ""
                        property bool filtered: doesNotBelongToAnyAct || sceneGroup.sceneActs.indexOf(arrayItem.act) >= 0

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 5

                            Image {
                                opacity: {
                                    switch(arrayItem.checked) {
                                    case "no": return 0
                                    case "partial": return 0.25
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
                                font.bold: groupsView.showingFilteredItems ? filtered : doesNotBelongToAnyAct
                                font.pointSize: Scrite.app.idealFontPointSize
                                leftPadding: arrayItem.type > 0 ? 20 : 0
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            id: groupItemMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                sceneGroup.toggle(index)
                                structureGroupsMenu.toggled(index, arrayItem.name)
                                Scrite.user.logActivity2("structure", "tag: " + arrayItem.name)
                            }
                        }
                    }
                }
            }

            DisabledFeatureNotice {
                anchors.fill: parent
                color: Qt.rgba(1,1,1,0.8)
                visible: !structureAppFeature.enabled
                featureName: "Structure Tagging"
                onClicked: structureGroupsMenu.close()
            }
        }
    }
}
