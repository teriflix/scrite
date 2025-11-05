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
    required property bool showSceneSidePanel
    required property real zoomLevel
    required property real spaceAvailableOnTheLeft // Space available to the left of this list-view in the container where its placed
    required property real spaceAvailableOnTheRight // Space available to the left of this list-view in the container where its placed

    required property ScreenplayAdapter screenplayAdapter

    property var additionalSceneMenuItems: []

    readonly property alias hasFocus: _private.hasFocus
    readonly property alias currentDelegate: _private.currentDelegate
    readonly property alias currentDelegateIndex: _private.currentIndex
    readonly property alias currentParagraphType: _private.currentParagraphType
    readonly property alias currentDelegateLoader: _private.currentDelegateLoader
    readonly property alias lastVisibleDelegateIndex: _private.lastItemIndex
    readonly property alias firstVisibleDelegateIndex: _private.firstItemIndex

    signal additionalSceneMenuItemClicked(string name)

    function isVisible(index) {
        return _private.isVisible(index)
    }

    function scrollToFirstScene() {
        positionViewAtBeginning()
    }

    function scrollToLastScene() {
        positionViewAtEnd()
    }

    function scrollIntoView(index) {
        _private.scrollIntoView(index)
    }

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

    Component.onCompleted: _private.updateFirstAndLastIndexLater()

    objectName: "ScreenplayEditorListView"

    model: screenplayAdapter
    currentIndex: -1

    highlightMoveDuration: 0
    highlightResizeDuration: 0
    highlightFollowsCurrentItem: false

    header: root.screenplayAdapter.isSourceScreenplay ? _private.header : null
    footer: root.screenplayAdapter.isSourceScreenplay && Runtime.screenplayEditorSettings.displayAddSceneBreakButtons ? _private.footer : null

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
            _private.scheduleMakeItemUnderCursorCurrent()
    }

    onWidthChanged: _private.updateFirstAndLastIndexLater()
    onHeightChanged: _private.updateFirstAndLastIndexLater()
    onOriginYChanged: _private.updateFirstAndLastIndexLater()
    onContentYChanged: _private.updateFirstAndLastIndexLater()

    QtObject {
        id: _private

        property int currentIndex: root.screenplayAdapter && root.screenplayAdapter.elementCount === root.count ? root.screenplayAdapter.currentIndex : -1
        property int currentParagraphType: currentDelegate ? currentDelegate.currentParagraphType : -1

        property Loader currentDelegateLoader: currentIndex >= 0 ? root.itemAtIndex(currentIndex) : null
        property AbstractScreenplayElementDelegate currentDelegate: currentDelegateLoader ? currentDelegateLoader.item : null

        property int lastItemIndex: -1
        property int firstItemIndex: -1

        property bool hasFocus: false
        property bool scrolling: scrollBarActive || root.moving
        property bool scrollBarActive: root.ScrollBar.vertical ? root.ScrollBar.vertical.pressed : false
        property bool modelCurrentIndexChangedInternally: false

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
            showSceneSidePanel: root.showSceneSidePanel
            additionalSceneMenuItems: root.additionalSceneMenuItems
            spaceAvailableForScenePanel: root.spaceAvailableOnTheRight

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

            onJumpToLastScene: () => { _private.jumpToLastScene() }
            onJumpToNextScene: () => { _private.jumpToNextScene() }
            onJumpToFirstScene: () => { _private.jumpToFirstScene() }
            onJumpToPreviousScene: () => { _private.jumpToPreviousScene() }
            onScrollToNextSceneRequest: () => { _private.scrollToNextScene() }
            onScrollToPreviousSceneRequest: () => { _private.scrollToPreviousScene() }

            onSplitSceneRequest: (paragraph, cursorPosition) => {
                                     if(root.screenplayAdapter.isSourceScreenplay) {
                                         root.screenplayAdapter.splitElement(screenplayElement, paragraph, cursorPosition)
                                     } else {
                                         MessageBox.information("Split Scene", "Scenes can be split only while editing the entire screenplay.")
                                     }
                                 }

            onMergeWithPreviousSceneRequest: () => {
                                                 if(root.screenplayAdapter.isSourceScreenplay) {
                                                     root.screenplayAdapter.mergeElementWithPrevious(screenplayElement)
                                                 } else {
                                                     MessageBox.information("Merge Scene", "Scenes can be merged only while editing the entire screenplay.")
                                                 }
                                             }

            onAdditionalSceneMenuItemClicked: (name) => { root.additionalSceneMenuItemClicked(name) }
        }

        readonly property Connections screenplayAdapterSignals: Connections {
            target: root.screenplayAdapter

            function onRowsInserted() {
                _private.updateFirstAndLastIndexLater()
            }

            function onRowsRemoved() {
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

                const index = root.screenplayAdapter.currentIndex
                if(_private.isVisible(index))
                    return

                if(index < 0)
                    root.positionViewAtBeginning()
                else
                    root.positionViewAtIndex(index, ListView.Beginning)

                if(_private.hasFocus) {
                    /**
                      We cannot use _private.currentItem or _private.currentDelegate at this point,
                      because those bound properties may not have gotten updated just yet.
                      */
                    const item = root.itemAtIndex(index)
                    if(item) {
                        const delegate = item.item
                        if(delegate)
                            delegate.focusIn(0)
                    }
                }

                _private.updateFirstAndLastIndexLater()
            }
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

        function isVisible(index) {
            if(root.contentHeight <= root.height)
                return true

            if( (firstItemIndex < 0 && lastItemIndex < 0) || (firstItemIndex === 0 && lastItemIndex === root.count-1) ) {
                updateFirstAndLastIndex()
            }

            return index >= firstItemIndex && index <= lastItemIndex
        }

        function updateFirstAndLastIndex() {
            if(root.screenplayAdapter === null) {
                firstItemIndex = -1
                lastItemIndex = -1
                return
            }

            const firstPt = root.contentItem.mapFromItem(root, root.width / 2, 1)
            const lastPt = root.contentItem.mapFromItem(root, root.width / 2, root.height-1)

            let first = root.indexAt(firstPt.x, firstPt.y);
            let last = root.indexAt(lastPt.x, lastPt.y);

            if (first === -1)
                first = 0;

            if (last === -1)
                last = root.screenplayAdapter.elementCount - 1;

            firstItemIndex = first
            lastItemIndex = last
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

        function makeItemUnderCursorCurrent() {
            const globalCursorPos = MouseCursor.position()
            let localCursorPos = MouseCursor.itemPosition(root, globalCursorPos)
            localCursorPos.x = root.width/2
            if(localCursorPos.y >= 0 && localCursorPos.y < root.height) {
                localCursorPos = root.contentItem.mapFromItem(root, localCursorPos.x, localCursorPos.y)
                currentIndex = root.indexAt(localCursorPos.x, localCursorPos.y)
                if(currentIndex >= 0 && currentIndex <= root.count)
                    changeAdapterCurrentIndexInternally(currentIndex)
            }
        }

        function scheduleMakeItemUnderCursorCurrent() {
            Runtime.execLater(_private, Runtime.screenplayEditorSettings.placeholderInterval, _private.makeItemUnderCursorCurrent)
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

        function ensureVisible(item, rect) {
            if(item === null || rect === undefined)
                return

            const pt = item.mapToItem(root.contentItem, rect.x, rect.y)
            const endY = root.contentY + root.height - rect.height
            const startY = root.contentY
            if( pt.y >= startY && pt.y <= endY )
                return

            if( pt.y < startY )
                root.contentY = Math.round(pt.y)
            else
                root.contentY = Math.round((pt.y + 2*rect.height) - root.height)
        }

        function jumpToNextScene() {
            const cidx = currentIndex
            const nidx = root.screenplayAdapter.nextSceneElementIndex()
            if(cidx === nidx)
                return

            root.positionViewAtIndex(nidx, ListView.Beginning)
            changeAdapterCurrentIndexInternally(nidx)
            root.positionViewAtIndex(nidx, ListView.Beginning)
            Qt.callLater(focusCurrentDelegateAt, 0)
        }

        function jumpToLastScene() {
            const cidx = currentIndex
            const lidx = root.screenplayAdapter.lastSceneElementIndex()
            if(cidx === lidx)
                return

            root.positionViewAtIndex(lidx, ListView.Beginning)
            changeAdapterCurrentIndexInternally(lidx)
            root.positionViewAtIndex(lidx, ListView.Beginning)
            Qt.callLater(focusCurrentDelegateAt, 0)
        }

        function jumpToFirstScene() {
            const cidx = currentIndex
            const fidx = root.screenplayAdapter.firstSceneElementIndex()
            if(cidx === fidx)
                return

            root.positionViewAtIndex(fidx, ListView.Beginning)
            changeAdapterCurrentIndexInternally(fidx)
            root.positionViewAtIndex(fidx, ListView.Beginning)
            Qt.callLater(focusCurrentDelegateAt, 0)
        }

        function jumpToPreviousScene() {
            const cidx = currentIndex
            const pidx = root.screenplayAdapter.previousSceneElementIndex()
            if(cidx === pidx)
                return

            root.positionViewAtIndex(pidx, ListView.Beginning)
            changeAdapterCurrentIndexInternally(pidx)
            root.positionViewAtIndex(pidx, ListView.Beginning)
            Qt.callLater(focusCurrentDelegateAt, 0)
        }

        function scrollToNextScene() {
            const cidx = currentIndex
            const nidx = root.screenplayAdapter.nextSceneElementIndex()
            if(cidx === nidx) {
                root.positionViewAtEnd()
                return
            }

            changeAdapterCurrentIndexInternally(nidx)
            scrollIntoView(nidx)
            Qt.callLater(focusCurrentDelegateAt, 0)
        }

        function scrollToPreviousScene() {
            const cidx = currentIndex
            const pidx = root.screenplayAdapter.previousSceneElementIndex()
            if(cidx === pidx) {
                root.positionViewAtIndex(cidx, ListView.Beginning)
                return
            }

            changeAdapterCurrentIndexInternally(pidx)
            scrollIntoView(pidx)
            Qt.callLater(focusCurrentDelegateAt, -1)
        }

        function focusCurrentDelegateAt(cursorPosition) {
            if(currentDelegate)
                currentDelegate.focusIn(cursorPosition)
        }

        onHasFocusChanged: {
            if(root.screenplayAdapter.isSourceScreenplay)
                Runtime.undoStack.screenplayEditorActive = hasFocus
            else
                Runtime.undoStack.sceneEditorActive = hasFocus
        }


        onLastItemIndexChanged: scheduleMakeItemUnderCursorCurrent()
        onFirstItemIndexChanged: scheduleMakeItemUnderCursorCurrent()
    }
}
