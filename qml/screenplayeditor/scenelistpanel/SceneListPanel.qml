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
import "qrc:/qml/screenplayeditor"

ListView {
    id: root

    required property bool readOnly
    required property bool tracksVisible
    required property ScreenplayAdapter screenplayAdapter

    readonly property alias sceneGroup: _private.sceneGroup
    readonly property alias delegateCount: _private.delegateCount

    function updateCacheBuffer() { _private.updateCacheBuffer() }
    function extents(startIndex, endIndex) { return _private.extents(startIndex, endIndex) }

    ScrollBar.vertical: VclScrollBar { flickable: root }

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

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

    footer: SceneListPanelFooter {
        width: root.width

        dragDropMimeType: _private.dragDropMimeType

        onDropEntered: (drag) => {
                           root.forceActiveFocus()
                       }

        onDropExited: () => { } // Nothing to do here

        onDropRequest: (drop) => {
                           _moveElementTask.targetIndex = root.screenplayAdapter.elementCount
                       }
    }

    delegate: SceneListPanelDelegate {
        id: _delegate

        Component.onCompleted: { _private.delegateCount = _private.delegateCount+1 }
        Component.onDestruction: { _private.delegateCount = _private.delegateCount-1 }

        width: root.width

        readOnly: root.readOnly
        viewHasFocus: root.FocusTracker.hasFocus
        leftPadding: root.__leftPadding
        rightPadding: root.__rightPadding
        sceneIconSize: _private.sceneIconSize
        leftPaddingRatio: root.__leftPaddingRatio
        sceneIconPadding: _private.sceneIconPadding
        dragDropMimeType: _private.dragDropMimeType
        screenplayAdapter: root.screenplayAdapter

        onDragStarted: () => {
                           _moveElementTask.draggedElement = _delegate.screenplayElement
                       }

        onDragFinished: (dropAction) => {
                            root.forceLayout()
                        }

        onDropEntered: (drag) => {
                            root.forceActiveFocus()
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
    property real __rightPadding: (root.contentHeight > root.height) ? 17 : 5
    property real __leftPaddingRatio: root.screenplayAdapter.hasNonStandardScenes ? 1 : 0

    Behavior on __leftPaddingRatio {
        enabled: Runtime.applicationSettings.enableAnimations
        NumberAnimation { duration: Runtime.stdAnimationDuration }
    }


    ScreenplayBreakElementsContextMenu {
        id: _breakElementContextMenu

        enabled: !root.readOnly
    }

    ScreenplaySceneElementsContextMenu {
        id: _sceneElementsContextMenu

        enabled: !root.readOnly
        sceneGroup: _private.sceneGroup
    }

    SceneListPanelMoveElementsTask {
        id: _moveElementTask

        sceneListView: root
    }

    Connections {
        target: root.screenplayAdapter.screenplay
        enabled: root.screenplayAdapter.isSourceScreenplay

        function onElementMoved(element, from, to) {
            Qt.callLater(root.forceLayout)
            Qt.callLater(_private.updateCacheBuffer)
        }

        function onRowsInserted() { Qt.callLater(_private.updateCacheBuffer) }
        function onRowsRemoved() { Qt.callLater(_private.updateCacheBuffer) }
        function onModelReset() { Qt.callLater(_private.updateCacheBuffer) }
    }

    onCountChanged: Qt.callLater(_private.updateCacheBuffer)
    onDelegateChanged: Qt.callLater(_private.updateCacheBuffer)

    QtObject {
        id: _private

        readonly property Connections screenplayConnections: Connections {
            target: root.screenplayAdapter.screenplay

            function onSelectionChanged() {
                Qt.callLater(_private.sceneGroup.refresh)
            }
        }

        readonly property SceneGroup sceneGroup: SceneGroup {
            id: _sceneGroup

            structure: Scrite.document.structure
            evaluateLengths: true

            function refresh() {
                root.screenplayAdapter.screenplay.gatherSelectedScenes(_sceneGroup)
            }
        }

        readonly property FontMetrics fontMetrics: FontMetrics {
            font.family: Runtime.sceneEditorFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }

        readonly property string dragDropMimeType: "sceneListView/sceneID"
        readonly property real sceneIconSize: fontMetrics.height
        readonly property real sceneIconPadding: 8

        property int delegateCount: 0

        function updateCacheBuffer() {
            if(tracksVisible)
                cacheBuffer = contentHeight
            else
                cacheBuffer = 0
        }

        Component.onCompleted: sceneGroup.refresh()
    }
}

