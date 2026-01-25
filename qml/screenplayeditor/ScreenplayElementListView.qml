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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Qt.labs.qmlmodels 1.0

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/screenplayeditor/delegates"

ListView {
    id: root

    required property var pageMargins
    required property bool readOnly
    required property bool showSceneComments
    required property real zoomLevel
    required property real spaceAvailableOnTheLeft // Space available to the left of this list-view in the container where its placed
    required property real spaceAvailableOnTheRight // Space available to the left of this list-view in the container where its placed

    required property ScreenplayAdapter screenplayAdapter

    readonly property alias hasFocus: _private.hasFocus
    readonly property alias currentDelegate: _private.currentDelegate
    readonly property alias currentDelegateIndex: _private.currentIndex
    readonly property alias currentParagraphType: _private.currentParagraphType
    readonly property alias currentDelegateLoader: _private.currentDelegateLoader
    readonly property alias lastVisibleDelegateIndex: _private.lastItemIndex
    readonly property alias firstVisibleDelegateIndex: _private.firstItemIndex

    function afterZoomLevelChange() {
        if(currentDelegate)
            currentDelegate.afterZoomLevelChange()
    }

    function beforeZoomLevelChange() {
        if(currentDelegate)
            currentDelegate.beforeZoomLevelChange()
    }

    FocusTracker.window: Scrite.window
    FocusTracker.objectName: "ScreenplayElementListView"
    FocusTracker.evaluationMethod: FocusTracker.StandardFocusEvaluation
    FocusTracker.indicator.target: _private
    FocusTracker.indicator.property: "hasFocus"

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    Component.onCompleted: Qt.callLater(_private.init)

    objectName: "ScreenplayEditorListView"

    model: screenplayAdapter
    currentIndex: -1
    boundsBehavior: ListView.StopAtBounds
    boundsMovement: ListView.StopAtBounds
    keyNavigationEnabled: false // because we handle keyboard interactions ourselves.

    highlightMoveDuration: 0
    highlightResizeDuration: 0
    highlightFollowsCurrentItem: false

    header: root.screenplayAdapter.isSourceScreenplay ? _private.header : null
    footer: root.screenplayAdapter.isSourceScreenplay ? _private.footer : null

    /**
      Why are we doing all this circus instead of using DelegateChooser?

      Initially, I used DelegateChooser with role as "delegateKind", and one DelegateChoice for each
      potential value of delegateKind. But it breaks down when the value of the delegateKind changes
      for a row in the model, becasue DelegateChooser doesn't recreate a delegate from a new choice
      whenever that happens. That's obviously not what we want.
      */
    delegate: Loader {
        id: _delegateLoader

        required property int index
        required property int screenplayElementType
        required property int breakType
        required property string sceneID
        required property string delegateKind
        required property ScreenplayElement screenplayElement

        z: root.screenplayAdapter.currentIndex === index ? 1 : 0

        width: root.width

        objectName: "ScreenplayElementListView-Delegate-" + index
        sourceComponent: _private.pickDelegateComponent(delegateKind)

        onStatusChanged: {
            if(status === Loader.Loading)
                Object.resetProperty(_delegateLoader, "height")
        }
    }

    onMovingChanged: {
        if(!movingVertically)
            _private.scheduleMakeItemUnderMouseCurrent()
    }

    onWidthChanged: _private.updateFirstAndLastIndexLater()
    onHeightChanged: _private.updateFirstAndLastIndexLater()
    onOriginYChanged: _private.updateFirstAndLastIndexLater()
    onContentYChanged: _private.updateFirstAndLastIndexLater()

    Item {
        enabled: _private.hasFocus

        ActionHandler {
            action: ActionHub.editOptions.find("jumpFirstScene")
            enabled: _private.currentIndex > _private.firstSceneElementIndex

            onTriggered: (source) => {
                             _private.jumpToFirstScene()
                         }
        }

        ActionHandler {
            action: ActionHub.editOptions.find("jumpLastScene")
            enabled: _private.currentIndex < _private.lastSceneElementIndex

            onTriggered: (source) => {
                             _private.jumpToLastScene()
                         }
        }

        ActionHandler {
            action: ActionHub.editOptions.find("jumpPreviousScene")
            enabled: _private.currentIndex > _private.firstSceneElementIndex

            onTriggered: (source) => {
                             _private.jumpToPreviousScene()
                         }
        }

        ActionHandler {
            action: ActionHub.editOptions.find("jumpNextScene")
            enabled: _private.currentIndex < _private.lastSceneElementIndex

            onTriggered: (source) => {
                             _private.jumpToNextScene()
                         }
        }

        ActionHandler {
            action: ActionHub.editOptions.find("scrollPreviousScene")
            enabled: _private.currentIndex > _private.firstSceneElementIndex

            onTriggered: (source) => {
                             _private.scrollToPreviousScene()
                         }
        }

        ActionHandler {
            action: ActionHub.editOptions.find("scrollNextScene")
            enabled: _private.currentIndex < _private.lastSceneElementIndex

            onTriggered: (source) => {
                             _private.scrollToNextScene()
                         }
        }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("jumpToSceneNumber")
        enabled: _private.firstSceneElementIndex !== _private.lastSceneElementIndex

        onTriggered: (source) => {
                         JumpToSceneNumberDialog.launch(root.screenplayAdapter)
                     }
    }

    QtObject {
        id: _private

        readonly property Action editSceneHeading: ActionHub.paragraphFormats.find("headingParagraph")
        readonly property Action editSceneContent: ActionHub.editOptions.find("editSceneContent")
        readonly property Action focusCursorPosition: ActionHub.editOptions.find("focusCursorPosition")
        readonly property Action ensureCursorCentered: Action {
            function go() {
                root.returnToBounds()
                const ci = root.screenplayAdapter.currentIndex
                root.positionViewAtIndex(ci, ListView.Center)
                _private.editSceneContent.trigger()
                if(ActionHandler.canHandle)
                   trigger()
            }
        }

        property int currentIndex: root.screenplayAdapter ? root.screenplayAdapter.currentIndex : -1
        property int currentParagraphType: currentDelegate ? currentDelegate.currentParagraphType : -1

        property Loader currentDelegateLoader: hasFocus || currentIndex >= 0 ? root.itemAtIndex(currentIndex) : null
        property AbstractScreenplayElementDelegate currentDelegate: currentDelegateLoader ? currentDelegateLoader.item : null

        property int lastItemIndex: -1
        property int firstItemIndex: -1

        property int lastSceneElementIndex: -1
        property int firstSceneElementIndex: -1

        property bool hasFocus: false
        property bool scrolling: scrollBarActive || root.moving
        property bool scrollBarActive: root.ScrollBar.vertical ? root.ScrollBar.vertical.pressed : false
        property bool modelCurrentIndexChangedInternally: false

        readonly property Connections screenplaySignals: Connections {
            target: Scrite.document

            enabled: _private.hasFocus && root.screenplayAdapter.screenplay === Scrite.document.screenplay

            function onNewSceneCreated(scene, elementIndex) {
                if(Runtime.screenplayEditorSettings.focusCursorOnSceneHeadingInNewScenes)
                    Qt.callLater(_private.editSceneHeading.trigger)
                else
                    _private.focusCursorPosition.set(elementIndex, 0)
            }
        }

        readonly property Component header: ScreenplayElementListViewHeader {
            width: root.width

            readOnly: root.readOnly
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter
        }

        readonly property Component footer: Column {
            width: root.width

            Rectangle {
                width: parent.width
                height: Runtime.screenplayEditorSettings.spaceBetweenScenes * root.zoomLevel

                color: Runtime.colors.primary.windowColor
                visible: height > 0
            }

            ScreenplayElementListViewFooter {
                width: parent.width

                readOnly: root.readOnly
                zoomLevel: root.zoomLevel
                pageMargins: root.pageMargins
                screenplayAdapter: root.screenplayAdapter

                visible: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons
            }
        }

        readonly property Component actBreakDelegate: ScreenplayActBreakDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            isCurrent: _private.currentIndex === index
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType

            onHasFocusChanged: {
                if(hasFocus)
                    _private.changeAdapterCurrentIndexInternally(index)
            }

            onEnsureVisible: (item, area) => {
                                 if(!_private.scrolling && !_private.modelCurrentIndexChangedInternally && _private.currentIndex === index)
                                    _private.ensureVisible(item, area)
                             }
        }

        readonly property Component episodeBreakDelegate: ScreenplayEpisodeBreakDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            isCurrent: _private.currentIndex === index
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType

            onHasFocusChanged: {
                if(hasFocus)
                    _private.changeAdapterCurrentIndexInternally(index)
            }

            onEnsureVisible: (item, area) => {
                                 if(!_private.scrolling && !_private.modelCurrentIndexChangedInternally && _private.currentIndex === index)
                                    _private.ensureVisible(item, area)
                             }
        }

        readonly property Component intervalBreakDelegate: ScreenplayIntervalBreakDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            isCurrent: _private.currentIndex === index
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType

            onHasFocusChanged: {
                if(hasFocus)
                    _private.changeAdapterCurrentIndexInternally(index)
            }

            onEnsureVisible: (item, area) => {
                                 if(!_private.scrolling && !_private.modelCurrentIndexChangedInternally && _private.currentIndex === index)
                                    _private.ensureVisible(item, area)
                             }
        }

        readonly property Component omittedSceneDelegate: OmittedScreenplayElementDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            isCurrent: _private.currentIndex === index
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType

            onHasFocusChanged: {
                if(hasFocus)
                    _private.changeAdapterCurrentIndexInternally(index)
            }

            onEnsureVisible: (item, area) => {
                                 if(!_private.scrolling && !_private.modelCurrentIndexChangedInternally && _private.currentIndex === index)
                                    _private.ensureVisible(item, area)
                             }
        }

        readonly property Component sceneDelegate: ScreenplayElementSceneDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            listView: root
            isCurrent: _private.currentIndex === index
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter
            showSceneComments: root.showSceneComments
            ensureCursorCenteredAction: _private.ensureCursorCentered
            spaceAvailableForScenePanel: root.spaceAvailableOnTheRight + root.spaceAvailableOnTheLeft

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType

            usePlaceholder: root.contentHeight > root.height && _private.scrolling

            onHasFocusChanged: {
                if(hasFocus)
                    _private.changeAdapterCurrentIndexInternally(index)
            }

            onEnsureVisible: (item, area) => {
                                 if(!_private.scrolling && !_private.modelCurrentIndexChangedInternally && _private.currentIndex === index)
                                    _private.ensureVisible(item, area)
                             }

            onEnsureCentered: (item, area) => {
                                  if(!_private.scrolling && !_private.modelCurrentIndexChangedInternally && _private.currentIndex === index) {
                                      _private.ensureCentered(item, area)
                                  }
                              }

            onSplitSceneRequest: (paragraph, cursorPosition) => {
                                     Qt.callLater(_private.splitScene, screenplayElement, paragraph, cursorPosition)
                                 }

            onMergeWithPreviousSceneRequest: () => {
                                                 Qt.callLater(_private.mergeWithPreviousScene, screenplayElement)
                                             }
        }

        readonly property Connections screenplayAdapterSignals: Connections {
            target: root.screenplayAdapter

            function onRowsInserted() {
                _private.updateFirstAndLastIndexLater()
            }

            function onRowsRemoved() {
                _private.updateFirstAndLastIndexLater()
            }

            function onRowsMoved() {
                _private.updateFirstAndLastIndexLater()
            }

            function onDataChanged() {
                _private.updateFirstAndLastIndexLater()
            }

            function onModelReset() {
                _private.updateFirstAndLastIndexLater()
            }

            function onCurrentIndexChanged() {
                if(_private.modelCurrentIndexChangedInternally)
                    return

                root.returnToBounds()

                const index = root.screenplayAdapter.currentIndex
                // if(_private.isVisible(index))
                //     return

                if(index < 0)
                    root.positionViewAtBeginning()
                else {
                    root.positionViewAtIndex(index, ListView.Beginning)
                    if(_private.hasFocus)
                        _private.editSceneContent.trigger()
                }

                _private.updateFirstAndLastIndexLater()
            }
        }

        function init() {
            root.returnToBounds()
            const index = root.screenplayAdapter.currentIndex
            if(index < 0)
                root.positionViewAtBeginning()
            else {
                root.positionViewAtIndex(index, ListView.Beginning)
                if(hasFocus)
                    editSceneContent.trigger()
            }
            updateFirstAndLastIndexLater()
        }

        function pickDelegateComponent(delegateKind) {
            switch(delegateKind) {
            case "scene": return sceneDelegate
            case "actBreak": return actBreakDelegate;
            case "omittedScene": return omittedSceneDelegate;
            case "episodeBreak": return episodeBreakDelegate;
            case "invervalBreak": return intervalBreakDelegate;
            }
            return null
        }

        function splitScene(screenplayElement, paragraph, cursorPosition) {
            if(root.screenplayAdapter.isSourceScreenplay) {
                root.screenplayAdapter.splitElement(screenplayElement, paragraph, cursorPosition)
                Qt.callLater(_private.ensureCursorCentered.go)
            } else {
                MessageBox.information("Split Scene", "Scenes can be split only while editing the entire screenplay.")
            }
        }

        function mergeWithPreviousScene(screenplayElement) {
            if(root.screenplayAdapter.isSourceScreenplay) {
                const newElement = root.screenplayAdapter.mergeElementWithPrevious(screenplayElement)
                Runtime.execLater(_private, Runtime.stdAnimationDuration/2, _private.postMergeWithPreviousScene, newElement)
            } else {
                MessageBox.information("Merge Scene", "Scenes can be merged only while editing the entire screenplay.")
            }
        }

        function postMergeWithPreviousScene(newElement) {
            scrollIntoView(root.screenplayAdapter.currentIndex)

            const cursorPosition = newElement.cursorPositionHint >= 0 ?
                                   newElement.cursorPositionHint : newElement.scene.cursorPosition
            _private.focusCursorPosition.set(root.screenplayAdapter.currentIndex, cursorPosition)
            newElement.cursorPositionHint = -1;

            _private.focusCursorPosition.trigger()
            Qt.callLater(_private.ensureCursorCentered.go)
        }

        function isVisible(index) {
            if(root.contentHeight <= root.height)
                return true

            return index >= firstItemIndex && index <= lastItemIndex
        }

        function updateFirstAndLastIndex() {
            if(root.screenplayAdapter === null) {
                firstItemIndex = -1
                lastItemIndex = -1
                firstSceneElementIndex = -1
                lastSceneElementIndex = -1
                return
            }

            const firstPt = root.contentItem.mapFromItem(root, root.width / 2, 1)
            const lastPt = root.contentItem.mapFromItem(root, root.width / 2, root.height-1)

            firstItemIndex = root.indexAt(firstPt.x, firstPt.y);
            lastItemIndex = root.indexAt(lastPt.x, lastPt.y);

            firstSceneElementIndex = root.screenplayAdapter.firstSceneElementIndex()
            lastSceneElementIndex = root.screenplayAdapter.lastSceneElementIndex()
        }

        function updateFirstAndLastIndexLater() {
            Qt.callLater(updateFirstAndLastIndex)
        }

        function changeAdapterCurrentIndexInternally(index) {
            if(root.screenplayAdapter.currentIndex === index)
                return

            modelCurrentIndexChangedInternally = true
            root.screenplayAdapter.currentIndex = index
            modelCurrentIndexChangedInternally = false
        }

        function makeItemUnderMouseCurrent() {
            if(!Runtime.screenplayEditorSettings.autoSelectSceneUnderMouse)
                return

            const globalCursorPos = MouseCursor.position()
            let localCursorPos = MouseCursor.itemPosition(root, globalCursorPos)
            localCursorPos.x = root.width/2
            if(localCursorPos.y >= 0 && localCursorPos.y < root.height) {
                localCursorPos = root.contentItem.mapFromItem(root, localCursorPos.x, localCursorPos.y)
                const ci = root.indexAt(localCursorPos.x, localCursorPos.y)
                if(ci >= 0 && ci <= root.count)
                    changeAdapterCurrentIndexInternally(ci)
            }
        }

        function scheduleMakeItemUnderMouseCurrent() {
            Runtime.execLater(_private, Runtime.placeholderInterval, _private.makeItemUnderMouseCurrent)
        }

        function scrollIntoView(index) {
            if(isVisible(index))
                return

            if(index < 0) {
                root.positionViewAtBeginning()
                return
            }

            if(index < firstItemIndex && firstItemIndex-index <= 2) {
                root.contentY -= root.height*0.2
            } else if(index > lastItemIndex && index-lastItemIndex <= 2) {
                root.contentY += root.height*0.2
            } else {
                root.positionViewAtIndex(index, ListView.Beginning)
            }
        }

        function ensureVisible(item, rect, marginTop, marginBottom) {
            if(item === null || rect === undefined)
                return

            if(marginTop === undefined)
                marginTop = Math.round(root.height * 0.05)
            if(marginBottom === undefined)
                marginBottom = Math.round(root.height * 0.15)

            const pt = item.mapToItem(root.contentItem, rect.x, rect.y)
            const startY = root.contentY + marginTop
            const endY = root.contentY + root.height - rect.height - marginBottom

            if (pt.y >= startY && pt.y <= endY)
                return

            if (pt.y < startY)
                root.contentY = Math.round(pt.y - marginTop)
            else
                root.contentY = Math.round((pt.y + rect.height + marginBottom) - root.height)

            root.returnToBounds()
        }

        function ensureCentered(item, rect) {
            if (item === null || rect === undefined)
                return;

            const pt = item.mapToItem(root.contentItem, rect.x, rect.y);
            root.contentY = Math.round(pt.y - (root.height - rect.height) / 2);
            root.returnToBounds()
        }

        function jumpToNextScene() {
            const cidx = currentIndex
            const nidx = root.screenplayAdapter.nextSceneElementIndex()
            if(cidx === nidx)
                return

            root.positionViewAtIndex(nidx, ListView.Beginning)
            focusCursorPosition.set(nidx, 0)
            changeAdapterCurrentIndexInternally(nidx)
            root.positionViewAtIndex(nidx, ListView.Beginning)
        }

        function jumpToLastScene() {
            const cidx = currentIndex
            const lidx = _private.lastSceneElementIndex
            if(cidx === lidx)
                return

            root.positionViewAtIndex(lidx, ListView.Beginning)
            focusCursorPosition.set(lidx, 0)
            changeAdapterCurrentIndexInternally(lidx)
            root.positionViewAtIndex(lidx, ListView.Beginning)
        }

        function jumpToFirstScene() {
            const cidx = currentIndex
            const fidx = _private.firstSceneElementIndex
            if(cidx === fidx)
                return

            root.positionViewAtIndex(fidx, ListView.Beginning)
            focusCursorPosition.set(fidx, 0)
            changeAdapterCurrentIndexInternally(fidx)
            root.positionViewAtIndex(fidx, ListView.Beginning)
        }

        function jumpToPreviousScene() {
            const cidx = currentIndex
            const pidx = root.screenplayAdapter.previousSceneElementIndex()
            if(cidx === pidx)
                return

            root.positionViewAtIndex(pidx, ListView.Beginning)
            focusCursorPosition.set(pidx, 0)
            changeAdapterCurrentIndexInternally(pidx)
            root.positionViewAtIndex(pidx, ListView.Beginning)
        }

        function scrollToNextScene() {
            const cidx = currentIndex
            const nidx = root.screenplayAdapter.nextSceneElementIndex()
            if(cidx === nidx) {
                root.positionViewAtEnd()
                return
            }

            focusCursorPosition.set(nidx, 0)
            changeAdapterCurrentIndexInternally(nidx)
            scrollIntoView(nidx)
        }

        function scrollToPreviousScene() {
            const cidx = currentIndex
            const pidx = root.screenplayAdapter.previousSceneElementIndex()
            if(cidx === pidx || pidx < 0) {
                root.positionViewAtIndex(cidx, ListView.Beginning)
                return
            }

            focusCursorPosition.set(pidx, -1)
            changeAdapterCurrentIndexInternally(pidx)
            scrollIntoView(pidx)
        }

        onLastItemIndexChanged: if(scrolling || root.flicking) scheduleMakeItemUnderMouseCurrent()
        onFirstItemIndexChanged: if(scrolling || root.flicking) scheduleMakeItemUnderMouseCurrent()
    }
}
