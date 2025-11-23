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

    property real zoomLevel: 1
    property real preferredHeight: _screenplayTools.preferredHeight
    property bool showNotesIcon: false
    property bool enableDragDrop: !Scrite.document.readOnly

    property alias tracksHeight: _screenplayTracksView.implicitHeight

    signal requestEditor()

    clip: true

    RowLayout {
        anchors.fill: parent

        spacing: 0

        TimelineTools {
            id: _screenplayTools

            Layout.fillHeight: true

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
                root.zoomLevel = Math.min(zoomLevel * 1.1, 4.0)
                _screenplayElementList.updateCacheBuffer()
            }

            onZoomOutRequest: {
                root.zoomLevel = Math.max(zoomLevel * 0.9, _screenplayElementList.perElementWidth/_screenplayElementList.minimumDelegateWidth)
                _screenplayElementList.updateCacheBuffer()
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

    QtObject {
        id: _private

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
    }

}
