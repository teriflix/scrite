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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Shapes 1.5
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/timelineview"

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

            canZoomIn: zoomLevel < _private.maximumZoomLevel
            canZoomOut: zoomLevel > _private.minimumZoomLevel

            ToolTipPopup {
                background: Rectangle {
                    color: Runtime.colors.accent.c500.background
                    opacity: 0.9
                }

                delay: 0
                text: {
                    const sceneGroup = _private.sceneGroup
                    const fields = [
                                     sceneGroup.sceneCount + " scene(s)",
                                     "<b>Duration</b> " + (sceneGroup.evaluatingLengths ? "...." : TMath.timeLengthString(sceneGroup.timeLength)),
                                     "<b>Page Count</b> " + (sceneGroup.evaluatingLengths ? "...." : sceneGroup.pageCount + " page(s)")
                                 ]
                    return "<p>Scene Selection:</p>" + SMath.formatAsBulletPoints(fields)
                }
                visible: _private.sceneGroup.evaluateLengths && _private.sceneGroup.sceneCount >= 2
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
                root.zoomLevel = Math.min(zoomLevel * 1.1, _private.maximumZoomLevel)
                _screenplayElementList.updateCacheBuffer()

                const ci = Scrite.document.screenplay.currentElementIndex
                if(ci >= 0)
                    _screenplayElementList.positionViewAtIndex(ci, ListView.Center)
            }

            onZoomOutRequest: {
                root.zoomLevel = Math.max(zoomLevel * 0.9, _private.minimumZoomLevel)
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
                enabled: _screenplayElementList.count === 0 && enableDragDrop

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

        sceneGroup: _private.sceneGroup
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("copy")
        enabled: true

        onTriggered: (source) => {
                         Scrite.document.screenplay.copySelection()
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("paste")
        enabled: !Scrite.document.readOnly && Scrite.document.screenplay.canPaste

        onTriggered: (source) => {
                         Scrite.document.screenplay.pasteAfter(Scrite.document.currentIndex)
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("remove")
        enabled: !Scrite.document.readOnly && Scrite.document.screenplay.canPaste

        onTriggered: (source) => {
                         if(_private.sceneGroup.sceneCount <= 1)
                             Scrite.document.screenplay.removeElement(Scrite.document.currentElement)
                         else
                             Scrite.document.screenplay.removeSelectedElements();
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("keywords")
        enabled: !Scrite.document.readOnly

        onTriggered: (source) => {
                         SceneGroupKeywordsDialog.launch(_private.sceneGroup)
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("clearSelection")
        enabled: Scrite.document.screenplay.hasSelectedElements

        onTriggered: (source) => {
                         Scrite.document.screenplay.clearSelection()
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("makeSequence")
        enabled: !Scrite.document.readOnly && _private.sceneGroup.canBeStacked

        onTriggered: (source) => {
                         if(!_private.sceneGroup.stack()) {
                             MessageBox.information("Make Sequence Error",
                                                    "Couldn't stack these scenes to make a sequence. Please try doing this on the Structure Tab.")
                         }
                     }
    }

    ActionHandler {
        action: ActionHub.sceneListPanelOptions.find("breakSequence")
        enabled: !Scrite.document.readOnly && _private.sceneGroup.canBeUnstacked

        onTriggered: (source) => {
                         if(!_private.sceneGroup.unstack()) {
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

        onTriggered: (source) => {
                         if(omitted)
                             Scrite.document.screenplay.includeSelectedElements()
                         else
                             Scrite.document.screenplay.omitSelectedElements()
                     }
    }

    Component {
        id: _screenplayElementComponent

        ScreenplayElement {
            screenplay: Scrite.document.screenplay
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

    QtObject {
        id: _private

        readonly property real maximumZoomLevel: 4
        property real minimumZoomLevel: _screenplayElementList.perElementWidth/_screenplayElementList.minimumDelegateWidth

        readonly property Connections screenplayConnections: Connections {
            target: Scrite.document.screenplay

            function onSelectionChanged() {
                Qt.callLater(_private.sceneGroup.refresh)
            }

            function onCurrentElementIndexChanged() {
                Qt.callLater(_private.sceneGroup.refresh)
            }
        }

        readonly property SceneGroup sceneGroup: SceneGroup {
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

        property SequentialAnimation dropSceneTask : SequentialAnimation {
            property var dropSource // must be a QObject subclass
            property int dropIndex

            PauseAnimation { duration: 50 }

            ScriptAction {
                script: {
                    const source = _private.dropSceneTask.dropSource
                    const index = _private.dropSceneTask.dropIndex

                    _private.dropSceneTask.dropSource = null
                    _private.dropSceneTask.dropIndex = -2

                    var sourceType = Object.typeOf(source)

                    if(sourceType === "ScreenplayElement") {
                        Scrite.document.screenplay.moveSelectedElements(index)
                        return
                    }

                    var sceneID = source.id
                    if(sceneID.length === 0)
                        return

                    var scene = Scrite.document.structure.findElementBySceneID(sceneID)
                    if(scene === null)
                        return

                    var element = _screenplayElementComponent.createObject()
                    element.sceneID = sceneID
                    Scrite.document.screenplay.insertElementAt(element, index)
                    _private.requestEditorLater()
                }
            }
        }

        readonly property TrackerPack trackerForUpdateCacheBuffer: TrackerPack {
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

        function dropSceneAt(source, index) {
            if(source === null)
                return

            dropSceneTask.dropSource = source
            dropSceneTask.dropIndex = index
            dropSceneTask.start()
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
            const userData = Scrite.document.userData
            if(userData.timelineView && userData.timelineView.version === 0) {
                const zl = userData.timelineView.zoomLevel
                if(typeof zl === "number")
                    root.zoomLevel = Runtime.bounded(_private.minimumZoomLevel, zl, _private.maximumZoomLevel);
            }
        }

        Component.onCompleted: {
            restoreZoomLevel()
            sceneGroup.refresh()
        }
        Component.onDestruction: saveZoomLevel()
    }
}
