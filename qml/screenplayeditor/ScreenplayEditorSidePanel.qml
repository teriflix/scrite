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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor/scenelistpanel"

Item {
    id: root

    required property bool readOnly
    required property ScreenplayAdapter screenplayAdapter

    property alias expanded: _sidePanel.expanded
    property alias minPanelWidth: _sidePanel.minPanelWidth
    property alias maxPanelWidth: _sidePanel.maxPanelWidth

    signal positionScreenplayEditorAtTitlePage()

    width: _sidePanel.width

    SidePanel {
        id: _sidePanel

        height: parent.height

        z: expanded ? 1 : 0

        label: ""
        buttonY: 20
        maxPanelWidth: Runtime.screenplayEditorSettings.sidePanelWidth

        content: root.screenplayAdapter.elementCount === 0 ? _private.emptyScreenplayContent : _private.filledScreenplayContent
    }

    QtObject {
        id: _private

        readonly property string dragDropMimeType: "sceneListView/sceneID"
        readonly property real sceneIconSize: (Runtime.sceneEditorFontMetrics.xHeight + Runtime.sceneEditorFontMetrics.lineSpacing)/2
        readonly property real sceneIconPadding: 8

        readonly property Component emptyScreenplayContent: Item {
            VclLabel {
                anchors.top: parent.top
                anchors.topMargin: 50
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width * 0.9

                text: "Scene headings will be listed here as you add them into your screenplay."
                visible: root.screenplayAdapter.elementCount === 0
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        readonly property Component filledScreenplayContent: FocusScope {
            EventFilter.target: Scrite.app
            EventFilter.active: root.screenplayAdapter.isSourceScreenplay && root.screenplayAdapter.screenplay.hasSelectedElements
            EventFilter.events: [EventFilter.KeyPress]
            EventFilter.onFilter: (object,event,result) => {
                                      if(event.key === Qt.Key_Escape) {
                                          root.screenplayAdapter.screenplay.clearSelection()
                                          result.acceptEvent = true
                                          result.filter = true
                                      }
                                  }

            ListView {
                id: _sceneListView

                ScrollBar.vertical: VclScrollBar { flickable: _sceneListView }

                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                anchors.fill: parent

                clip: true
                model: root.screenplayAdapter
                focus: true
                currentIndex: root.screenplayAdapter.currentIndex

                boundsBehavior: ListView.StopAtBounds
                boundsMovement: ListView.StopAtBounds
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                highlightFollowsCurrentItem: true

                highlightRangeMode: ListView.ApplyRange
                keyNavigationWraps: false
                keyNavigationEnabled: true
                preferredHighlightEnd: height*0.8
                preferredHighlightBegin: height*0.2

                header: SceneListPanelHeader {
                    width: _sceneListView.width

                    color: root.screenplayAdapter.currentIndex < 0 ? Runtime.colors.primary.c300.background : Runtime.colors.primary.c10.background

                    leftPadding: _sceneListView.__leftPadding
                    rightPadding: _sceneListView.__rightPadding

                    onClicked: {
                        if(root.screenplayAdapter.isSourceScreenplay)
                            root.screenplayAdapter.screenplay.clearSelection()
                        root.screenplayAdapter.currentIndex = -1

                        root.positionScreenplayEditorAtTitlePage()
                    }
                }

                footer: SceneListPanelFooter {
                    width: _sceneListView.width

                    dragDropMimeType: _private.dragDropMimeType

                    onDropEntered: (drag) => {
                                       _sceneListView.forceActiveFocus()
                                   }

                    onDropExited: () => { } // Nothing to do here

                    onDropRequest: (drop) => {
                                       _moveElementTask.targetIndex = root.screenplayAdapter.elementCount
                                   }
                }

                delegate: SceneListPanelDelegate {
                    id: _delegate

                    width: _sceneListView.width

                    readOnly: root.readOnly
                    viewHasFocus: root.FocusTracker.hasFocus
                    leftPadding: _sceneListView.__leftPadding
                    rightPadding: _sceneListView.__rightPadding
                    sceneIconSize: _private.sceneIconSize
                    leftPaddingRatio: _sceneListView.__leftPaddingRatio
                    sceneIconPadding: _private.sceneIconPadding
                    dragDropMimeType: _private.dragDropMimeType
                    screenplayAdapter: root.screenplayAdapter

                    onDragStarted: () => {
                                       _moveElementTask.draggedElement = _delegate.screenplayElement
                                   }

                    onDragFinished: (dropAction) => {
                                        _sceneListView.forceLayout()
                                    }

                    onDropEntered: (drag) => {
                                        _sceneListView.forceActiveFocus()
                                   }

                    onDropExited: () => { } // Nothing to do here

                    onDropRequest: (drop) => {
                                       _moveElementTask.targetIndex = _delegate.index
                                   }

                    onContextMenuRequest: () => {
                                              if(screenplayElementType === ScreenplayElement.BreakElementType) {
                                                  _breakElementContextMenu.element = _delegate.screenplayElement
                                                  _breakElementContextMenu.popup(_delegate)
                                              } else {
                                                  _sceneElementsContextMenu.element = _delegate.screenplayElement
                                                  _sceneElementsContextMenu.popup(_delegate)
                                              }
                                          }

                    onCollapseSideListPanelRequest: () => {
                                                        _sidePanel.expanded = false
                                                    }
                }

                property real __leftPadding: Math.max(2*_private.sceneIconPadding, (_private.sceneIconSize + 2*_private.sceneIconPadding)*__leftPaddingRatio)
                property real __rightPadding: (_sceneListView.contentHeight > _sceneListView.height) ? 17 : 5
                property real __leftPaddingRatio: root.screenplayAdapter.hasNonStandardScenes ? 1 : 0

                Behavior on __leftPaddingRatio {
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: Runtime.stdAnimationDuration }
                }
            }

            ScreenplayBreakElementsContextMenu {
                id: _breakElementContextMenu

                enabled: !root.readOnly
            }

            ScreenplaySceneElementsContextMenu {
                id: _sceneElementsContextMenu

                enabled: !root.readOnly
            }

            SceneListPanelMoveElementsTask {
                id: _moveElementTask

                sceneListView: _sceneListView
            }

            Connections {
                target: root.screenplayAdapter.screenplay
                enabled: root.screenplayAdapter.isSourceScreenplay

                function onElementMoved(element, from, to) {
                    Qt.callLater(_sceneListView.forceLayout)
                }
            }
        }
    }
}
