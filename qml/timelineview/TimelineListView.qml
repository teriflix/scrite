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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

ListView {
    id: root

    required property DropArea mainDropArea

    readonly property real omittedDelegateWidth: 34
    readonly property real breakDelegateWidth: 70
    readonly property real perElementWidth: 2.5
    readonly property real minimumDelegateWidthForTextVisibility: 50

    property bool moveMode: false
    property bool mutiSelectionMode: false
    property bool scrollBarRequired: root.width < root.contentWidth
    property real minimumDelegateWidth: {
        var treshold = Math.floor(width / 100)
        if(Scrite.document.screenplay.elementCount < treshold)
            return 100

        var pc = Scrite.document.screenplay.maximumParagraphCount
        if(pc < 4)
            return 100
        if(pc >= 5 && pc < 10)
            return 50
        return 34
    }
    property color dropAreaHighlightColor: Runtime.colors.accent.highlight.background

    function updateCacheBuffer() { _private.updateCacheBuffer() }
    function extents(startIndex, endIndex) { return _private.extents(startIndex, endIndex) }

    signal editorRequest()
    signal dropSceneAtRequest(QtObject source, int index)

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    Keys.onRightPressed: Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.currentElementIndex+1
    Keys.onLeftPressed: Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.currentElementIndex-1
    Keys.onPressed: (event) => {
        event.accepted = false
        if(event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
            if(event.isAutoRepeat)
                return
            if(event.modifiers !== Qt.NoModifier)
                return
            Qt.callLater(_private.removeCurrentElement)
            event.accepted = true
        }
    }


    FocusTracker.window: Scrite.window
    FocusTracker.indicator.target: Runtime.undoStack
    FocusTracker.indicator.property: "timelineEditorActive"

    EventFilter.active: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform
    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (watched, event, result) => {
        if(event.delta < 0)
            contentX = Math.min(contentX+20, contentWidth-width)
        else
            contentX = Math.max(contentX-20, 0)
        result.acceptEvent = true
        result.filter = true
    }

    ScrollBar.horizontal: VclScrollBar {
        flickable: root
        opacity: 1
    }

    Behavior on minimumDelegateWidth {
        enabled: Runtime.applicationSettings.enableAnimations
        NumberAnimation { duration: Runtime.stdAnimationDuration }
    }

    clip: true
    model: Scrite.document.loading ? null : Scrite.document.screenplay

    orientation: Qt.Horizontal
    currentIndex: Scrite.document.screenplay.currentElementIndex

    move: _moveAndDisplace
    moveDisplaced: _moveAndDisplace

    preferredHighlightEnd: width*0.6
    preferredHighlightBegin: width*0.2

    highlightRangeMode: FocusTracker.hasFocus ? ListView.NoHighlightRange : ListView.ApplyRange
    highlightMoveDuration: 0
    highlightResizeDuration: 0
    highlightFollowsCurrentItem: true

    footer: TimelineViewFooter {
        mainDropArea: root.mainDropArea
        screenplayElementList: root
        dropAreaHighlightColor: root.dropAreaHighlightColor

        onDropSceneAtRequest: (source, index) => { root.dropSceneAtRequest(source,index) }
    }

    highlight: Item {
        Item {
            anchors.leftMargin: 7.5
            anchors.rightMargin: 2.5
            anchors.bottomMargin: root.scrollBarRequired ? 20 : 10
            anchors.fill: parent

            BoxShadow {
                anchors.fill: parent
            }
        }
    }

    delegate: TimelineViewDelegate {
        screenplayElementList: root

        onEditorRequest: root.editorRequest()
        onDropSceneAtRequest: (source, index) => { root.dropSceneAtRequest(source,index) }
    }

    onCountChanged: updateCacheBuffer()

    Transition {
        id: _moveAndDisplace
        NumberAnimation { properties: "x,y"; duration: 250 }
    }

    QtObject {
        id: _private

        EventFilter.target: Scrite.app
        EventFilter.active: Scrite.document.screenplay.hasSelectedElements
        EventFilter.events: [EventFilter.KeyPress]
        EventFilter.onFilter: (object,event,result) => {
                                  if(event.key === Qt.Key_Escape) {
                                      Scrite.document.screenplay.clearSelection()
                                      result.acceptEvent = true
                                      result.filter = true
                                  }
                              }

        function removeCurrentElement() {
            if(Scrite.document.loading)
                return
            var cidx = currentIndex
            if(cidx < 0)
                return
            var celement = Scrite.document.screenplay.elementAt(cidx)
            if(celement)
                Scrite.document.screenplay.removeElement(celement)
        }

        function updateCacheBuffer() {
            if(Runtime.screenplayTracks.trackCount > 0)
                cacheBuffer = Math.max(extents(count-1, count-1).to + 20, contentWidth)
            else
                cacheBuffer = 0
        }

        function extents(startIndex, endIndex) {
            var x = 0;
            var ret = { "from": 0, "to": 0 }
            if(startIndex < 0 || endIndex < 0)
                return ret;

            var idx = -1
            var nrElements = Scrite.document.screenplay.elementCount
            for(var i=0; i<nrElements; i++) {
                var element = Scrite.document.screenplay.elementAt(i)
                if(element.elementType === ScreenplayElement.SceneElementType)
                    ++idx
                if(idx === startIndex)
                    ret.from = x+7.5
                if(element.elementType === ScreenplayElement.BreakElementType)
                    x += breakDelegateWidth
                else {
                    var sceneElementCount = element.scene ? element.scene.elementCount : 1
                    x += Math.max(minimumDelegateWidth, sceneElementCount*perElementWidth*zoomLevel)
                }
                if(idx === endIndex)
                    break
            }
            ret.to = x-2.5
            return ret
        }
    }
}

