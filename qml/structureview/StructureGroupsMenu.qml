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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/notifications"

VclMenu {
    id: root

    property string innerTitle: ""
    property SceneGroup sceneGroup: null

    signal toggled(int row, string name)

    width: 450
    height: 500

    title: "Tag Groups"
    autoWidth: false
    enabled: !Scrite.document.readOnly
    closePolicy: Popup.CloseOnEscape|Popup.CloseOnPressOutside

    HelpTipNotification {
        id: htn

        tipName: "story_beat_tagging"
        enabled: false
    }

    VclMenuItem {
        width: root.width
        height: root.height

        background: Item { }

        contentItem: Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.bottomMargin: root.bottomPadding

                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    enabled: Runtime.appFeatures.structure.enabled && sceneGroup.sceneCount > 0
                    opacity: enabled ? 1 : 0.5

                    Rectangle {
                        anchors.fill: innerTitleText
                        color: Runtime.colors.primary.c700.background
                        visible: innerTitleText.visible
                    }

                    VclLabel {
                        id: innerTitleText

                        anchors.top: parent.top
                        anchors.margins: 3
                        anchors.horizontalCenter: parent.horizontalCenter

                        width: parent.width - 8
                        visible: text !== ""
                        wrapMode: Text.WordWrap

                        text: {
                            let ret = innerTitle
                            if(sceneGroup.hasSceneStackIds) {
                                if(ret !== "")
                                    ret += "<br/>"
                                ret += "<font size=\"-2\"><i>All scenes in the selected stack(s) are going to be tagged.</i></font>"
                            }
                            return ret;
                        }

                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize

                        color: Runtime.colors.primary.c700.text
                        padding: 5
                        horizontalAlignment: Text.AlignHCenter
                    }

                    ListView {
                        id: groupsView

                        property bool scrollBarVisible: groupsView.height < groupsView.contentHeight
                        property bool showingFilteredItems: sceneGroup.hasSceneActs && sceneGroup.hasGroupActs

                        function adjustScrolling() {
                            if(!showingFilteredItems) {
                                positionViewAtBeginning()
                                return
                            }

                            let prefCategory = Scrite.document.structure.preferredGroupCategory

                            let acts = sceneGroup.sceneActs
                            let index = -1
                            for(let i=0; i<sceneGroup.count; i++) {
                                let item = sceneGroup.at(i)

                                if(item.category.toUpperCase() !== prefCategory)
                                    continue

                                if( item.act === "" || acts.indexOf(item.act) >= 0) {
                                    positionViewAtIndex(i, ListView.Beginning)
                                    return
                                }
                            }
                        }

                        function adjustScrollingLater() {
                            Runtime.execLater(groupsView, 50, adjustScrolling)
                        }

                        ScrollBar.vertical: VclScrollBar { flickable: groupsView }
                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

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
                            required property string section

                            width: groupsView.width - (groupsView.scrollBarVisible ? 20 : 1)
                            height: 30
                            color: Runtime.colors.primary.windowColor

                            VclLabel {
                                id: categoryLabel
                                text: section
                                topPadding: 5
                                bottomPadding: 5
                                anchors.centerIn: parent
                                color: Runtime.colors.primary.button.text
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }
                        }

                        delegate: Rectangle {
                            required property int index
                            required property var arrayItem

                            property bool doesNotBelongToAnyAct: arrayItem.act === ""
                            property bool filtered: doesNotBelongToAnyAct || sceneGroup.sceneActs.indexOf(arrayItem.act) >= 0

                            width: groupsView.width - (groupsView.scrollBarVisible ? 20 : 1)
                            height: 30

                            color: groupItemMouseArea.containsMouse ? Runtime.colors.primary.button.background : Qt.rgba(0,0,0,0)
                            opacity: groupsView.showingFilteredItems ? (filtered ? 1 : 0.5) : 1

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8

                                spacing: 5

                                Image {
                                    anchors.verticalCenter: parent.verticalCenter

                                    width: 24; height: 24

                                    source: "qrc:/icons/navigation/check.png"
                                    opacity: {
                                        switch(arrayItem.checked) {
                                        case "no": return 0
                                        case "partial": return 0.25
                                        case "yes": return 1
                                        }
                                        return 0
                                    }
                                }

                                VclLabel {
                                    anchors.verticalCenter: parent.verticalCenter

                                    width: parent.width - parent.spacing - 24

                                    text: arrayItem.label
                                    elide: Text.ElideRight
                                    leftPadding: arrayItem.type > 0 ? 20 : 0

                                    font.bold: groupsView.showingFilteredItems ? filtered : doesNotBelongToAnyAct
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                }
                            }

                            MouseArea {
                                id: groupItemMouseArea

                                anchors.fill: parent

                                hoverEnabled: true

                                onClicked: {
                                    sceneGroup.toggle(index)
                                    root.toggled(index, arrayItem.name)
                                    Scrite.user.logActivity2("structure", "tag: " + arrayItem.name)
                                }
                            }
                        }

                        onShowingFilteredItemsChanged: adjustScrollingLater()
                    }
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Customise"
                    onClicked: StructureStoryBeatsDialog.launch()
                }
            }

            DisabledFeatureNotice {
                anchors.fill: parent

                color: Qt.rgba(1,1,1,0.8)
                visible: !Runtime.appFeatures.structure.enabled
                featureName: "Structure Tagging"

                onClicked: root.close()
            }
        }
    }

    onOpened: htn.enabled = true
}
