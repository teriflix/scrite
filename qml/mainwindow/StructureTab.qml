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
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notifications"

SplitView {
    id: root

    Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

    orientation: Qt.Vertical

    Rectangle {
        id: _row1

        SplitView.fillHeight: true

        color: Runtime.colors.primary.c10.background

        SplitView {
            id: _split1

            Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

            anchors.fill: parent

            orientation: Qt.Horizontal

            Rectangle {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 80

                color: Runtime.colors.primary.c10.background
                border {
                    width: Runtime.showNotebookInStructure ? 0 : 1
                    color: Runtime.colors.primary.borderColor
                }

                Item {
                    id: structureEditorTabs

                    property int currentTabIndex: 0

                    anchors.fill: parent

                    Announcement.onIncoming: (type,data) => {
                        var sdata = "" + data
                        var stype = "" + type
                        if(Runtime.showNotebookInStructure) {
                            if(stype === Runtime.announcementIds.tabRequest) {
                                if(sdata === "Structure")
                                    structureEditorTabs.currentTabIndex = 0
                                else if(sdata.startsWith("Notebook")) {
                                    structureEditorTabs.currentTabIndex = 1
                                    if(sdata !== "Notebook")
                                        Runtime.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchTo(sdata)
                                        })
                                }
                            } else if(stype === Runtime.announcementIds.characterNotesRequest) {
                                structureEditorTabs.currentTabIndex = 1
                                Runtime.execLater(notebookViewLoader, 100, function() {
                                    notebookViewLoader.item.switchToCharacterTab(data)
                                })
                            }
                            else if(stype === Runtime.announcementIds.sceneNotesRequest) {
                                structureEditorTabs.currentTabIndex = 1
                                Runtime.execLater(notebookViewLoader, 100, function() {
                                    notebookViewLoader.item.switchToSceneTab(data)
                                })
                            }
                        }
                    }

                    Loader {
                        id: structureEditorTabBar

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom

                        // active: !Runtime.appFeatures.structure.enabled && Runtime.showNotebookInStructure
                        active: {
                            if(structureEditorTabs.currentTabIndex === 0)
                                return !Runtime.appFeatures.structure.enabled && Runtime.showNotebookInStructure
                            else if(structureEditorTabs.currentTabIndex === 1)
                                return !Runtime.appFeatures.notebook.enabled && Runtime.showNotebookInStructure
                            return false
                        }
                        visible: active

                        sourceComponent: Rectangle {
                            width: _mainToolbar.height+4

                            color: Runtime.colors.primary.c100.background

                            Column {
                                anchors.horizontalCenter: parent.horizontalCenter

                                FlatToolButton {
                                    ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"

                                    down: structureEditorTabs.currentTabIndex === 0
                                    visible: Runtime.showNotebookInStructure
                                    iconSource: "qrc:/icons/navigation/structure_tab.png"

                                    onClicked: Announcement.shout(Runtime.announcementIds.tabRequest, "Structure")
                                }

                                FlatToolButton {
                                    ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"

                                    down: structureEditorTabs.currentTabIndex === 1
                                    visible: Runtime.showNotebookInStructure
                                    iconSource: "qrc:/icons/navigation/notebook_tab.png"

                                    onClicked: Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                                }
                            }

                            Rectangle {
                                anchors.right: parent.right

                                width: 1
                                height: parent.height

                                color: Runtime.colors.primary.borderColor
                            }
                        }
                    }

                    Loader {
                        id: structureViewLoader

                        anchors.top: parent.top
                        anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        active: Runtime.appFeatures.structure.enabled
                        visible: !Runtime.showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
                        sourceComponent: StructureView {
                            HelpTipNotification {
                                tipName: "structure"
                                enabled: structureViewLoader.visible
                            }

                            onEditorRequest: { } // TODO
                            onReleaseEditorRequest: { } // TODO
                        }

                        DisabledFeatureNotice {
                            anchors.fill: parent
                            visible: !parent.active
                            featureName: "Structure"
                        }
                    }

                    Loader {
                        id: notebookViewLoader
                        anchors.top: parent.top
                        anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        active: visible && Runtime.appFeatures.notebook.enabled
                        visible: Runtime.showNotebookInStructure && structureEditorTabs.currentTabIndex === 1

                        sourceComponent: NotebookView {
                            toolbarSize: _mainToolbar.height+4
                            toolbarSpacing: _mainToolbar.spacing
                            toolbarLeftMargin: _mainToolbar.anchors.leftMargin
                        }

                        DisabledFeatureNotice {
                            anchors.fill: parent
                            visible: !parent.active
                            featureName: "Notebook"
                        }
                    }
                }

                /**
                  Some of our users find it difficult to know that they can pull the splitter handle
                  to reveal the timeline and/or screenplay editor. So we load an animation letting them
                  know about that and get rid of it once the animation is done.
                  */
                Loader {
                    id: splitViewAnimationLoader

                    property string sessionId

                    anchors.fill: parent

                    active: false

                    sourceComponent: Rectangle {
                        color: Scrite.app.translucent(Runtime.colors.primary.button, 0.5)

                        MouseArea {
                            anchors.fill: parent
                            onClicked: splitViewAnimationLoader.active = false
                        }

                        Timer {
                            interval: 5000
                            repeat: false
                            running: true
                            onTriggered: splitViewAnimationLoader.active = false
                        }

                        Item {
                            id: screenplayEditorHandle
                            width: 1
                            property real marginOnTheRight: 0
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.rightMargin: marginOnTheRight
                            visible: !screenplayEditor2.active

                            Rectangle {
                                height: parent.height * 0.5
                                width: 5
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                color: Runtime.colors.primary.windowColor
                                visible: screenplayEditorHandleAnimation.running
                            }

                            VclLabel {
                                color: Runtime.colors.primary.c50.background
                                text: "Pull this handle to view the screenplay editor."
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                                anchors.right: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.rightMargin: 20
                            }

                            SequentialAnimation {
                                id: screenplayEditorHandleAnimation
                                loops: 2
                                running: screenplayEditorHandle.visible

                                NumberAnimation {
                                    target: screenplayEditorHandle
                                    property: "marginOnTheRight"
                                    duration: 500
                                    from: 0; to: 50
                                }

                                NumberAnimation {
                                    target: screenplayEditorHandle
                                    property: "marginOnTheRight"
                                    duration: 500
                                    from: 50; to: 0
                                }
                            }
                        }

                        Item {
                            id: timelineViewHandle
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: marginOnTheBottom
                            height: 1
                            visible: !structureEditorRow2.active
                            property real marginOnTheBottom: 0

                            Rectangle {
                                width: parent.width * 0.5
                                height: 5
                                anchors.bottom: parent.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: Runtime.colors.primary.windowColor
                                visible: timelineViewHandleAnimation.running
                            }

                            VclLabel {
                                color: Runtime.colors.primary.c50.background
                                text: "Pull this handle to get the timeline view."
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.top
                                anchors.bottomMargin: 20
                            }

                            SequentialAnimation {
                                id: timelineViewHandleAnimation
                                loops: 2
                                running: timelineViewHandle.visible

                                NumberAnimation {
                                    target: timelineViewHandle
                                    property: "marginOnTheBottom"
                                    duration: 500
                                    from: 0; to: 50
                                }

                                NumberAnimation {
                                    target: timelineViewHandle
                                    property: "marginOnTheBottom"
                                    duration: 500
                                    from: 50; to: 0
                                }
                            }
                        }
                    }
                }
            }

            Loader {
                id: screenplayEditor2
                SplitView.preferredWidth: scriteMainWindow.width * 0.5
                SplitView.minimumWidth: 16
                onWidthChanged: Runtime.workspaceSettings.screenplayEditorWidth = width
                active: width >= 50
                sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null

                Rectangle {
                    visible: !parent.active
                    anchors.fill: parent
                    color: Runtime.colors.primary.c400.background
                }
            }
        }
    }

    Loader {
        id: structureEditorRow2
        SplitView.preferredHeight: 140 + Runtime.minimumFontMetrics.height*Runtime.screenplayTracks.trackCount
        SplitView.minimumHeight: 16
        SplitView.maximumHeight: SplitView.preferredHeight
        active: height >= 50
        sourceComponent: Rectangle {
            color: FocusTracker.hasFocus ? Runtime.colors.accent.c100.background : Runtime.colors.accent.c50.background
            FocusTracker.window: Scrite.window

            Behavior on color {
                enabled: Runtime.applicationSettings.enableAnimations
                ColorAnimation { duration: 250 }
            }

            TimelineView {
                anchors.fill: parent
                showNotesIcon: Runtime.showNotebookInStructure
            }

            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0,0,0,0)
                border { width: 1; color: Runtime.colors.accent.borderColor }
            }
        }

        Rectangle {
            visible: !parent.active
            anchors.fill: parent
            color: Runtime.colors.primary.c400.background
        }
    }

    Connections {
        target: Scrite.document
        function onAboutToSave() { root.saveLayoutDetails() }
        function onJustLoaded() { root.restoreLayoutDetails() }
    }

    Component.onCompleted: restoreLayoutDetails()
    Component.onDestruction: saveLayoutDetails()

    function saveLayoutDetails() {
        var userData = Scrite.document.userData
        userData["structureTab"] = {
            "version": 0,
            "screenplayEditorWidth": screenplayEditor2.width/_row1.width,
            "timelineViewHeight": structureEditorRow2.height
        }
        Scrite.document.userData = userData
    }

    function restoreLayoutDetails() {
        var userData = Scrite.document.userData
        if(userData.structureTab && userData.structureTab.version === 0) {
            structureEditorRow2.SplitView.preferredHeight = userData.structureTab.timelineViewHeight
            structureEditorRow2.height = structureEditorRow2.SplitView.preferredHeight
            screenplayEditor2.SplitView.preferredWidth = _row1.width*userData.structureTab.screenplayEditorWidth
            screenplayEditor2.width = screenplayEditor2.SplitView.preferredWidth
        }

        if(Runtime.structureCanvasSettings.showPullHandleAnimation && mainUiContentLoader.sessionId !== Scrite.document.sessionId) {
            Runtime.execLater(splitViewAnimationLoader, 250, function() {
                splitViewAnimationLoader.active = !screenplayEditor2.active || !structureEditorRow2.active
            })
            mainUiContentLoader.sessionId = Scrite.document.sessionId
        }
    }
}
