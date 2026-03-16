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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Layouts

import io.scrite.components

import "./globals"
import "./controls"
import "./helpers"
import "./dialogs"
import "./screenplayeditor"
import "./timelineview"

Item {
    id: root

    property real zoomLevel: 2
    property real preferredHeight: _screenplayTools.preferredHeight
    property bool showNotesIcon: false
    property bool enableDragDrop: !Scrite.document.readOnly
    property bool showCursor: Runtime.timelineViewSettings.showCursor

    property alias tracksHeight: _screenplayTracksView.implicitHeight

    signal requestEditor()

    clip: true

    RowLayout {
        anchors.fill: parent

        spacing: 0

        TimelineTools {
            id: _screenplayTools

            Layout.fillHeight: true

            canZoomIn: root.zoomLevel < _private.maximumZoomLevel
            canZoomOut: root.zoomLevel > _private.minimumZoomLevel

            ToolTipPopup {
                background: Rectangle {
                    color: Runtime.colors.accent.c500.background
                    opacity: 0.9
                }

                delay: 0
                text: {
                    const sceneGroup = _sceneGroup
                    const fields = [
                                     sceneGroup.sceneCount + " scene(s)",
                                     "<b>Duration</b> " + (sceneGroup.evaluatingLengths ? "...." : TMath.timeLengthString(sceneGroup.timeLength)),
                                     "<b>Page Count</b> " + (sceneGroup.evaluatingLengths ? "...." : sceneGroup.pageCount + " page(s)")
                                 ]
                    return "<p>Scene Selection:</p>" + SMath.formatAsBulletPoints(fields)
                }
                visible: _sceneGroup.evaluateLengths && _sceneGroup.sceneCount >= 2
                parseShortcutInText: false
            }

            onClearRequest: {
                MessageBox.question("Clear Confirmation",
                    "Are you sure you want to clear the screenplay?",
                    ["Yes", "No"],
                    (buttonText) => {
                        if(buttonText === "Yes") {
                            _screenplayElementList.forceActiveFocus()
                            Scrite.document.screenplay.clearElements()
                        }
                    }
                )
            }

            onZoomInRequest: {
                root.zoomLevel = Math.min(root.zoomLevel * 1.1, _private.maximumZoomLevel)
                _screenplayElementList.updateCacheBuffer()

                const ci = Scrite.document.screenplay.currentElementIndex
                if(ci >= 0)
                    _screenplayElementList.positionViewAtIndex(ci, ListView.Center)
            }

            onZoomOutRequest: {
                root.zoomLevel = Math.max(root.zoomLevel * 0.9, _private.minimumZoomLevel)
                _screenplayElementList.updateCacheBuffer()

                const ci = Scrite.document.screenplay.currentElementIndex
                if(ci >= 0)
                    _screenplayElementList.positionViewAtIndex(ci, ListView.Center)
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            DropArea {
                id: _mainDropArea
                anchors.fill: parent
                keys: [Runtime.timelineViewSettings.dropAreaKey]
                enabled: _screenplayElementList.count === 0 && root.enableDragDrop

                onEntered: (drag) => {
                               _screenplayElementList.forceActiveFocus()
                               drag.acceptProposedAction()
                           }

                onDropped: (drop) => {
                               _private.dropSceneAt(drop.source, Scrite.document.screenplay.elementCount)
                               drop.acceptProposedAction()
                           }
            }

            ColumnLayout {
                anchors.fill: parent

                spacing: 0

                ScreenplayTracksView {
                    id: _screenplayTracksView

                    Layout.fillWidth: true

                    listView: _screenplayElementList
                    screenplay: _screenplayElementList.model
                    visible: Runtime.screenplayTracksSettings.displayTracks
                }

                TimelineListView {
                    id: _screenplayElementList

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    showCursor: root.showCursor
                    mainDropArea: _mainDropArea
                    zoomLevel: root.zoomLevel
                    dragDropEnabled: root.enableDragDrop
                    visibleTrackCount: _screenplayTracksView.trackCount

                    onEditorRequest: _private.requestEditorLater()
                    onDropSceneAtRequest: (source, index) => { _private.dropSceneAt(source, index) }

                    Loader {
                        anchors.fill: parent

                        active: Runtime.applicationSettings.enableAnimations && !_screenplayElementList.FocusTracker.hasFocus

                        sourceComponent: TimelineViewHighlightedItemAnimation {
                            screenplayElementList: _screenplayElementList
                        }
                    }
                }
            }
        }
    }

    ScreenplayBreakElementsContextMenu {
        id: _breakElementContextMenu
    }

    ScreenplaySceneElementsContextMenu {
        id: _sceneElementsContextMenu

        sceneGroup: _sceneGroup
    }

    // Private Section
    QtObject {
        id: _private

        readonly property real maximumZoomLevel: 4
        property real minimumZoomLevel: _screenplayElementList.perElementWidth/_screenplayElementList.minimumDelegateWidth

        function dropSceneAt(source, index) {
            if(source === null)
                return

            _dropSceneTask.dropSource = source
            _dropSceneTask.dropIndex = index
            _dropSceneTask.start()
        }

        function requestEditorLater() {
            Runtime.execLater(root, 100, root.requestEditor)
        }

        function saveZoomLevel() {
            let userData = Scrite.document.userData
            userData["timelineView"] = {
                "version": 0,
                "zoomLevel": root.zoomLevel
            }
            Scrite.document.userData = userData
        }

        function restoreZoomLevel() {
            const userData = Scrite.document.userData || {}
            const timelineView = userData.timelineView
            if(!timelineView || timelineView.version !== 0)
                return

            const zl = timelineView.zoomLevel
            if(typeof zl === "number")
                root.zoomLevel = Runtime.bounded(_private.minimumZoomLevel, zl, _private.maximumZoomLevel)
        }

        Component.onCompleted: {
            restoreZoomLevel()
            _sceneGroup.refresh()
        }
        Component.onDestruction: saveZoomLevel()
    }

    Component {
        id: _screenplayElementComponent

        ScreenplayElement {
            screenplay: Scrite.document.screenplay
        }
    }

    SceneGroup {
        id: _sceneGroup

        structure: Scrite.document.structure
        evaluateLengths: true

        function refresh() {
            clearScenes()
            if(Scrite.document.screenplay.hasSelectedElements)
                Scrite.document.screenplay.gatherSelectedScenes(_sceneGroup)
            else
                addScene(Scrite.document.screenplay.activeScene)
        }
    }

    Connections {
        id: _screenplayConnections

        target: Scrite.document.screenplay

        function onSelectionChanged() {
            Qt.callLater(_sceneGroup.refresh)
        }

        function onCurrentElementIndexChanged() {
            Qt.callLater(_sceneGroup.refresh)
        }
    }


    Connections {
        target: Scrite.document.screenplay

        function onCurrentElementIndexChanged(val) {
            if(!Scrite.document.loading) {
                Runtime.execLater(_screenplayElementList, 150, function() {
                    if(_screenplayElementList.currentIndex === 0)
                        _screenplayElementList.positionViewAtBeginning()
                    else if(_screenplayElementList.currentIndex === _screenplayElementList.count-1)
                        _screenplayElementList.positionViewAtEnd()
                    else
                        _screenplayElementList.positionViewAtIndex(Scrite.document.screenplay.currentElementIndex, ListView.Contain)
                })
            }
        }

        function onRequestEditorAt(index) {
            _screenplayElementList.positionViewAtIndex(index, ListView.Contain)
        }
    }

    Connections {
        target: Scrite.document

        function onAboutToSave() {
            _private.saveZoomLevel()
        }
    }

    SequentialAnimation {
        id: _dropSceneTask

        property var dropSource // must be a QObject subclass
        property int dropIndex

        PauseAnimation { duration: 50 }

        ScriptAction {
            script: {
                const source = _dropSceneTask.dropSource
                const index = _dropSceneTask.dropIndex

                _dropSceneTask.dropSource = null
                _dropSceneTask.dropIndex = -2

                let sourceType = Object.typeOf(source)

                if(sourceType === "ScreenplayElement") {
                    Scrite.document.screenplay.moveSelectedElements(index)
                    return
                }

                let sceneID = source.id
                if(sceneID.length === 0)
                    return

                let scene = Scrite.document.structure.findElementBySceneID(sceneID)
                if(scene === null)
                    return

                let element = _screenplayElementComponent.createObject()
                element.sceneID = sceneID
                Scrite.document.screenplay.insertElementAt(element, index)
                _private.requestEditorLater()
            }
        }
    }

    TrackerPack {
        id: _trackerForUpdateCacheBuffer

        TrackSignal {
            target: _screenplayTracksView
            signal: "trackCountChanged()"
        }

        TrackProperty {
            target: Scrite.document.screenplay
            property: "currentElementIndex"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementsChanged()"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementInserted(ScreenplayElement*,int)"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementRemoved(ScreenplayElement*,int)"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementMoved(ScreenplayElement*,int,int)"
        }

        onTracked: _screenplayElementList.updateCacheBuffer()
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("copy")
        enabled: true

        onTriggered: () => {
                         Scrite.document.screenplay.copySelection()
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("paste")
        enabled: !Scrite.document.readOnly && Scrite.document.screenplay.canPaste

        onTriggered: () => {
                         Scrite.document.screenplay.pasteAfter(Scrite.document.screenplay.currentElementIndex)
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("remove")
        enabled: !Scrite.document.readOnly && Scrite.document.screenplay.canPaste

        onTriggered: () => {
                         if(_sceneGroup.sceneCount <= 1)
                             Scrite.document.screenplay.removeElement(Scrite.document.screenplay.elementAt(Scrite.document.screenplay.currentElementIndex))
                         else
                             Scrite.document.screenplay.removeSelectedElements();
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("keywords")
        enabled: !Scrite.document.readOnly

        onTriggered: () => {
                         SceneGroupKeywordsDialog.launch(_sceneGroup)
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("clearSelection")
        enabled: Scrite.document.screenplay.hasSelectedElements

        onTriggered: () => {
                         Scrite.document.screenplay.clearSelection()
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("makeSequence")
        enabled: !Scrite.document.readOnly && _sceneGroup.canBeStacked

        onTriggered: () => {
                         if(!_sceneGroup.stack()) {
                             MessageBox.information("Make Sequence Error",
                                                    "Couldn't stack these scenes to make a sequence. Please try doing this on the Structure Tab.")
                         }
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("breakSequence")
        enabled: !Scrite.document.readOnly && _sceneGroup.canBeUnstacked

        onTriggered: () => {
                         if(!_sceneGroup.unstack()) {
                             MessageBox.information("Break Sequence Error",
                                                    "Couldn't unstack these scenes to make a sequence. Please try doing this on the Structure Tab.")
                         }
                     }
    }

    ActionHandler {
        property bool omitted: Scrite.document.screenplay.selectedElementsOmitStatus !== Screenplay.NotOmitted
        property string text: omitted ? "Include" : "Omit"

        action: ActionHub.sceneListPanelOptions.find("includeOmit")
        enabled: !Scrite.document.readOnly

        onTriggered: () => {
                         if(omitted)
                             Scrite.document.screenplay.includeSelectedElements()
                         else
                             Scrite.document.screenplay.omitSelectedElements()
                     }
    }
}
