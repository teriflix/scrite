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

Item {
    id: root

    SplitView {
        id: _mainSplit

        Material.background: _private.splitViewBackgroundColor

        anchors.fill: parent

        orientation: Qt.Vertical

        Item {
            id: _row1

            SplitView.fillHeight: true

            SplitView {
                id: _editorSplit

                Material.background: _private.splitViewBackgroundColor

                anchors.fill: parent

                orientation: Qt.Horizontal

                Item {
                    id: _col1

                    SplitView.fillWidth: true
                    SplitView.minimumWidth: 80

                    Rectangle {
                        anchors.fill: parent

                        color: Runtime.colors.primary.c50.background
                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 1

                        spacing: 0

                        VerticalToolBar {
                            id: _toolbar

                            Layout.fillHeight: true

                            actions: _contentLoader.item ? _contentLoader.item.toolbarActions : null
                        }

                        Loader {
                            id: _contentLoader

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            sourceComponent: _private.currentTab === Runtime.MainWindowTab.NotebookTab ? _private.notebook : _private.structureCanvas
                        }
                    }
                }

                Item {
                    id: _col2

                    SplitView.minimumWidth: 16
                    SplitView.preferredWidth: root.width * 0.5

                    ScreenplayTab {
                        id: _screenplayEditor

                        anchors.fill: parent
                    }
                }
            }
        }

        Item {
            id: _row2

            SplitView.minimumHeight: 16
            SplitView.maximumHeight: _private.preferredTimelineHeight * 2
            SplitView.preferredHeight: _private.preferredTimelineHeight

            TimelineView {
                id: _timeline

                anchors.fill: parent

                enabled: Runtime.appFeatures.structure.enabled && visible
                visible: height > _private.minimumTimelineHeight
                showCursor: Runtime.timelineViewSettings.showCursor && _screenplayEditor.hasFocus
                showNotesIcon: Runtime.showNotebookInStructure
            }

            VclLabel {
                anchors.centerIn: parent

                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                text: "Timeline hidden due to space constraint."
                visible: !_timeline.visible
                width: _row2.width * 0.5
            }

            DisabledFeatureNotice {
                anchors.fill: parent

                visible: !Runtime.appFeatures.structure.enabled
                featureName: "Structure"
            }
        }
    }

    QtObject {
        id: _private

        property int currentTab: Runtime.mainWindowTab === Runtime.MainWindowTab.NotebookTab ? Runtime.MainWindowTab.NotebookTab : Runtime.MainWindowTab.StructureTab

        property real minimumTimelineHeight: 70 + _timeline.tracksHeight
        property real preferredTimelineHeight: 140 + _timeline.tracksHeight
        property color splitViewBackgroundColor: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

        Component.onCompleted: restoreLayoutDetails()
        Component.onDestruction: saveLayoutDetails()

        property Connections documentConnections: Connections {
            target: Scrite.document

            function onJustReset() {

            }

            function onAboutToSave() {
                _private.saveLayoutDetails()
            }

            function onJustLoaded() {
                _private.restoreLayoutDetails()
            }
        }

        readonly property Component structureCanvas: Item {
            readonly property ActionManager toolbarActions: Runtime.appFeatures.structure.enabled ? ActionHub.structureCanvasOperations : null

            Loader {
                anchors.fill: parent

                active: Runtime.appFeatures.structure.enabled

                sourceComponent: StructureView {}
            }

            DisabledFeatureNotice {
                anchors.fill: parent

                visible: !Runtime.appFeatures.structure.enabled
                featureName: "Structure"
            }
        }

        readonly property Component notebook: Item {
            readonly property ActionManager toolbarActions: Runtime.appFeatures.notebook.enabled ? ActionHub.notebookOperations : null

            Loader {
                anchors.fill: parent

                active: Runtime.appFeatures.notebook.enabled

                sourceComponent: NotebookView { }
            }

            DisabledFeatureNotice {
                anchors.fill: parent

                visible: !Runtime.appFeatures.notebook.enabled
                featureName: "Notebook"
            }
        }

        readonly property BusyMessage busyMessage: BusyMessage {
            message: "Loading tab ..."

            function aboutToSwitchTab(from, to) {
                visible = true
            }

            function finishedTabSwitch(to) {
                visible = false
            }
        }

        readonly property Connections runtimeConnections: Connections {
            target: Runtime

            function onAboutToSwitchTab(from, to) {
                _private.busyMessage.aboutToSwitchTab(from, to)
            }

            function onFinishedTabSwitch(to) {
                _private.busyMessage.finishedTabSwitch(to)
            }
        }

        function saveLayoutDetails() {
            var userData = Scrite.document.userData
            userData["structureTab"] = {
                "version": 0,
                "screenplayEditorWidth": _col2.width/_editorSplit.width,
                "timelineViewHeight": _row2.height
            }
            Scrite.document.userData = userData
        }

        function restoreLayoutDetails() {
            const userData = Scrite.document.userData
            if(userData.structureTab && userData.structureTab.version === 0) {
                _row2.SplitView.preferredHeight = userData.structureTab.timelineViewHeight
                _col2.SplitView.preferredWidth = _editorSplit.width * userData.structureTab.screenplayEditorWidth
            }
        }
    }
}
