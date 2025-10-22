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

                        ToolBar {
                            id: _toolbar

                            Layout.fillHeight: true

                            Material.accent: Runtime.colors.accent.key
                            Material.background: Runtime.colors.primary.c10.background
                            Material.elevation: 0
                            Material.primary: Runtime.colors.primary.key
                            Material.theme: Runtime.colors.theme

                            GridLayout {
                                id: _toolbarLayout

                                readonly property size buttonSize: Runtime.estimateTypeSize("ToolButton { icon.source: \"qrc:/icons/content/blank.png\"; display: ToolButton.IconOnly; }")
                                property int buttonCount: (toolbarActions ? toolbarActions.count : 0) + (Runtime.showNotebookInStructure ? 2 : 0)
                                property ActionManager toolbarActions: _contentLoader.item ? _contentLoader.item.toolbarActions : null

                                anchors.fill: parent

                                flow: Flow.TopToBottom
                                rows: Math.floor(_toolbar.height/buttonSize.height)
                                columns: Math.ceil( (buttonCount * buttonSize.height)/_toolbar.height )

                                ActionToolButton {
                                    action: ActionHub.mainWindowTabs.find("structureTab")
                                    visible: Runtime.showNotebookInStructure
                                }

                                ActionToolButton {
                                    action: ActionHub.mainWindowTabs.find("notebookTab")
                                    visible: Runtime.showNotebookInStructure
                                }

                                Repeater {
                                    model: _toolbarLayout.toolbarActions

                                    ActionToolButton {
                                        required property var qmlAction

                                        action: qmlAction
                                    }
                                }

                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                }
                            }
                        }

                        Loader {
                            id: _contentLoader

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            sourceComponent: _private.currentTabContent === Runtime.MainWindowTab.NotebookTab ? _private.notebook : _private.structureCanvas
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
            SplitView.maximumHeight: _private.preferredTimelineHeight
            SplitView.preferredHeight: _private.preferredTimelineHeight

            TimelineView {
                id: _timeline

                anchors.fill: parent

                enabled: Runtime.appFeatures.structure.enabled
                showNotesIcon: Runtime.showNotebookInStructure
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

        property int currentTabContent: Runtime.MainWindowTab.StructureTab // can be this or Runtime.MainWindowTab.NotebookTab

        property real preferredTimelineHeight: 140 + Runtime.minimumFontMetrics.height*Runtime.screenplayTracks.trackCount
        property color splitViewBackgroundColor: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

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

        Announcement.onIncoming: (type, data) => {
                                     if(type === announcementIds.embeddedTabRequest) {
                                         if(data === "Notebook")
                                            currentTabContent = Runtime.MainWindowTab.NotebookTab
                                         else
                                            currentTabContent = Runtime.MainWindowTab.StructureTab
                                     }
                                 }
    }
}
