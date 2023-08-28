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
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import io.scrite.components 1.0
import "../js/utils.js" as Utils

Rectangle {
    // This editor has to specialize in rendering scenes within a ScreenplayAdapter
    // The adapter may contain a single scene or an entire screenplay, that doesnt matter.
    // This way we can avoid having a SceneEditor and ScreenplayEditor as two distinct
    // QML components.

    id: screenplayEditor
    property ScreenplayFormat screenplayFormat: Scrite.document.displayFormat
    property ScreenplayPageLayout pageLayout: screenplayFormat.pageLayout
    property alias source: sourcePropertyAlias.value
    property bool toolBarVisible: toolbar.visible
    property bool commentsPanelAllowed: true
    property alias enableSceneListPanel: sceneListSidePanel.visible
    property alias sceneListPanelExpanded: sceneListSidePanel.expanded
    property var additionalCharacterMenuItems: []
    property var additionalSceneMenuItems: []
    signal additionalCharacterMenuItemClicked(string characterName, string menuItemName)
    signal additionalSceneMenuItemClicked(Scene scene, string menuItemName)

    property alias zoomLevel: zoomSlider.zoomLevel
    property int zoomLevelModifier: 0

    function zoomLevelModifierToApply() {
        return zoomSlider.zoomLevelModifierToApply()
    }

    color: primaryColors.windowColor
    clip: true

    PropertyAlias {
        id: sourcePropertyAlias
        sourceObject: screenplayAdapter
        sourceProperty: "source"
    }

    QtObject {
        id: privateData

        readonly property int _InternalSource: 0
        readonly property int _ExternalSource: 1
        property int currentIndexChangeSoruce: _ExternalSource

        function changeCurrentIndexTo(val) {
            currentIndexChangeSoruce = _InternalSource
            screenplayAdapter.currentIndex = val
            privateData.currentIndexChangeSoruce = privateData._ExternalSource
        }
    }

    Connections {
        id: screenplayAdapterConnections
        target: screenplayAdapter

        function internalSwitchToCurrentIndex() {
            screenplayEditorBusyOverlay.reset()
            forceContentViewPosition.stop()

            const currentIndex = screenplayAdapter.currentIndex
            if(currentIndex < 0) {
                contentView.scrollToFirstScene()
                return
            }

            Utils.execLater(contentView, 100, function() {
                contentView.scrollIntoView(currentIndex)
            })
        }

        function externalSwitchToCurrentIndex() {
            screenplayEditorBusyOverlay.ref()
            contentView.focus = false

            Utils.execLater(contentView, Scrite.app.isMacOSPlatform ? 50 : 10, () => {
                                positionViewAtCurrentIndex()
                                screenplayEditorBusyOverlay.reset()
                            })
        }

        function positionViewAtCurrentIndexLater() {
            forceContentViewPosition.start()
        }

        function positionViewAtCurrentIndex() {
            const currentIndex = screenplayAdapter.currentIndex
            if(currentIndex < 0)
                contentView.scrollToFirstScene()
            else
                contentView.positionViewAtIndex(currentIndex, ListView.Beginning)
        }

        function onCurrentIndexChanged(val) {
            if(privateData.currentIndexChangeSoruce === privateData._InternalSource)
                internalSwitchToCurrentIndex()
            else
                externalSwitchToCurrentIndex()
        }

        function onSourceChanged() {
            contentView.commentsExpandCounter = 0
            contentView.commentsExpanded = false
            screenplayEditorBusyOverlay.reset()
        }
    }

    Connections {
        id: delegateLoadedConnection
        target: contentView
        enabled: !contentViewIsBeingAltered.get
        function onDelegateLoaded() {
            screenplayAdapterConnections.positionViewAtCurrentIndexLater()
        }
    }

    DelayedPropertyBinder {
        id: contentViewIsBeingAltered
        initial: false
        set: contentView.moving || verticalScrollBar.isBeingUsed || contentView.FocusTracker.hasFocus
        delay: 250
    }

    Timer {
        id: forceContentViewPosition
        running: false
        interval: 250
        repeat: false
        onTriggered: screenplayAdapterConnections.positionViewAtCurrentIndex()
    }

    Connections {
        target: Scrite.document.screenplay
        enabled: screenplayAdapter.isSourceScreenplay
        function onRequestEditorAt(index) {
            contentView.positionViewAtIndex(index, ListView.Beginning)
        }
    }

    // Ctrl+Shift+N should result in the newly added scene to get keyboard focus
    Connections {
        target: screenplayAdapter.isSourceScreenplay ? Scrite.document : null
        ignoreUnknownSignals: true
        function onNewSceneCreated(scene, screenplayIndex) {
            Utils.execLater(screenplayAdapter.screenplay, 100, function() {
                contentView.positionViewAtIndex(screenplayIndex, ListView.Visible)
                var item = contentView.loadedItemAtIndex(screenplayIndex)
                if(mainTabBar.currentIndex === 0 || mainUndoStack.screenplayEditorActive)
                    item.assumeFocus()
            })
        }
        function onLoadingChanged() { zoomSlider.reset() }
    }

    Rectangle {
        id: toolbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 1
        color: primaryColors.c100.background
        width: ruler.width
        height: screenplaySearchBar.height * opacity
        enabled: screenplayAdapter.screenplay
        border.width: 1
        border.color: primaryColors.borderColor
        visible: opacity > 0
        opacity: globalScreenplayEditorToolbar.showFind ? 1 : 0
        Behavior on opacity {
            enabled: applicationSettings.enableAnimations
            NumberAnimation { duration: 100 }
        }

        onVisibleChanged: {
            if(visible)
                screenplaySearchBar.assumeFocus()
        }

        SearchBar {
            id: screenplaySearchBar
            searchEngine.objectName: "Screenplay Search Engine"
            anchors.horizontalCenter: parent.horizontalCenter
            allowReplace: !Scrite.document.readOnly
            showReplace: globalScreenplayEditorToolbar.showReplace
            width: toolbar.width * 0.6
            onShowReplaceRequest: globalScreenplayEditorToolbar.showReplace = flag

            Repeater {
                id: searchAgents
                model: screenplayAdapter.screenplay ? 1 : 0

                Item {
                    property string searchString
                    property var searchResults: []
                    property int previousSceneIndex: -1

                    signal replaceCurrentRequest(string replacementText)

                    SearchAgent.onReplaceAll: {
                        screenplayTextDocument.syncEnabled = false
                        screenplayAdapter.screenplay.replace(searchString, replacementText, 0)
                        screenplayTextDocument.syncEnabled = true
                    }
                    SearchAgent.onReplaceCurrent: replaceCurrentRequest(replacementText)

                    SearchAgent.engine: screenplaySearchBar.searchEngine

                    SearchAgent.onSearchRequest: {
                        searchString = string
                        searchResults = screenplayAdapter.screenplay.search(string, 0)
                        SearchAgent.searchResultCount = searchResults.length
                    }

                    SearchAgent.onCurrentSearchResultIndexChanged: {
                        if(SearchAgent.currentSearchResultIndex >= 0) {
                            var searchResult = searchResults[SearchAgent.currentSearchResultIndex]
                            var sceneIndex = searchResult["sceneIndex"]
                            if(sceneIndex !== previousSceneIndex)
                                clearPreviousElementUserData()
                            var sceneResultIndex = searchResult["sceneResultIndex"]
                            var screenplayElement = screenplayAdapter.screenplay.elementAt(sceneIndex)
                            var data = {
                                "searchString": searchString,
                                "sceneResultIndex": sceneResultIndex,
                                "currentSearchResultIndex": SearchAgent.currentSearchResultIndex,
                                "searchResultCount": SearchAgent.searchResultCount
                            }
                            contentView.positionViewAtIndex(sceneIndex, ListView.Visible)
                            screenplayElement.userData = data
                            previousSceneIndex = sceneIndex
                        }
                    }

                    SearchAgent.onClearSearchRequest: {
                        screenplayAdapter.screenplay.currentElementIndex = previousSceneIndex
                        searchString = ""
                        searchResults = []
                        clearPreviousElementUserData()
                    }

                    function clearPreviousElementUserData() {
                        if(previousSceneIndex >= 0) {
                            var screenplayElement = screenplayAdapter.screenplay.elementAt(previousSceneIndex)
                            if(screenplayElement)
                                screenplayElement.userData = undefined
                        }
                        previousSceneIndex = -1
                    }
                }
            }
        }
    }

    Item {
        id: screenplayEditorWorkspace
        anchors.top: toolbar.visible ? toolbar.bottom : parent.top
        anchors.left: sidePanels.right
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        clip: true

        EventFilter.events: [EventFilter.Wheel]
        EventFilter.onFilter: (object,event,result) => {
                                  EventFilter.forwardEventTo(contentView)
                                  result.filter = true
                                  result.accepted = true
                              }

        Item {
            id: pageRulerArea
            width: pageLayout.paperWidth * screenplayEditor.zoomLevel * Screen.devicePixelRatio
            height: parent.height
            anchors.left: parent.left
            anchors.leftMargin: leftMargin
            readonly property real minLeftMargin: 80
            property real leftMargin: {
                const availableMargin = (parent.width-width-20)/2 // 20 is width of scrollbar
                const commentsWidth = Math.min(contentView.spaceForComments, 400)
                if(contentView.commentsExpanded && sidePanels.expanded)
                    return Math.max(minLeftMargin, (parent.width-(commentsWidth+width))/2)
                if(contentView.commentsExpanded && !sidePanels.expanded) {
                    if(availableMargin > commentsWidth)
                        return availableMargin
                    return 2*availableMargin - commentsWidth
                }
                return availableMargin
            }
            Behavior on leftMargin {
                enabled: applicationSettings.enableAnimations && contentView.commentsExpandCounter > 0
                NumberAnimation { duration: 50 }
            }

            Rectangle {
                id: contentArea
                anchors.top: ruler.visible ? ruler.bottom : parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.topMargin: ruler.visible ? 5 : 1
                color: screenplayAdapter.elementCount === 0 || contentView.spacing === 0 ? "white" : Qt.rgba(0,0,0,0)

                TrackerPack {
                    id: trackerPack
                    property int counter: 0
                    TrackProperty { target: screenplayEditorSettings; property: "displaySceneCharacters" }
                    // We shouldnt be tracking changes in elementCount as a reason to reset
                    // the model used by contentView. This causes too many delegate creation/deletions.
                    // Just not effective.
                    // TrackProperty { target: screenplayAdapter; property: "elementCount" }
                    TrackProperty { target: screenplayAdapter; property: "source" }
                    onTracked: counter = counter+1
                }

                ResetOnChange {
                    id: contentViewModel
                    trackChangesOn: trackerPack.counter
                    from: null
                    to: screenplayAdapter
                    onJustReset: {
                        if(screenplayAdapter.currentIndex < 0)
                            contentView.positionViewAtBeginning()
                        else
                            contentView.positionViewAtIndex(screenplayAdapter.currentIndex, ListView.Beginning)
                    }
                }

                Timer {
                    id: postSplitElementTimer
                    property int newCurrentIndex: -1
                    running: false
                    repeat: false
                    interval: 250
                    onTriggered: {
                        if(newCurrentIndex < 0)
                            return
                        contentView.positionViewAtIndex(newCurrentIndex, ListView.Center)
                        Utils.execLater(postSplitElementTimer, 250, function() {
                            var item = contentView.itemAtIndex(postSplitElementTimer.newCurrentIndex)
                            if(item)
                                item.item.assumeFocus()
                            postSplitElementTimer.newCurrentIndex = -1
                        })
                    }
                }

                Component {
                    id: contentViewHeaderComponent

                    Item {
                        id: contentViewHeaderItem
                        width: contentView.width
                        height: {
                            if(!screenplayAdapter.isSourceScreenplay)
                                return contentView.spacing
                            var ret = logLineEditor.visible ? logLineEditor.contentHeight : 0;
                            if(screenplayAdapter.isSourceScreenplay)
                                ret += titleCardLoader.active ? titleCardLoader.height : Math.max(ruler.topMarginPx,editTitlePageButton.height+20)
                            return ret + contentView.spacing
                        }
                        property real padding: width * 0.1

                        Rectangle {
                            anchors.fill: parent
                            anchors.bottomMargin: contentView.spacing
                            visible: screenplayAdapter.elementCount > 0 && contentView.spacing > 0 && screenplayAdapter.isSourceScreenplay
                        }

                        function editTitlePage(source) {
                            modalDialog.arguments = {"activeTabIndex": 2}
                            modalDialog.popupSource = source
                            modalDialog.sourceComponent = optionsDialogComponent
                            modalDialog.active = true
                        }

                        Connections {
                            target: titleCardLoader.item

                            function onEditTitlePageRequest(sourceItem) {
                                contentViewHeaderItem.editTitlePage(sourceItem)
                            }
                        }

                        Loader {
                            id: titleCardLoader
                            active: screenplayAdapter.isSourceScreenplay && (Scrite.document.screenplay.hasTitlePageAttributes || logLineEditor.visible || Scrite.document.screenplay.coverPagePhoto !== "")
                            sourceComponent: titleCardComponent
                            anchors.left: parent.left
                            anchors.right: parent.right

                            ToolButton3 {
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.rightMargin: ruler.rightMarginPx
                                iconSource: "../icons/action/edit_title_page.png"
                                onClicked: editTitlePage(this)
                                visible: parent.active && enabled
                                enabled: !Scrite.document.readOnly
                            }
                        }

                        ToolButton2 {
                            id: editTitlePageButton
                            text: "Edit Title Page"
                            icon.source: "../icons/action/edit_title_page.png"
                            flat: false
                            width: implicitWidth * 1.5
                            height: implicitHeight * 1.25
                            visible: screenplayAdapter.isSourceScreenplay && titleCardLoader.active === false && enabled
                            opacity: hovered ? 1 : 0.75
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: screenplayAdapter.elementCount > 0 ? -contentView.spacing/2 : 0
                            onClicked: editTitlePage(this)
                            enabled: !Scrite.document.readOnly
                        }

                        Item {
                            id: logLineEditor
                            anchors.top: titleCardLoader.active ? titleCardLoader.bottom : editTitlePageButton.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.leftMargin: ruler.leftMarginPx
                            anchors.rightMargin: ruler.rightMarginPx
                            anchors.topMargin: Math.max(ruler.topMarginPx * 0.1, 10)
                            anchors.bottomMargin: Math.max(ruler.topMarginPx * 0.1, 10)
                            property real contentHeight: visible ? logLineEditorLayout.height + anchors.topMargin + anchors.bottomMargin : 0
                            height: logLineEditorLayout.height
                            visible: screenplayEditorSettings.showLoglineEditor && screenplayAdapter.isSourceScreenplay && (Scrite.document.readOnly ? logLineField.text !== "" : true)

                            TextLimiterSyntaxHighlighterDelegate {
                                id: loglineLimitHighlighter
                                textLimiter: TextLimiter {
                                    id: loglineLimiter
                                    maxWordCount: 50
                                    maxLetterCount: 240
                                    countMode: TextLimiter.CountInText
                                }
                            }

                            Column {
                                id: logLineEditorLayout
                                width: parent.width
                                anchors.centerIn: parent
                                spacing: 13

                                Text {
                                    id: logLineFieldHeading
                                    text: logLineField.activeFocus ? ("Logline: (" + (loglineLimiter.limitReached ? "WARNING: " : "") + loglineLimiter.wordCount + "/" + loglineLimiter.maxWordCount + " words, " +
                                          loglineLimiter.letterCount + "/" + loglineLimiter.maxLetterCount + " letters)") : "Logline: "
                                    font.family: screenplayFormat.defaultFont2.family
                                    font.pointSize: screenplayFormat.defaultFont2.pointSize-2
                                    visible: logLineField.length > 0
                                    color: loglineLimiter.limitReached ? "darkred" : primaryColors.a700.background
                                }

                                TextAreaInput {
                                    id: logLineField
                                    width: parent.width
                                    font: screenplayFormat.defaultFont2
                                    readOnly: Scrite.document.readOnly
                                    text: Scrite.document.screenplay.logline
                                    onTextChanged: Scrite.document.screenplay.logline = text
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    placeholderText: "Type your logline here, max " + loglineLimiter.maxWordCount + " words or " + loglineLimiter.maxLetterCount + " letters."
                                    Component.onCompleted: Transliterator.highlighter.addDelegate(loglineLimitHighlighter)
                                }
                            }
                        }
                    }
                }

                Component {
                    id: contentViewDummyHeaderComponent

                    Item {
                        width: contentView.width
                        height: 10 * zoomSlider.zoomLevel
                    }
                }

                Component {
                    id: contentViewFooterComponent

                    Item {
                        width: contentView.width
                        z: 10 // So that the UiElementHightlight doesnt get clipped at the top-edge of the footer.
                        height: {
                            if(!screenplayAdapter.isSourceScreenplay)
                                return contentView.spacing
                            return Math.max(ruler.bottomMarginPx, addEpisodeButton.height+20) + contentView.spacing
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.topMargin: contentView.spacing
                            visible: screenplayAdapter.elementCount > 0 && contentView.spacing > 0 && screenplayAdapter.isSourceScreenplay
                        }

                        Row {
                            id: addButtonsRow
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: screenplayAdapter.elementCount > 0 ? contentView.spacing/2 : 0
                            visible: screenplayAdapter.isSourceScreenplay && enabled
                            enabled: !Scrite.document.readOnly
                            spacing: 20

                            ToolButton3 {
                                id: addSceneButton
                                iconSource: "../icons/action/add_scene.png"
                                shortcutText: "Ctrl+Shift+N"
                                ToolTip.delay: 0
                                text: "Add Scene"
                                suggestedWidth: 48
                                suggestedHeight: 48
                                onClicked: {
                                    Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.lastSceneIndex()
                                    if(!Scrite.document.readOnly)
                                        Scrite.document.createNewScene(true)
                                }
                            }

                            ToolButton3 {
                                id: addActBreakButton
                                iconSource: "../icons/action/add_act.png"
                                shortcutText: "Ctrl+Shift+B"
                                ToolTip.delay: 0
                                text: "Add Act Break"
                                suggestedWidth: 48
                                suggestedHeight: 48
                                onClicked: Scrite.document.screenplay.addBreakElement(Screenplay.Act)
                            }

                            ToolButton3 {
                                id: addEpisodeButton
                                iconSource: "../icons/action/add_episode.png"
                                shortcutText: "Ctrl+Shift+P"
                                ToolTip.delay: 0
                                text: "Add Episode Break"
                                suggestedWidth: 48
                                suggestedHeight: 48
                                onClicked: Scrite.document.screenplay.addBreakElement(Screenplay.Episode)
                            }
                        }

                        Loader {
                            id: addButtonsAnimator
                            active: mainTabBar.currentIndex === 0 && contentView.count === 1 && !modalDialog.active && !splashLoader.active && !instanceSettings.screenplayEditorAddButtonsAnimationShown
                            anchors.fill: parent
                            sourceComponent: UiElementHighlight {
                                uiElement: addButtonsRow
                                uiElementBoxVisible: true
                                descriptionPosition: Item.Bottom
                                description: "Use these buttons to add new a scene, act or episode."
                                highlightAnimationEnabled: false
                                onDone: addButtonsAnimator.active = false
                                Component.onCompleted: instanceSettings.screenplayEditorAddButtonsAnimationShown = true
                            }
                        }
                    }
                }

                Component {
                    id: contentViewDummyFooterComponent

                    Item {
                        width: contentView.width
                        height: contentView.height * 0.25 * zoomSlider.zoomLevel
                    }
                }

                ListView {
                    id: contentView
                    anchors.fill: parent
                    model: defaultCacheBuffer >= 0 ? contentViewModel.value : null
                    spacing: screenplayAdapter.elementCount > 0 ? screenplayEditorSettings.spaceBetweenScenes*zoomLevel : 0
                    property int commentsExpandCounter: 0
                    property bool commentsExpanded: false
                    property bool scrollingBetweenScenes: false
                    readonly property bool loadAllDelegates: false // for future use
                    property real spaceForComments: {
                        if(screenplayEditorSettings.displaySceneComments && commentsPanelAllowed)
                            return Math.round(screenplayEditorWorkspace.width - pageRulerArea.width - pageRulerArea.minLeftMargin - 20)
                        return 0
                    }
                    property int commentsPanelTabIndex: screenplayEditorSettings.commentsPanelTabIndex
                    onCommentsPanelTabIndexChanged: screenplayEditorSettings.commentsPanelTabIndex = commentsPanelTabIndex
                    onCommentsExpandedChanged: commentsExpandCounter = commentsExpandCounter+1
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

                    function delegateWasLoaded() { Qt.callLater(delegateLoaded) }
                    signal delegateLoaded()

                    property bool allowContentYAnimation
                    Behavior on contentY {
                        enabled: applicationSettings.enableAnimations && contentView.allowContentYAnimation
                        NumberAnimation {
                            duration: 100
                            onFinished: contentView.allowContentYAnimation = false
                        }
                    }

                    header: {
                        if(screenplayEditorSettings.displayEmptyTitleCard)
                            return contentViewHeaderComponent
                        if(screenplayAdapter.isSourceScreenplay) {
                            const logLineEditorVisible = screenplayEditorSettings.showLoglineEditor && (Scrite.document.readOnly ? screenplayAdapter.screenplay.logline !== "" : true)
                            if (screenplayAdapter.screenplay.hasTitlePageAttributes || logLineEditorVisible)
                                return contentViewHeaderComponent
                        }
                        return contentViewDummyHeaderComponent
                    }

                    footer: screenplayEditorSettings.displayAddSceneBreakButtons ? contentViewFooterComponent : contentViewDummyFooterComponent

                    delegate: contentViewDelegateComponent

                    snapMode: ListView.NoSnap
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds
                    keyNavigationEnabled: false
                    ScrollBar.vertical: verticalScrollBar
                    property int numberOfWordsAddedToDict : 0

                    FocusTracker.window: Scrite.window
                    FocusTracker.indicator.target: mainUndoStack
                    FocusTracker.indicator.property: screenplayAdapter.isSourceScreenplay ? "screenplayEditorActive" : "sceneEditorActive"

                    property int defaultCacheBuffer: -1
                    function configureCacheBuffer() {
                        defaultCacheBuffer = cacheBuffer
                        cacheBuffer = Qt.binding( () => {
                                                     if(!model)
                                                        return defaultCacheBuffer
                                                     return (screenplayEditorSettings.optimiseScrolling || contentView.loadAllDelegates) ? 2147483647 : defaultCacheBuffer
                                                 })
                    }

                    Component.onCompleted: {
                        if(Scrite.app.isMacOSPlatform)
                            flickDeceleration = 7500
                        positionViewAtIndex(screenplayAdapter.currentIndex, ListView.Beginning)
                        configureCacheBuffer()
                    }

                    property point firstPoint: mapToItem(contentItem, width/2, 1)
                    property point lastPoint: mapToItem(contentItem, width/2, height-2)
                    property int firstItemIndex: screenplayAdapter.elementCount > 0 ? Math.max(indexAt(firstPoint.x, firstPoint.y), 0) : 0
                    property int lastItemIndex: screenplayAdapter.elementCount > 0 ? validOrLastIndex(indexAt(lastPoint.x, lastPoint.y)) : 0

                    onContentYChanged: Qt.callLater(evaluateFirstAndLastPoint)
                    onOriginYChanged: Qt.callLater(evaluateFirstAndLastPoint)
                    onFirstItemIndexChanged: makeAVisibleItemCurrentTimer.start()
                    onLastItemIndexChanged: makeAVisibleItemCurrentTimer.start()
                    onMovingChanged: makeAVisibleItemCurrentTimer.start()

                    function evaluateFirstAndLastPoint() {
                        firstPoint = mapToItem(contentItem, width/2, 1)
                        lastPoint = mapToItem(contentItem, width/2, height-2)
                    }

                    Timer {
                        id: makeAVisibleItemCurrentTimer
                        interval: 600
                        repeat: false
                        running: false
                        onTriggered: {
                            if(contentView.moving) {
                                start()
                                return
                            }
                            contentView.makeItemUnderCursorCurrent()
                        }
                    }

                    function makeItemUnderCursorCurrent() {
                        // If the current item is already visible, then lets not
                        // second guess user's intent. We will leave the current
                        // item as is.
                        var ci = screenplayAdapter.currentIndex
                        if(ci >= firstItemIndex && ci <= lastItemIndex)
                            return

                        // Lets first confirm that the mouse pointer is within
                        // the contentView area.
                        var gp = Scrite.app.cursorPosition()
                        var pos = Scrite.app.mapGlobalPositionToItem(contentView,gp)
                        if(pos.x >= 0 && pos.x < contentView.width && pos.y >= 0 && pos.y < contentView.height) {
                            // Find out the item under mouse and make it current.
                            pos = mapToItem(contentItem, pos.x, pos.y)
                            ci = indexAt(pos.x, pos.y)
                            if(ci >= 0 && ci <= screenplayAdapter.elementCount-1)
                                privateData.changeCurrentIndexTo(ci)
                        }
                    }

                    function validOrLastIndex(val) { return val < 0 ? screenplayAdapter.elementCount-1 : val }

                    function isVisible(index) {
                        return index >= firstItemIndex && index <= lastItemIndex
                    }

                    function scrollToFirstScene() {
                        positionViewAtBeginning()
                    }

                    function scrollIntoView(index) {
                        if(index < 0) {
                            positionViewAtBeginning()
                            return
                        }

                        var topIndex = firstItemIndex
                        var bottomIndex = lastItemIndex

                        if(index >= topIndex && index <= bottomIndex)
                            return // item is already visible

                        if(index < topIndex && topIndex-index <= 2) {
                            contentView.contentY -= height*0.2
                        } else if(index > bottomIndex && index-bottomIndex <= 2) {
                            contentView.contentY += height*0.2
                        } else {
                            positionViewAtIndex(index, ListView.Beginning)
                        }
                    }

                    function ensureVisible(item, rect) {
                        if(item === null)
                            return

                        var pt = item.mapToItem(contentView.contentItem, rect.x, rect.y)
                        var startY = contentView.contentY
                        var endY = contentView.contentY + contentView.height - rect.height
                        if( pt.y >= startY && pt.y <= endY )
                            return

                        if( pt.y < startY )
                            contentView.contentY = Math.round(pt.y)
                        else
                            contentView.contentY = Math.round((pt.y + 2*rect.height) - contentView.height)
                    }

                    function loadedItemAtIndex(index) {
                        var loader = contentView.itemAtIndex(index)
                        if(loader.item === null)
                            loader.load()
                        return loader.item
                    }
                }
            }

            RulerItem {
                id: ruler
                width: parent.width
                height: 20
                font.pixelSize: 10
                leftMargin: pageLayout.leftMargin * Screen.devicePixelRatio
                rightMargin: pageLayout.rightMargin * Screen.devicePixelRatio
                zoomLevel: screenplayEditor.zoomLevel
                resolution: Scrite.document.displayFormat.pageLayout.resolution
                visible: screenplayEditorSettings.displayRuler

                property real leftMarginPx: leftMargin * zoomLevel
                property real rightMarginPx: rightMargin * zoomLevel
                property real topMarginPx: pageLayout.topMargin * Screen.devicePixelRatio * zoomLevel
                property real bottomMarginPx: pageLayout.bottomMargin * Screen.devicePixelRatio * zoomLevel
            }
        }

        BusyIcon {
            anchors.centerIn: parent
            running: Scrite.document.loading || !screenplayTextDocument.paused && screenplayTextDocument.updating
            visible: running
        }
    }

    ScrollBar {
        id: verticalScrollBar
        anchors.top: screenplayEditorWorkspace.top
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        orientation: Qt.Vertical
        minimumSize: 0.1
        policy: screenplayAdapter.elementCount > 0 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        property bool isBeingUsed: false
        onPressedChanged: {
            if(pressed)
                isBeingUsed = true
            else
                Utils.execLater(verticalScrollBar, 250, () => {
                                    privateData.changeCurrentIndexTo(contentView.lastItemIndex)
                                    isBeingUsed = false
                                })
        }
    }

    Rectangle {
        id: statusBar
        height: 30
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: primaryColors.windowColor
        border.width: 1
        border.color: primaryColors.borderColor
        clip: true

        Item {
            anchors.fill: metricsDisplay

            ToolTip.text: "Page count and time estimates are approximate, assuming " + screenplayTextDocument.timePerPageAsString + " per page."
            ToolTip.delay: 1000
            ToolTip.visible: metricsDisplayOverlayMouseArea.containsMouse

            MouseArea {
                id: metricsDisplayOverlayMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Row {
            id: metricsDisplay
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            spacing: 10

            Image {
                id: toggleLockButton
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                enabled: !Scrite.document.readOnly
                source: {
                    if(Scrite.document.readOnly)
                        return "../icons/action/lock_outline.png"
                    if(Scrite.user.loggedIn)
                        return Scrite.document.hasCollaborators ? "../icons/file/protected.png" : "../icons/file/unprotected.png"
                    return Scrite.document.locked ? "../icons/action/lock_outline.png" : "../icons/action/lock_open.png"
                }
                scale: toggleLockMouseArea.containsMouse ? (toggleLockMouseArea.pressed ? 1 : 1.5) : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: toggleLockMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    ToolTip.text: {
                        if(Scrite.document.readOnly)
                            return "Cannot lock/unlock for editing on this computer."
                        if(Scrite.user.loggedIn)
                            return Scrite.document.hasCollaborators ? "Add/Remove collaborators who can view & edit this document." : "Protect this document so that you and select collaborators can view/edit it."
                        return Scrite.document.locked ? "Unlock to allow editing on this and other computers." : "Lock to allow editing of this document only on this computer."
                    }
                    ToolTip.visible: containsMouse
                    ToolTip.delay: 1000

                    onClicked: {
                        if(Scrite.user.loggedIn)
                            editCollaborators()
                        else
                            toggleLock()
                    }

                    Component {
                        id: collaboratorsDialog
                        CollaboratorsDialog { }
                    }

                    function editCollaborators() {
                        modalDialog.sourceComponent = collaboratorsDialog
                        modalDialog.popupSource = parent
                        modalDialog.active = true
                    }

                    function toggleLock() {
                        var locked = !Scrite.document.locked
                        Scrite.document.locked = locked

                        var message = ""
                        if(locked)
                            message = "Document LOCKED. You will be able to edit it only on this computer."
                        else
                            message = "Document unlocked. You will be able to edit it on this and any other computer."

                        showInformation({"message": message}, this)
                    }
                }
            }

            Image {
                source: "../icons/navigation/refresh.png"
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: screenplayTextDocument.paused ? 0.85 : 1
                scale: refreshMouseArea.containsMouse ? (refreshMouseArea.pressed ? 1 : 1.5) : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: refreshMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if(screenplayTextDocument.paused)
                            screenplayTextDocument.paused = false
                        else
                            screenplayTextDocument.reload()
                    }
                    ToolTip.visible: containsMouse && !pressed
                    ToolTip.text: enabled ? "Computes page layout from scratch, thereby reevaluating page count and time." : "Resume page and time computation."
                    ToolTip.delay: 1000
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
            }

            Image {
                source: "../icons/content/page_count.png"
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: screenplayTextDocument.paused ? 0.85 : 1
                scale: pageCountMouseAra.containsMouse ? (pageCountMouseAra.pressed ? 1 : 1.5) : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: pageCountMouseAra
                    anchors.fill: parent
                    onClicked: screenplayTextDocument.paused = !screenplayTextDocument.paused
                    hoverEnabled: true
                    ToolTip.visible: containsMouse && !pressed
                    ToolTip.text: "Click here to toggle page computation, in case the app is not responding fast while typing."
                    ToolTip.delay: 1000
                }
            }

            Text {
                font.pixelSize: statusBar.height * 0.5
                text: screenplayTextDocument.paused ? "- of -" : (screenplayTextDocument.currentPage + " of " + screenplayTextDocument.pageCount)
                anchors.verticalCenter: parent.verticalCenter
                opacity: screenplayTextDocument.paused ? 0.5 : 1
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
            }

            Image {
                source: "../icons/content/time.png"
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: screenplayTextDocument.paused ? 0.85 : 1
                scale: timeMouseArea.containsMouse ? (timeMouseArea.pressed ? 1 : 1.5) : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: timeMouseArea
                    anchors.fill: parent
                    onClicked: screenplayTextDocument.paused = !screenplayTextDocument.paused
                    hoverEnabled: true
                    ToolTip.visible: containsMouse && !pressed
                    ToolTip.text: "Click here to toggle time computation, in case the app is not responding fast while typing."
                    ToolTip.delay: 1000
                }
            }

            Text {
                font.pixelSize: statusBar.height * 0.5
                text: screenplayTextDocument.paused ? "- of -" : (screenplayTextDocument.currentTimeAsString + " of " + (screenplayTextDocument.pageCount > 1 ? screenplayTextDocument.totalTimeAsString : screenplayTextDocument.timePerPageAsString))
                anchors.verticalCenter: parent.verticalCenter
                opacity: screenplayTextDocument.paused ? 0.5 : 1
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
                visible: wordCountLabel.visible
            }

            Text {
                id: wordCountLabel
                font.pixelSize: statusBar.height * 0.5
                text: {
                    const currentScene = screenplayAdapter.currentScene
                    const currentSceneWordCount = currentScene ? currentScene.wordCount + " / " : ""
                    const totalWordCount = screenplayAdapter.wordCount + (screenplayAdapter.wordCount !== 1 ? " words" : " word")
                    return currentSceneWordCount + totalWordCount
                }
                anchors.verticalCenter: parent.verticalCenter
                visible: taggingOptionsPosMapper.mappedPosition.x > width

                ItemPositionMapper {
                    id: taggingOptionsPosMapper
                    from: taggingOptions
                    position: Qt.point(0,0)
                    to: wordCountLabel
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Displays 'current scene word count' / 'whole screenplay word count'."
                    ToolTip.delay: 1000
                }
            }
        }

        Item {
            id: headingTextAreaOnStatusBar
            anchors.left: metricsDisplay.right
            anchors.right: taggingOptions.visible ? taggingOptions.left : zoomSlider.left
            anchors.margins: 5
            clip: true
            height: parent.height

            ItemPositionMapper {
                id: contentViewPositionMapper
                from: contentView
                position: Qt.point(0,0)
                to: headingTextAreaOnStatusBar
            }

            Item {
                x: contentViewPositionMapper.mappedPosition.x
                width: contentView.width
                height: parent.height
                visible: x > 0

                property ScreenplayElement currentSceneElement: {
                    if(screenplayAdapter.isSourceScene || screenplayAdapter.elementCount === 0)
                        return null

                    var element = null
                    if(contentView.isVisible(screenplayAdapter.currentIndex)) {
                        element = screenplayAdapter.currentElement
                    } else {
                        var data = screenplayAdapter.at(contentView.firstItemIndex)
                        element = data ? data.screenplayElement : null
                    }

                    return element
                }
                property Scene currentScene: currentSceneElement ? currentSceneElement.scene : null
                property SceneHeading currentSceneHeading: currentScene && currentScene.heading.enabled ? currentScene.heading : null

                Text {
                    id: currentSceneNumber
                    anchors.verticalCenter: currentSceneHeadingText.verticalCenter
                    anchors.left: currentSceneHeadingText.left
                    anchors.leftMargin: Math.min(-recommendedMargin, -contentWidth)
                    font: currentSceneHeadingText.font
                    text: parent.currentSceneHeading ? parent.currentSceneElement.resolvedSceneNumber + ". " : ''
                    property real recommendedMargin: headingFontMetrics.averageCharacterWidth*5 + ruler.leftMarginPx*0.075
                }

                Text {
                    id: currentSceneHeadingText
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: ruler.leftMarginPx
                    anchors.rightMargin: ruler.rightMarginPx
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: height*0.1
                    font.family: headingFontMetrics.font.family
                    font.pixelSize: parent.height * 0.6
                    elide: Text.ElideRight
                    text: parent.currentSceneHeading ? parent.currentSceneHeading.text : ''
                }
            }
        }

        Row {
            id: taggingOptions
            spacing: 10
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: zoomSlider.left
            anchors.rightMargin: spacing
            height: zoomSlider.height

            ToolButton3 {
                iconSource: "../icons/action/layout_grouping.png"
                height: parent.height; width: height
                anchors.verticalCenter: parent.verticalCenter
                down: taggingMenu.active
                onClicked: taggingMenu.show()
                ToolTip.text: "Grouping Options"
                visible: screenplayEditorSettings.allowTaggingOfScenes && mainTabBar.currentIndex === 0

                MenuLoader {
                    id: taggingMenu
                    anchors.left: parent.left
                    anchors.bottom: parent.top
                    menu: Menu2 {
                        id: layoutGroupingMenu
                        width: 350

                        MenuItem2 {
                            text: "None"
                            icon.source: font.bold ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                            font.bold: Scrite.document.structure.preferredGroupCategory === ""
                            onTriggered: Scrite.document.structure.preferredGroupCategory = ""
                        }

                        MenuSeparator { }

                        Repeater {
                            model: Scrite.document.structure.groupCategories

                            MenuItem2 {
                                text: Scrite.app.camelCased(modelData)
                                icon.source: font.bold ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                                font.bold: Scrite.document.structure.preferredGroupCategory === modelData
                                onTriggered: Scrite.document.structure.preferredGroupCategory = modelData
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
            }
        }

        ZoomSlider {
            id: zoomSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            property var zoomLevels: screenplayFormat.fontZoomLevels
            zoomLevel: zoomLevels[value]
            from: 0; to: zoomLevels.length-1
            height: parent.height-6
            stepSize: 1
            function reset() {
                var zls = zoomLevels
                for(var i=0; i<zls.length; i++) {
                    if(zls[i] === 1) {
                        value = i
                        return
                    }
                }
            }

            onValueChanged: screenplayFormat.fontZoomLevelIndex = value
            Component.onCompleted: {
                reset()
                value = value + zoomLevelModifier
                screenplayFormat.fontZoomLevelIndex = value
            }

            function zoomLevelModifierToApply() {
                var zls = zoomLevels
                var oneLevel = value
                for(var i=0; i<zls.length; i++) {
                    if(zls[i] === 1) {
                        oneLevel = i
                        break
                    }
                }
                return value - oneLevel
            }

            Connections {
                target: screenplayFormat
                function onFontZoomLevelIndexChanged() {
                    if(!Scrite.document.empty)
                        zoomSlider.value = screenplayFormat.fontZoomLevelIndex
                }
            }

            Connections {
                target: Scrite.app.transliterationEngine
                function onPreferredFontFamilyForLanguageChanged() {
                    const oldValue = zoomSlider.value
                    zoomSlider.value = screenplayFormat.fontZoomLevelIndex
                    Qt.callLater( (val) => { zoomSlider.value = val }, oldValue )
                }
            }

            property int savedZoomValue: -1

            Announcement.onIncoming: (type,data) => {
                                         const stype = "" + type
                                         const sdata = "" + data
                                         if(stype === "DF77A452-FDB2-405C-8A0F-E48982012D36") {
                                             if(sdata === "save") {
                                                 zoomSlider.savedZoomValue = zoomSlider.value
                                                 zoomSlider.reset()
                                             } else if(sdata === "restore") {
                                                 if(zoomSlider.savedZoomValue >= 0)
                                                    zoomSlider.value = zoomSlider.savedZoomValue
                                                 zoomSlider.savedZoomValue = -1
                                             }
                                         }
                                     }
        }
    }

    Component {
        id: episodeBreakComponent

        Rectangle {
            id: episodeBreakItem
            property int theIndex: componentIndex
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            height: episodeBreakSubtitle.height + headingFontMetrics.lineSpacing*0.1
            color: primaryColors.c10.background

            TextField {
                id: episodeBreakTitle
                maximumLength: 5
                width: headingFontMetrics.averageCharacterWidth*maximumLength
                anchors.right: episodeBreakSubtitle.left
                anchors.rightMargin: ruler.leftMarginPx * 0.075
                anchors.bottom: parent.bottom
                horizontalAlignment: Text.AlignRight
                text: "Ep " + (theElement.episodeIndex+1)
                readOnly: true
                visible: episodeBreakSubtitle.length > 0
                font: episodeBreakSubtitle.font
                background: Item { }
            }

            TextField2 {
                id: episodeBreakSubtitle
                label: ""
                anchors.left: parent.left
                anchors.right: deleteBreakButton.left
                anchors.leftMargin: ruler.leftMarginPx
                anchors.rightMargin: 5
                anchors.bottom: parent.bottom
                placeholderText: theElement.breakTitle
                font.family: headingFontMetrics.font.family
                font.bold: true
                font.pointSize: headingFontMetrics.font.pointSize+2
                text: theElement.breakSubtitle
                enableTransliteration: true
                onTextEdited: theElement.breakSubtitle = text
                onEditingComplete: theElement.breakSubtitle = text
            }

            ToolButton3 {
                id: deleteBreakButton
                iconSource: "../icons/action/delete.png"
                width: headingFontMetrics.lineSpacing
                height: headingFontMetrics.lineSpacing
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: ruler.rightMarginPx
                onClicked: screenplayAdapter.screenplay.removeElement(episodeBreakItem.theElement)
                ToolTip.text: "Deletes this episode break."
            }
        }
    }

    Component {
        id: actBreakComponent

        Rectangle {
            id: actBreakItem
            property int theIndex: componentIndex
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            height: actBreakTitle.height
            color: primaryColors.c10.background

            TextField {
                id: actBreakTitle
                maximumLength: 7
                width: headingFontMetrics.averageCharacterWidth*maximumLength
                anchors.right: actBreakSubtitle.left
                anchors.rightMargin: ruler.leftMarginPx * 0.075
                anchors.bottom: parent.bottom
                horizontalAlignment: Text.AlignRight
                text: theElement.breakTitle
                readOnly: true
                visible: actBreakSubtitle.length > 0
                font: actBreakSubtitle.font
                background: Item { }
            }

            TextField2 {
                id: actBreakSubtitle
                label: ""
                anchors.left: parent.left
                anchors.right: deleteBreakButton.left
                anchors.leftMargin: ruler.leftMarginPx
                anchors.rightMargin: 5
                anchors.bottom: parent.bottom
                placeholderText: theElement.breakTitle
                font.family: headingFontMetrics.font.family
                font.bold: true
                font.pointSize: headingFontMetrics.font.pointSize+2
                text: theElement.breakSubtitle
                enableTransliteration: true
                onTextEdited: theElement.breakSubtitle = text
                onEditingComplete: theElement.breakSubtitle = text
            }

            ToolButton3 {
                id: deleteBreakButton
                iconSource: "../icons/action/delete.png"
                width: headingFontMetrics.lineSpacing
                height: headingFontMetrics.lineSpacing
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: ruler.rightMarginPx
                onClicked: screenplayAdapter.screenplay.removeElement(actBreakItem.theElement)
                ToolTip.text: "Deletes this act break."
            }
        }
    }

    Component {
        id: placeholderSceneComponent

        Rectangle {
            required property int spElementIndex
            required property var spElementData
            required property int spElementType

            property bool evaluateSuggestedSceneHeight: true

            border.width: 1
            border.color: screenplayElement.scene ? screenplayElement.scene.color : primaryColors.c400.background
            color: screenplayElement.scene ? Qt.tint(screenplayElement.scene.color, "#E7FFFFFF") : primaryColors.c300.background

            readonly property ScreenplayElement screenplayElement: spElementData.screenplayElement
            readonly property Scene scene: spElementData.scene
            readonly property int screenplayElementType: spElementData.screenplayElementType
            property real suggestedSceneHeight: screenplayElement.omitted ? (sceneHeadingText.height + sceneHeadingText.anchors.topMargin*2) : sizeHint.height

            SceneSizeHintItem {
                id: sizeHint
                visible: false
                asynchronous: false
                width: contentWidth * zoomLevel
                height: contentHeight * zoomLevel
                format: Scrite.document.printFormat
                scene: parent.scene
                active: parent.evaluateSuggestedSceneHeight && !parent.screenplayElement.omitted
            }

            Text {
                font: sceneHeadingText.font
                anchors.verticalCenter: sceneHeadingText.verticalCenter
                anchors.right: sceneHeadingText.left
                anchors.rightMargin: 20
                width: headingFontMetrics.averageCharacterWidth*5
                color: screenplayElement.hasUserSceneNumber ? "black" : "gray"
                text: screenplayElement.resolvedSceneNumber
            }

            Text {
                id: sceneHeadingText
                anchors.left: parent.left
                anchors.leftMargin: ruler.leftMarginPx
                anchors.right: parent.right
                anchors.rightMargin: ruler.rightMarginPx
                anchors.top: parent.top
                anchors.topMargin: 20

                width: parent.width - 20
                property SceneElementFormat headingFormat: screenplayFormat.elementFormat(SceneElement.Heading)
                font: headingFormat.font2

                color: screenplayElementType === ScreenplayElement.BreakElementType ? "gray" : "black"
                elide: Text.ElideMiddle
                text: {
                    if(scene && scene.heading.enabled)
                        return screenplayElement.omitted ? "[OMITTED]" : scene.heading.text
                    if(screenplayElementType === ScreenplayElement.BreakElementType)
                        return screenplayElement.breakTitle
                    return "NO SCENE HEADING"
                }
            }

            Image {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.top: sceneHeadingText.bottom
                anchors.topMargin: 20 * zoomLevel
                anchors.bottomMargin: 20 * zoomLevel
                fillMode: Image.TileVertically
                source: "../images/sample_scene.png"
                opacity: 0.5
                visible: !parent.screenplayElement.omitted
            }
        }
    }

    Component {
        id: noContentComponent

        Item {
            width: contentArea.width
            height: 0
        }
    }

    Component {
        id: omittedContentComponent

        Rectangle {
            id: omittedContentItem
            property int theIndex: componentIndex
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            property bool isCurrent: theElement === screenplayAdapter.currentElement
            z: isCurrent ? 2 : 1

            width: contentArea.width
            height: omittedContentItemLayout.height
            color: Scrite.app.isVeryLightColor(theScene.color) ? primaryColors.highlight.background : Qt.tint(theScene.color, "#9CFFFFFF")

            Column {
                id: omittedContentItemLayout
                width: parent.width

                Loader {
                    id: omittedSceneHeadingAreaLoader
                    width: parent.width
                    active: omittedContentItem.theScene
                    sourceComponent: sceneHeadingArea
                    z: 1
                    onItemChanged: {
                        if(item) {
                            item.theElementIndex = omittedContentItem.theIndex
                            item.theScene = omittedContentItem.theScene
                            item.theElement = omittedContentItem.theElement
                        }
                    }
                }

                // For future expansion
            }
        }
    }

    Component {
        id: contentComponent

        Rectangle {
            id: contentItem
            property int theIndex: componentIndex
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            property bool isCurrent: theElement === screenplayAdapter.currentElement
            z: isCurrent ? 2 : 1

            width: contentArea.width
            height: contentItemLayout.height
            color: "white"
            readonly property var binder: sceneDocumentBinder
            readonly property var editor: sceneTextEditor
            property bool canSplitScene: sceneTextEditor.activeFocus && !Scrite.document.readOnly && sceneDocumentBinder.currentElement && sceneDocumentBinder.currentElementCursorPosition === 0 && screenplayAdapter.isSourceScreenplay
            property bool canJoinToPreviousScene: sceneTextEditor.activeFocus && !Scrite.document.readOnly && sceneTextEditor.cursorPosition === 0 && contentItem.theIndex > 0

            FocusTracker.window: Scrite.window
            FocusTracker.onHasFocusChanged: {
                contentItem.theScene.undoRedoEnabled = FocusTracker.hasFocus
                sceneHeadingAreaLoader.item.sceneHasFocus = FocusTracker.hasFocus
            }

            SceneDocumentBinder {
                id: sceneDocumentBinder
                scene: contentItem.theScene
                textDocument: sceneTextEditor.textDocument
                applyTextFormat: true
                cursorPosition: sceneTextEditor.activeFocus ? sceneTextEditor.cursorPosition : -1
                selectionEndPosition: sceneTextEditor.activeFocus ? sceneTextEditor.selectionEnd : -1
                selectionStartPosition: sceneTextEditor.activeFocus ? sceneTextEditor.selectionStart : -1
                shots: Scrite.document.structure.shots
                transitions: Scrite.document.structure.transitions
                characterNames: Scrite.document.structure.characterNames
                screenplayFormat: screenplayEditor.screenplayFormat
                screenplayElement: contentItem.theElement
                forceSyncDocument: !sceneTextEditor.activeFocus
                spellCheckEnabled: !Scrite.document.readOnly && spellCheckEnabledFlag.value
                autoCapitalizeSentences: !Scrite.document.readOnly && screenplayEditorSettings.enableAutoCapitalizeSentences
                autoPolishParagraphs: !Scrite.document.readOnly && screenplayEditorSettings.enableAutoPolishParagraphs
                liveSpellCheckEnabled: sceneTextEditor.activeFocus
                property bool firstInitializationDone: false
                onDocumentInitialized: {
                    if(!firstInitializationDone && !contentView.scrollingBetweenScenes)
                        sceneTextEditor.cursorPosition = 0
                    firstInitializationDone = true
                }
                onRequestCursorPosition: (position) => {
                                             if(position >= 0)
                                                contentItem.assumeFocusLater(position, 100)
                                         }


                function changeCase(textCase) {
                    const sstart = sceneTextEditor.selectionStart
                    const send = sceneTextEditor.selectionEnd
                    const cp = sceneTextEditor.cursorPosition
                    changeTextCase(textCase)
                    if(sstart >= 0 && send > 0 && send > sstart)
                        Utils.execLater(sceneTextEditor, 150, () => {
                                            sceneTextEditor.forceActiveFocus()
                                            sceneTextEditor.select(sstart, send)
                                        })
                    else if(cp >= 0)
                        contentItem.assumeFocusLater(cp, 100)

                }

                property var currentParagraphType: currentElement ? currentElement.type : SceneHeading.Action
                applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
                onCurrentParagraphTypeChanged: {
                    if(currentParagraphType === SceneElement.Action) {
                        ruler.paragraphLeftMargin = 0
                        ruler.paragraphRightMargin = 0
                    } else {
                        var elementFormat = screenplayEditor.screenplayFormat.elementFormat(currentParagraphType)
                        ruler.paragraphLeftMargin = ruler.leftMargin + pageLayout.contentWidth * elementFormat.leftMargin * Screen.devicePixelRatio
                        ruler.paragraphRightMargin = ruler.rightMargin + pageLayout.contentWidth * elementFormat.rightMargin * Screen.devicePixelRatio
                    }
                }

                function preserveScrollAndReload() {
                    var cy = contentView.contentY
                    reload()
                    contentView.contentY = cy
                }
            }

            ResetOnChange {
                id: spellCheckEnabledFlag
                trackChangesOn: contentView.numberOfWordsAddedToDict
                from: false
                to: screenplayEditorSettings.enableSpellCheck
                delay: 100
            }

            SidePanel {
                id: commentsSidePanel
                property color theSceneDarkColor: Scrite.app.isLightColor(contentItem.theScene.color) ? primaryColors.c500.background : contentItem.theScene.color
                buttonColor: expanded ? Qt.tint(contentItem.theScene.color, "#C0FFFFFF") : Qt.tint(contentItem.theScene.color, "#D7EEEEEE")
                backgroundColor: buttonColor
                borderColor: expanded ? primaryColors.borderColor : (contentView.spacing > 0 ? Scrite.app.translucent(theSceneDarkColor,0.25) : Qt.rgba(0,0,0,0))
                z: contentItem.isCurrent ? 1 : 0
                borderWidth: contentItem.isCurrent ? 2 : 1
                anchors.top: parent.top
                anchors.left: parent.right

                property real screenY: screenplayEditor.mapFromItem(parent, 0, 0).y
                property real maxTopMargin: contentItem.height-height-20
                anchors.topMargin: screenY < 0 ? Math.min(-screenY,maxTopMargin) : -1

                cornerComponent: expanded ? commentsExpandedSidePanelCornerComponent : commentsCollapsedSidePanelCornerComponent

                Component {
                    id: commentsCollapsedSidePanelCornerComponent

                    Item {
                        Image {
                            property Attachments sceneAttachments: contentItem.theScene.attachments
                            property Attachment sceneFeaturedAttachment: sceneAttachments.featuredAttachment
                            property Attachment sceneFeaturedImage: sceneFeaturedAttachment && sceneFeaturedAttachment.type === Attachment.Photo ? sceneFeaturedAttachment : null
                            property string sceneComments: contentItem.theScene.comments
                            property bool hasSceneComments: sceneComments !== ""
                            width: Math.max((hasSceneComments ? 14 : 12), Math.min(parent.width,parent.height)-(hasSceneComments ? 6 : 10))
                            height: width
                            y: 2
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.horizontalCenterOffset: 1.5
                            opacity: 0.75
                            smooth: true
                            mipmap: true
                            source: {
                                if(hasSceneComments)
                                    return "../icons/content/note.png"

                                if(sceneFeaturedImage)
                                    return "../icons/filetype/photo.png"

                                return ""
                            }
                        }
                    }
                }

                Component {
                    id: commentsExpandedSidePanelCornerComponent

                    Column {
                        spacing: 8

                        ToolButton3 {
                            iconSource: down ? "../icons/content/comments_panel_inverted.png" : "../icons/content/comments_panel.png"
                            suggestedWidth: parent.width
                            suggestedHeight: parent.width
                            down: contentView.commentsPanelTabIndex === 0
                            downIndicatorColor: commentsSidePanel.theSceneDarkColor
                            onClicked: contentView.commentsPanelTabIndex = 0
                            ToolTip.visible: hovered
                            ToolTip.text: "View/edit scene comments."
                        }

                        ToolButton3 {
                            iconSource: down ? "../icons/filetype/photo_inverted.png" : "../icons/filetype/photo.png"
                            suggestedWidth: parent.width
                            suggestedHeight: parent.width
                            down: contentView.commentsPanelTabIndex === 1
                            downIndicatorColor: commentsSidePanel.theSceneDarkColor
                            onClicked: contentView.commentsPanelTabIndex = 1
                            ToolTip.visible: hovered
                            ToolTip.text: "View/edit scene featured image."
                        }
                    }
                }

                Connections {
                    target: contentView
                    function onContentYChanged() {
                        commentsSidePanel.screenY = screenplayEditor.mapFromItem(commentsSidePanel.parent, 0, 0).y
                    }
                }

                // anchors.leftMargin: expanded ? 0 : -minPanelWidth
                label: expanded && anchors.topMargin > 0 ? ("Scene " + contentItem.theElement.resolvedSceneNumber) : ""
                height: {
                    if(expanded) {
                        if(contentItem.isCurrent)
                            return contentInstance ? Math.min(contentItem.height, Math.max(contentInstance.contentHeight+60, 350)) : 300
                        return Math.min(300, parent.height)
                    }
                    return sceneHeadingAreaLoader.height + (synopsisEditorArea.visible ? synopsisEditorArea.height : 0)
                }
                property bool commentsExpanded: contentView.commentsExpanded
                expanded: commentsExpanded
                onCommentsExpandedChanged: expanded = commentsExpanded
                onExpandedChanged: contentView.commentsExpanded = expanded
                maxPanelWidth: Math.min(contentView.spaceForComments, 400)
                width: maxPanelWidth
                clip: true
                visible: width >= 100 && screenplayEditorSettings.displaySceneComments
                opacity: expanded ? (screenplayAdapter.currentIndex < 0 || screenplayAdapter.currentIndex === contentItem.theIndex ? 1 : 0.75) : 1
                Behavior on opacity {
                    enabled: applicationSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }
                content: TabView3 {
                    id: commentsSidePanelTabView
                    tabBarVisible: false
                    tabColor: commentsSidePanel.theSceneDarkColor
                    currentTabContent: currentTabIndex === 0 ? commentsEditComponent : featuredPhotoComponent
                    currentTabIndex: contentView.commentsPanelTabIndex

                    Component {
                        id: featuredPhotoComponent

                        SceneFeaturedImage {
                            scene: contentItem.theScene
                            fillModeAttrib: "commentsPanelFillMode"
                            defaultFillMode: Image.PreserveAspectCrop
                            mipmap: !(contentView.moving || contentView.flicking)
                        }
                    }

                    Component {
                        id: commentsEditComponent

                        TextAreaInput {
                            id: commentsEdit
                            background: Rectangle {
                                color: Qt.tint(contentItem.theScene.color, "#E7FFFFFF")
                            }
                            font.pointSize: Scrite.app.idealFontPointSize + 1
                            onTextChanged: contentItem.theScene.comments = text
                            wrapMode: Text.WordWrap
                            text: contentItem.theScene.comments
                            leftPadding: 10
                            rightPadding: 10
                            topPadding: 10
                            bottomPadding: 10
                            readOnly: Scrite.document.readOnly
                            onActiveFocusChanged: {
                                if(activeFocus)
                                    privateData.changeCurrentIndexTo(contentItem.theIndex)
                            }

                            Transliterator.spellCheckEnabled: screenplayEditorSettings.enableSpellCheck

                            SpecialSymbolsSupport {
                                anchors.top: parent.bottom
                                anchors.left: parent.left
                                textEditor: commentsEdit
                                textEditorHasCursorInterface: true
                                enabled: !Scrite.document.readOnly
                            }

                            SpellingSuggestionsMenu2 { }

                            Item {
                                x: parent.cursorRectangle.x
                                y: parent.cursorRectangle.y
                                width: parent.cursorRectangle.width
                                height: parent.cursorRectangle.height

                                ToolTip.visible: parent.height < parent.contentHeight
                                ToolTip.text: "Please consider capturing long comments as scene notes in the notebook tab."
                                ToolTip.delay: 1000
                            }
                        }
                    }
                }
            }

            property real totalSceneHeadingHeight: sceneHeadingAreaLoader.height + (synopsisEditorArea.visible ? synopsisEditorArea.height : 0)

            Column {
                id: contentItemLayout
                width: parent.width

                Loader {
                    id: sceneHeadingAreaLoader
                    width: parent.width
                    active: contentItem.theScene
                    sourceComponent: sceneHeadingArea
                    z: 1
                    onItemChanged: {
                        if(item) {
                            item.theElementIndex = contentItem.theIndex
                            item.theScene = contentItem.theScene
                            item.theElement = contentItem.theElement
                            item.sceneTextEditor = sceneTextEditor
                        }
                    }

                    function edit() {
                        if(item)
                            item.edit()
                    }

                    function editSceneNumber() {
                        if(item)
                            item.editSceneNumber()
                    }

                    Announcement.onIncoming: (type,data) => {
                        if(!sceneTextEditor.activeFocus)
                            return
                        var stype = "" + type
                        var sdata = "" + data
                        if(stype === "2E3BBE4F-05FE-49EE-9C0E-3332825B72D8") {
                            if(sdata === "Scene Heading")
                                sceneHeadingAreaLoader.edit()
                            else if(sdata === "Scene Number")
                                sceneHeadingAreaLoader.editSceneNumber()
                        }
                    }
                }

                Rectangle {
                    id: synopsisEditorArea
                    width: parent.width
                    height: synopsisEditorLayout.height + 10*Math.min(zoomLevel,1)
                    color: Qt.tint(contentItem.theScene.color, "#E7FFFFFF")
                    visible: screenplayEditorSettings.displaySceneSynopsis

                    Column {
                        id: synopsisEditorLayout
                        width: parent.width-10
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: ruler.leftMarginPx
                        anchors.rightMargin: ruler.rightMarginPx

                        Text {
                            id: synopsisEditorHeading
                            text: (contentItem.theScene.structureElement.hasNativeTitle ? contentItem.theScene.structureElement.nativeTitle : "Synopsis") + ":"
                            font.bold: true
                            font.pointSize: sceneHeadingFieldsFontPointSize
                            visible: synopsisEditorField.length > 0
                            width: parent.width*0.8
                            elide: Text.ElideMiddle
                        }

                        TextAreaInput {
                            id: synopsisEditorField
                            width: parent.width
                            font.pointSize: sceneHeadingFieldsFontPointSize
                            readOnly: Scrite.document.readOnly
                            text: contentItem.theScene.synopsis
                            Transliterator.spellCheckEnabled: screenplayEditorSettings.enableSpellCheck
                            onTextChanged: contentItem.theScene.synopsis = text
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            placeholderText: "Enter the synopsis of your scene here."
                            background: Item { }

                            SpellingSuggestionsMenu2 { }

                            onActiveFocusChanged: {
                                if(activeFocus) {
                                    contentView.ensureVisible(synopsisEditorField, Qt.rect(0, -10, cursorRectangle.width, cursorRectangle.height+20))
                                    privateData.changeCurrentIndexTo(contentItem.theIndex)
                                } else
                                    Qt.callLater( function() { synopsisEditorField.cursorPosition = -1 } )
                            }
                            Keys.onTabPressed: sceneTextEditor.forceActiveFocus()

                            property int sceneTextEditorCursorPosition: -1
                            Keys.onReturnPressed: {
                                if(event.modifiers & Qt.ShiftModifier) {
                                    event.accepted = false
                                    return
                                }
                                if(sceneTextEditorCursorPosition >= 0)
                                    sceneTextEditor.cursorPosition = sceneTextEditorCursorPosition
                                sceneTextEditor.forceActiveFocus()
                            }

                            Announcement.onIncoming: (type,data) => {
                                if(!sceneTextEditor.activeFocus || !screenplayEditorSettings.displaySceneSynopsis)
                                    return
                                var sdata = "" + data
                                var stype = "" + type
                                if(stype === "2E3BBE4F-05FE-49EE-9C0E-3332825B72D8" && sdata === "Scene Synopsis") {
                                    synopsisEditorField.sceneTextEditorCursorPosition = sceneTextEditor.cursorPosition
                                    synopsisEditorField.forceActiveFocus()
                                }
                            }
                        }
                    }
                }

                TextArea {
                    // Basic editing functionality
                    id: sceneTextEditor
                    width: parent.width
                    height: Math.ceil(contentHeight + topPadding + bottomPadding + sceneEditorFontMetrics.lineSpacing)
                    topPadding: sceneEditorFontMetrics.height
                    bottomPadding: sceneEditorFontMetrics.height
                    leftPadding: ruler.leftMarginPx
                    rightPadding: ruler.rightMarginPx
                    palette: Scrite.app.palette
                    selectByMouse: true
                    selectByKeyboard: true
                    persistentSelection: true
                    // renderType: TextArea.NativeRendering
                    property bool hasSelection: selectionStart >= 0 && selectionEnd >= 0 && selectionEnd > selectionStart
                    property Scene scene: contentItem.theScene
                    readOnly: Scrite.document.readOnly

                    background: Item {
                        id: sceneTextEditorBackground

                        ResetOnChange {
                            id: document
                            trackChangesOn: sceneDocumentBinder.documentLoadCount + zoomSlider.value
                            from: null
                            to: screenplayTextDocument.paused ? null : screenplayTextDocument
                            delay: 100
                        }

                        ScreenplayElementPageBreaks {
                            id: pageBreaksEvaluator
                            screenplayElement: contentItem.theElement
                            screenplayDocument: Scrite.document.loading ? null : document.value
                        }

                        Repeater {
                            model: pageBreaksEvaluator.pageBreaks

                            Item {
                                id: pageBreakLine
                                property rect cursorRect: modelData.position >= 0 ? sceneTextEditor.positionToRectangle(modelData.position) : Qt.rect(0,0,0,0)
                                x: 0
                                y: (modelData.position >= 0 ? cursorRect.y : -contentItem.totalSceneHeadingHeight) - height/2
                                width: sceneTextEditorBackground.width
                                height: 1
                                // color: primaryColors.c400.background

                                PageNumberBubble {
                                    x: -width - 20
                                    pageNumber: modelData.pageNumber
                                }
                            }
                        }

                        Rectangle {
                            visible: sceneTextEditor.cursorVisible && sceneTextEditor.activeFocus && screenplayEditorSettings.highlightCurrentLine && Scrite.app.usingMaterialTheme
                            x: 0; y: sceneTextEditor.cursorRectangle.y-2*zoomLevel
                            width: parent.width
                            height: sceneTextEditor.cursorRectangle.height+4*zoomLevel
                            color: primaryColors.c100.background

                            Rectangle {
                                width: currentSceneHighlight.width
                                anchors.left: parent.left
                                anchors.leftMargin: currentSceneHighlight.width
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                color: currentSceneHighlight.color
                            }
                        }
                    }
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font: screenplayFormat.defaultFont2
                    placeholderText: activeFocus ? "" : "Click here to type your scene content..."
                    onActiveFocusChanged: {
                        if(activeFocus) {
                            completionModel.actuallyEnable = true
                            contentView.ensureVisible(sceneTextEditor, cursorRectangle)
                            privateData.changeCurrentIndexTo(contentItem.theIndex)
                            globalScreenplayEditorToolbar.sceneEditor = contentItem
                            markupTools.sceneDocumentBinder = sceneDocumentBinder
                            justReceivedFocus = true
                        } else {
                            if(globalScreenplayEditorToolbar.sceneEditor === contentItem)
                                globalScreenplayEditorToolbar.sceneEditor = null
                            if(markupTools.sceneDocumentBinder === sceneDocumentBinder)
                                markupTools.sceneDocumentBinder = null
                        }
                    }

                    function reload() {
                        Utils.execLater(sceneDocumentBinder, 1000, function() {
                            sceneDocumentBinder.preserveScrollAndReload()
                        } )
                    }

                    Connections {
                        target: contentItem.theScene

                        property int cursorPositionBeforeReset: -1

                        function onSceneAboutToReset() {
                            if(sceneTextEditor.activeFocus)
                                cursorPositionBeforeReset = sceneTextEditor.cursorPosition
                            else
                                cursorPositionBeforeReset = -1
                            sceneTextEditor.keepCursorInView = false
                        }

                        function onSceneReset(cp) {
                            Utils.execLater(sceneTextEditor, 50, () => { sceneTextEditor.keepCursorInView = true } )
                            if(cursorPositionBeforeReset >= 0) {
                                contentItem.assumeFocusLater(cursorPositionBeforeReset, 250)
                                if(cp >= 0)
                                    contentItem.assumeFocusLater(cp, 500)
                            }
                            cursorPositionBeforeReset = -1
                        }
                    }

                    property bool keepCursorInView: true
                    onCursorRectangleChanged: if(keepCursorInView) Qt.callLater(bringCursorToView)

                    function bringCursorToView() {
                        if(activeFocus /*&& contentView.isVisible(contentItem.theIndex)*/) {
                            const tcr = cursorRectangle
                            const buffer = Math.max(contentView.height * 0.2, tcr.height*3)
                            const cr = Qt.rect(tcr.x, tcr.y-buffer*0.3, tcr.width, buffer)

                            const crv = contentView.mapFromItem(sceneTextEditor, cr)
                            if(crv.y >= 0 && crv.y < contentView.height-cr.height)
                                return

                            const cy = contentView.contentY
                            contentView.allowContentYAnimation = true
                            contentView.ensureVisible(sceneTextEditor, cr)
                            if(cy == contentView.contentY)
                                contentView.allowContentYAnimation = false
                        }
                    }

                    property bool justReceivedFocus: false

                    Loader {
                        id: cursorHighlight
                        x: sceneTextEditor.cursorRectangle.x
                        y: sceneTextEditor.cursorRectangle.y
                        width: sceneTextEditor.cursorRectangle.width
                        height: sceneTextEditor.cursorRectangle.height
                        active: sceneTextEditor.justReceivedFocus

                        sourceComponent: Item {

                            Rectangle {
                                id: cursorRectangle
                                width: parent.width*Screen.devicePixelRatio
                                height: parent.height
                                anchors.centerIn: parent
                                color: Scrite.document.readOnly ? primaryColors.borderColor : "black"
                            }

                            SequentialAnimation {
                                running: true

                                ParallelAnimation {
                                    NumberAnimation {
                                        target: cursorRectangle
                                        property: "width"
                                        duration: 250
                                        from: sceneTextEditor.cursorRectangle.width*Screen.devicePixelRatio
                                        to: sceneTextEditor.cursorRectangle.width*10
                                    }

                                    NumberAnimation {
                                        target: cursorRectangle
                                        property: "height"
                                        duration: 250
                                        from: sceneTextEditor.cursorRectangle.height
                                        to: sceneTextEditor.cursorRectangle.height*2
                                    }

                                    NumberAnimation {
                                        target: cursorRectangle
                                        property: "opacity"
                                        duration: 250
                                        from: 1
                                        to: 0.1
                                    }
                                }

                                ParallelAnimation {
                                    NumberAnimation {
                                        target: cursorRectangle
                                        property: "width"
                                        duration: 250
                                        from: sceneTextEditor.cursorRectangle.width*10
                                        to: sceneTextEditor.cursorRectangle.width*1.5
                                    }

                                    NumberAnimation {
                                        target: cursorRectangle
                                        property: "height"
                                        duration: 250
                                        from: sceneTextEditor.cursorRectangle.height*2
                                        to: sceneTextEditor.cursorRectangle.height
                                    }

                                    NumberAnimation {
                                        target: cursorRectangle
                                        property: "opacity"
                                        duration: 250
                                        from: 0.1
                                        to: 1
                                    }
                                }

                                ScriptAction {
                                    script: {
                                        sceneTextEditor.justReceivedFocus = false
                                    }
                                }
                            }
                        }
                    }

                    // Support for transliteration.
                    property bool userIsTyping: false
                    EventFilter.target: Scrite.app
                    EventFilter.active: sceneTextEditor.activeFocus
                    EventFilter.events: [EventFilter.KeyPress] // Wheel, ShortcutOverride
                    EventFilter.onFilter: {
                        if(object === sceneTextEditor) {
                            // Enter, Tab and other keys must not trigger
                            // Transliteration. Only space should.
                            sceneTextEditor.userIsTyping = event.hasText
                            completionModel.actuallyEnable = event.hasText
                            result.filter = event.controlModifier && (event.key === Qt.Key_Z || event.key === Qt.Key_Y)
                        } else if(event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
                            if(event.key === Qt.Key_PageUp)
                                contentItem.scrollToPreviousScene()
                            else
                                contentItem.scrollToNextScene()
                            result.filter = true
                            result.acceptEvent = true
                        }
                    }
                    Transliterator.enabled: contentItem.theScene && !contentItem.theScene.isBeingReset && userIsTyping
                    Transliterator.textDocument: textDocument
                    Transliterator.cursorPosition: cursorPosition
                    Transliterator.hasActiveFocus: activeFocus
                    Transliterator.applyLanguageFonts: false // SceneDocumentBinder handles it separately.
                    Transliterator.spellCheckEnabled: false // SceneDocumentBinder handles it separately.
                    Transliterator.onAboutToTransliterate: {
                        contentItem.theScene.beginUndoCapture(false)
                        contentItem.theScene.undoRedoEnabled = false
                    }
                    Transliterator.onFinishedTransliterating: {
                        contentItem.theScene.endUndoCapture()
                        contentItem.theScene.undoRedoEnabled = true
                    }

                    // Support for auto completion
                    Item {
                        id: cursorOverlay
                        x: parent.cursorRectangle.x
                        y: parent.cursorRectangle.y
                        width: parent.cursorRectangle.width
                        height: parent.cursorRectangle.height
                        visible: parent.cursorVisible

                        SpecialSymbolsSupport {
                            anchors.top: parent.bottom
                            anchors.left: parent.left
                            textEditor: sceneTextEditor
                            // Because of a bug in Qt we will be unable to include Emoji's in
                            // the generated PDFs.
                            // More information about the bug can be found here:
                            // https://bugreports.qt.io/browse/QTBUG-78833
                            // When we update Qt to say 5.15 in the next cycle, we can allow
                            // Emoji's to be used within the screenplay editor. Until then, it
                            // just won't work.
                            includeEmojis: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform
                            textEditorHasCursorInterface: true
                            enabled: !Scrite.document.readOnly
                        }

                        ResetOnChange {
                            id: completionModelEnable
                            trackChangesOn: sceneTextEditor.cursorRectangle.y
                            from: false
                            to: true
                            delay: 250
                        }

                        Connections {
                            target: sceneDocumentBinder
                            function onCompletionModeChanged() {
                                completionModel.completable = false
                                Utils.execLater(completionModel, 250, updateCompletionModel)
                            }
                            function updateCompletionModel() {
                                completionModel.strings = sceneDocumentBinder.autoCompleteHints
                                completionModel.priorityStrings = sceneDocumentBinder.priorityAutoCompleteHints
                                completionModel.completable = sceneDocumentBinder.completionMode !== SceneDocumentBinder.NoCompletionMode
                            }
                        }

                        BatchChange {
                            id: completionModelCount
                            trackChangesOn: completionModel.count
                        }

                        CompletionModel {
                            id: completionModel
                            property bool actuallyEnable: true
                            property string suggestion: currentCompletion
                            property bool hasSuggestion: completionModelCount.value > 0
                            property bool completable: false
                            enabled: /*allowEnable &&*/ sceneTextEditor.activeFocus && completionModelEnable.value && completable
                            sortStrings: false
                            acceptEnglishStringsOnly: false
                            completionPrefix: sceneDocumentBinder.completionPrefix
                            filterKeyStrokes: sceneTextEditor.activeFocus
                            maxVisibleItems: -1
                            onRequestCompletion: {
                                sceneTextEditor.acceptCompletionSuggestion()
                                Announcement.shout("E69D2EA0-D26D-4C60-B551-FD3B45C5BE60", contentItem.theScene.id)
                            }
                            minimumCompletionPrefixLength: 0
                            onHasSuggestionChanged: {
                                if(hasSuggestion) {
                                    if(!completionViewPopup.visible)
                                        completionViewPopup.open()
                                } else {
                                    if(completionViewPopup.visible)
                                        completionViewPopup.close()
                                }
                            }
                        }

                        Popup {
                            id: completionViewPopup
                            x: -Scrite.app.boundingRect(completionModel.completionPrefix, defaultFontMetrics.font).width
                            y: parent.height
                            width: Scrite.app.largestBoundingRect(completionModel.strings, defaultFontMetrics.font).width + leftInset + rightInset + leftPadding + rightPadding + 30
                            height: completionView.height + topInset + bottomInset + topPadding + bottomPadding
                            focus: false
                            closePolicy: Popup.NoAutoClose
                            Connections {
                                target: dialogUnderlay
                                function onVisibleChanged() {
                                    if(dialogUnderlay.visible)
                                        completionViewPopup.close()
                                }
                            }
                            contentItem: ListView {
                                id: completionView
                                model: completionModel
                                clip: true
                                FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                                height: Math.min(contentHeight, 7*(defaultFontMetrics.lineSpacing+2*5))
                                interactive: true
                                ScrollBar.vertical: ScrollBar2 {
                                    flickable: completionView
                                }
                                delegate: Text {
                                    width: completionView.width-(completionView.contentHeight > completionView.height ? 20 : 1)
                                    text: string
                                    padding: 5
                                    font: defaultFontMetrics.font
                                    color: index === completionView.currentIndex ? primaryColors.highlight.text : primaryColors.c10.text
                                    MouseArea {
                                        property bool singleClickAutoComplete: screenplayEditorSettings.singleClickAutoComplete
                                        anchors.fill: parent
                                        hoverEnabled: singleClickAutoComplete
                                        onContainsMouseChanged: if(singleClickAutoComplete) completionModel.currentRow = index
                                        cursorShape: singleClickAutoComplete ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        onClicked: {
                                            if(singleClickAutoComplete || completionModel.currentRow === index)
                                                completionModel.requestCompletion( completionModel.currentCompletion )
                                            else
                                                completionModel.currentRow = index
                                        }
                                        onDoubleClicked: completionModel.requestCompletion( completionModel.currentCompletion )
                                    }
                                }
                                highlightMoveDuration: 0
                                highlightResizeDuration: 0
                                highlight: Rectangle {
                                    color: primaryColors.highlight.background
                                }
                                currentIndex: completionModel.currentRow
                            }
                        }

                        // Context menus must ideally show up directly below the cursor
                        // So, we keep the menu loaders inside the cursorOverlay
                        SpellingSuggestionsMenu {
                            id: spellingSuggestionsMenuLoader
                            anchors.bottom: parent.bottom
                            spellingSuggestions: sceneDocumentBinder.spellingSuggestions

                            property int cursorPosition: -1

                            onReplaceRequest: (suggestion) => {
                                                  if(cursorPosition >= 0) {
                                                      sceneDocumentBinder.replaceWordAt(cursorPosition, suggestion)
                                                      sceneTextEditor.cursorPosition = cursorPosition
                                                  }
                                              }

                            onMenuAboutToShow: () => {
                                                   cursorPosition = sceneTextEditor.cursorPosition
                                                   sceneTextEditor.persistentSelection = true
                                               }

                            onMenuAboutToHide: () => {
                                                   sceneTextEditor.persistentSelection = false
                                                   sceneTextEditor.forceActiveFocus()
                                                   sceneTextEditor.cursorPosition = cursorPosition
                                               }

                            onAddToDictionaryRequest: () => {
                                                          sceneDocumentBinder.addWordUnderCursorToDictionary()
                                                          ++contentView.numberOfWordsAddedToDict
                                                      }

                            onAddToIgnoreListRequest: () => {
                                                          sceneDocumentBinder.addWordUnderCursorToIgnoreList()
                                                          ++contentView.numberOfWordsAddedToDict
                                                      }
                        }

                        MenuLoader {
                            id: editorContextMenu
                            anchors.bottom: parent.bottom
                            enabled: !Scrite.document.readOnly
                            menu: Menu2 {
                                property int sceneTextEditorCursorPosition: -1
                                property SceneElement sceneCurrentElement
                                property TextFormat sceneTextFormat: sceneDocumentBinder.textFormat
                                onAboutToShow: {
                                    sceneCurrentElement = sceneDocumentBinder.currentElement
                                    sceneTextEditorCursorPosition = sceneTextEditor.cursorPosition
                                    sceneTextEditor.persistentSelection = true
                                }
                                onAboutToHide: sceneTextEditor.persistentSelection = false

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
                                    enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
                                    onClicked: { sceneTextEditor.cut2(); editorContextMenu.close() }
                                }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
                                    enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
                                    onClicked: { sceneTextEditor.copy2(); editorContextMenu.close() }
                                }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
                                    enabled: sceneTextEditor.canPaste
                                    onClicked: { sceneTextEditor.paste2(); editorContextMenu.close() }
                                }

                                MenuSeparator {  }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Split Scene"
                                    enabled: contentItem.canSplitScene
                                    onClicked: {
                                        sceneTextEditor.splitSceneAt(sceneTextEditorCursorPosition)
                                        editorContextMenu.close()
                                    }
                                }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Join Previous Scene"
                                    enabled: contentItem.canJoinToPreviousScene
                                    onClicked: {
                                        sceneTextEditor.mergeWithPreviousScene()
                                        editorContextMenu.close()
                                    }
                                }

                                MenuSeparator {  }

                                Menu2 {
                                    title: "Format"
                                    width: 250

                                    Repeater {
                                        model: [
                                            { "value": SceneElement.Action, "display": "Action" },
                                            { "value": SceneElement.Character, "display": "Character" },
                                            { "value": SceneElement.Dialogue, "display": "Dialogue" },
                                            { "value": SceneElement.Parenthetical, "display": "Parenthetical" },
                                            { "value": SceneElement.Shot, "display": "Shot" },
                                            { "value": SceneElement.Transition, "display": "Transition" }
                                        ]

                                        MenuItem2 {
                                            focusPolicy: Qt.NoFocus
                                            text: modelData.display + "\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+" + (index+1))
                                            enabled: sceneCurrentElement !== null
                                            onClicked: {
                                                sceneCurrentElement.type = modelData.value
                                                editorContextMenu.close()
                                            }
                                        }
                                    }
                                }

                                Menu2 {
                                    title: "Translate"
                                    enabled: sceneTextEditor.hasSelection

                                    Repeater {
                                        model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                                        MenuItem2 {
                                            focusPolicy: Qt.NoFocus
                                            visible: index >= 0
                                            enabled: modelData.value !== TransliterationEngine.English
                                            text: modelData.key
                                            onClicked: {
                                                editorContextMenu.close()
                                                sceneTextEditor.forceActiveFocus()
                                                sceneTextEditor.scene.beginUndoCapture()
                                                sceneTextEditor.Transliterator.transliterateToLanguage(sceneTextEditor.selectionStart, sceneTextEditor.selectionEnd, modelData.value)
                                                sceneTextEditor.scene.endUndoCapture()
                                                sceneTextEditor.reload()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: sceneTextEditor.activeFocus && !Scrite.document.readOnly
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: completionModel.hasSuggestion ? "Auto-complete" : sceneDocumentBinder.nextTabFormatAsString
                        ShortcutsModelItem.shortcut: "Tab"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: sceneTextEditor.activeFocus && !Scrite.document.readOnly
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: "Create New Paragraph"
                        ShortcutsModelItem.shortcut: Scrite.app.isMacOSPlatform ? "Return" : "Enter"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: contentItem.canSplitScene
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: "Split Scene"
                        ShortcutsModelItem.shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Return" : "Ctrl+Enter"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: contentItem.canJoinToPreviousScene
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: "Join Previous Scene"
                        ShortcutsModelItem.shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Delete" : "Ctrl+Backspace"
                    }

                    function acceptCompletionSuggestion() {
                        if(completionModel.suggestion !== "") {
                            var suggestion = completionModel.suggestion
                            userIsTyping = false
                            if(sceneDocumentBinder.hasCompletionPrefixBoundary)
                                remove(sceneDocumentBinder.completionPrefixStart, sceneDocumentBinder.completionPrefixEnd)
                            else
                                remove(sceneDocumentBinder.currentBlockPosition(), cursorPosition)
                            insert(cursorPosition, suggestion)
                            userIsTyping = true
                            Transliterator.enableFromNextWord()
                            completionModel.actuallyEnable = false
                            return true
                        }
                        return false
                    }

                    Keys.onTabPressed: {
                        if(!Scrite.document.readOnly) {
                            // if(!acceptCompletionSuggestion())
                            // https://www.scrite.io/index.php/forum/topic/cant-press-tab-to-get-back-to-action/
                            // This was a good suggestion. Since we now show auto-complete popup,
                            // it just doesnt make sense to auto-complete on tab.
                            sceneDocumentBinder.tab()
                            event.accepted = true
                        }
                    }
                    Keys.onBacktabPressed: {
                        if(!Scrite.document.readOnly)
                            sceneDocumentBinder.backtab()
                    }

                    // split-scene handling.
                    Keys.onReturnPressed: {
                        if(Scrite.document.readOnly) {
                            event.accepted = false
                            return
                        }

                        if(event.modifiers & Qt.ControlModifier) {
                            contentItem.splitScene()
                            event.accepted = true
                            return
                        }

                        event.accepted = false
                    }

                    // Context menu
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        enabled: !Scrite.document.readOnly && contextMenuEnableBinder.get
                        cursorShape: Qt.IBeamCursor
                        onClicked: {
                            mouse.accepted = true
                            sceneTextEditor.persistentSelection = true
                            if(!sceneTextEditor.hasSelection && sceneDocumentBinder.spellCheckEnabled) {
                                sceneTextEditor.cursorPosition = sceneTextEditor.positionAt(mouse.x, mouse.y)
                                if(sceneDocumentBinder.wordUnderCursorIsMisspelled) {
                                    spellingSuggestionsMenuLoader.popup()
                                    return
                                }
                            }
                            sceneTextEditor.persistentSelection = false
                            editorContextMenu.popup()
                        }

                        DelayedPropertyBinder {
                            id: contextMenuEnableBinder
                            initial: false
                            set: !editorContextMenu.active && !spellingSuggestionsMenuLoader.active && sceneTextEditor.activeFocus
                            delay: 100
                        }
                    }

                    // Scrolling up and down
                    Keys.onUpPressed: {
                        if(sceneTextEditor.hasSelection) {
                            event.accepted = sceneTextEditor.cursorPosition === 0
                            return
                        }

                        if(event.modifiers & Qt.ControlModifier) {
                            contentItem.scrollToPreviousScene()
                            event.accepted = true
                            return
                        }

                        if(sceneDocumentBinder.canGoUp())
                            event.accepted = false
                        else {
                            event.accepted = true
                            contentItem.scrollToPreviousScene()
                        }
                    }
                    Keys.onDownPressed: {
                        if(sceneTextEditor.hasSelection) {
                            event.accepted = sceneTextEditor.cursorPosition >= sceneTextEditor.length -1
                            return
                        }

                        if(event.modifiers & Qt.ControlModifier) {
                            contentItem.scrollToNextScene()
                            event.accepted = true
                            return
                        }

                        if(sceneDocumentBinder.canGoDown())
                            event.accepted = false
                        else {
                            event.accepted = true
                            contentItem.scrollToNextScene()
                        }
                    }
                    Keys.onPressed: {
                        event.accepted = false

                        if(event.modifiers === Qt.ControlModifier) {
                            switch(event.key) {
                            case Qt.Key_Delete:
                                if(Scrite.app.isMacOSPlatform) {
                                    event.accepted = true
                                    if(sceneTextEditor.cursorPosition === 0)
                                        contentItem.mergeWithPreviousScene()
                                    else
                                        contentItem.showCantMergeSceneMessage()
                                }
                                break
                            case Qt.Key_Backspace:
                                if(sceneTextEditor.cursorPosition === 0) {
                                    event.accepted = true
                                    contentItem.mergeWithPreviousScene()
                                }
                                else
                                    contentItem.showCantMergeSceneMessage()
                                break
                            case Qt.Key_0:
                                event.accepted = true
                                sceneHeadingAreaLoader.edit()
                                break
                            case Qt.Key_X:
                                event.accepted = true
                                cut2()
                                break
                            case Qt.Key_C:
                                event.accepted = true
                                copy2()
                                break
                            case Qt.Key_V:
                                event.accepted = true
                                paste2()
                                break
                            }
                        }
                    }

                    // Search & Replace
                    TextDocumentSearch {
                        id: textDocumentSearch
                        textDocument: sceneTextEditor.textDocument
                        searchString: sceneDocumentBinder.documentLoadCount > 0 ? (contentItem.theElement.userData ? contentItem.theElement.userData.searchString : "") : ""
                        currentResultIndex: searchResultCount > 0 ? (contentItem.theElement.userData ? contentItem.theElement.userData.sceneResultIndex : -1) : -1
                        onHighlightText: selection = {"start": start, "end": end}
                        onClearHighlight: selection = { "start": -1, "end": -1 }

                        property var selection: { "start": -1, "end": -1 }
                        property int loadCount: sceneDocumentBinder.documentLoadCount

                        onLoadCountChanged: Qt.callLater(highlightSearchResultTextSnippet)
                        onSelectionChanged: Qt.callLater(highlightSearchResultTextSnippet)
                        Component.onCompleted: Qt.callLater(highlightSearchResultTextSnippet)

                        function highlightSearchResultTextSnippet() {
                            if(selection.start >= 0 && selection.end >= 0) {
                                if(sceneTextEditor.selectionStart === selection.start && sceneTextEditor.selectionEnd === selection.end )
                                    return;

                                sceneTextEditor.select(selection.start, selection.end)
                                sceneTextEditor.update()
                                Utils.execLater(textDocumentSearch, 50, scrollToSelection)
                            } else {
                                sceneTextEditor.deselect()
                            }
                        }

                        function scrollToSelection() {
                            var rect = Scrite.app.uniteRectangles( sceneTextEditor.positionToRectangle(selection.start),
                                                           sceneTextEditor.positionToRectangle(selection.end) )
                            rect = Scrite.app.adjustRectangle(rect, -20, -50, 20, 50)
                            contentView.ensureVisible(contentItem, rect)
                            contentView.ensureVisible(contentItem, rect)
                        }
                    }

                    Connections {
                        target: searchAgents.count > 0 ? searchAgents.itemAt(0).SearchAgent : null
                        ignoreUnknownSignals: true
                        function onReplaceCurrent(replacementText) {
                            if(textDocumentSearch.currentResultIndex >= 0) {
                                contentItem.theScene.beginUndoCapture()
                                textDocumentSearch.replace(replacementText)
                                contentItem.theScene.endUndoCapture()
                            }
                        }
                    }

                    // Custom Copy & Paste
                    function cut2() {
                        if(Scrite.document.readOnly)
                            return

                        if(hasSelection) {
                            sceneDocumentBinder.copy(selectionStart, selectionEnd)
                            remove(selectionStart, selectionEnd)
                            justReceivedFocus = true
                        }
                    }

                    function copy2() {
                        if(hasSelection)
                            sceneDocumentBinder.copy(selectionStart, selectionEnd)
                    }

                    function paste2() {
                        if(Scrite.document.readOnly)
                            return

                        if(canPaste) {
                            // Fix for https://github.com/teriflix/scrite/issues/195
                            // [0.5.2 All] Pasting doesnt replace the selected text #195
                            if(sceneTextEditor.hasSelection)
                                sceneTextEditor.remove(sceneTextEditor.selectionStart, sceneTextEditor.selectionEnd)
                            var cp = sceneTextEditor.cursorPosition
                            var cp2 = sceneDocumentBinder.paste(sceneTextEditor.cursorPosition)
                            if(cp2 < 0)
                                sceneTextEditor.paste()
                            else
                                sceneTextEditor.cursorPosition = cp2
                            justReceivedFocus = true
                        }
                    }

                    function splitSceneAt(pos) {
                        if(Scrite.document.readOnly)
                            return

                        Qt.callLater( function() {
                            forceActiveFocus()
                            cursorPosition = pos
                            contentItem.splitScene()
                        })
                    }

                    function mergeWithPreviousScene() {
                        if(Scrite.document.readOnly)
                            return

                        Qt.callLater( function() {
                            forceActiveFocus()
                            cursorPosition = 0
                            contentItem.mergeWithPreviousScene()
                        })
                    }

                    // Highlight cursor after undo/redo
                    Connections {
                        target: contentItem.theScene
                        ignoreUnknownSignals: true
                        enabled: sceneTextEditor.activeFocus && !sceneTextEditor.readOnly
                        function onSceneRefreshed() { sceneTextEditor.justReceivedFocus = true }

                        //
                        property int preResetCursorPosition: -1
                        function onModelAboutToBeReset() {
                            if(sceneTextEditor.activeFocus)
                                preResetCursorPosition = sceneTextEditor.cursorPosition
                        }

                        function onModelReset() {
                            if(preResetCursorPosition >= 0) {
                                contentItem.assumeFocusLater(preResetCursorPosition, 100)
                                preResetCursorPosition = -1
                            }
                        }
                    }

                    Connections {
                        target: screenplayTextDocument
                        ignoreUnknownSignals: true
                        enabled: sceneTextEditor.activeFocus && !sceneTextEditor.readOnly
                        property bool needsCursorAnimation: false
                        function onUpdateScheduled() { needsCursorAnimation = true }
                        function onUpdateFinished() {
                            if(needsCursorAnimation)
                                Qt.callLater( function() {
                                    sceneTextEditor.justReceivedFocus = true
                                })
                            needsCursorAnimation = false
                        }
                    }
                }
            }

            Rectangle {
                id: currentSceneHighlight
                width: parent.width * 0.01
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: Scrite.app.isVeryLightColor(contentItem.theScene.color) ? primaryColors.highlight.background : Qt.tint(contentItem.theScene.color, "#9CFFFFFF")
                visible: screenplayAdapter.currentIndex === contentItem.theIndex
            }

            function mergeWithPreviousScene() {
                if(!contentItem.canJoinToPreviousScene) {
                    showCantMergeSceneMessage()
                    return
                }
                Scrite.document.setBusyMessage("Merging scene...")
                Utils.execLater(contentItem, 100, mergeWithPreviousSceneImpl)
            }

            function mergeWithPreviousSceneImpl() {
                screenplayTextDocument.syncEnabled = false
                var ret = screenplayAdapter.mergeElementWithPrevious(contentItem.theElement)
                screenplayTextDocument.syncEnabled = true
                Scrite.document.clearBusyMessage()
                if(ret === null)
                    showCantMergeSceneMessage()
                contentView.scrollIntoView(screenplayAdapter.currentIndex)
            }

            function showCantMergeSceneMessage() {
                showInformation({
                    "message": "Scene can be merged only when cursor is placed at the start of the first paragraph in a scene.",
                    "closeOnEscape": true
                })
            }

            function splitScene() {
                if(!contentItem.canSplitScene) {
                    showCantSplitSceneMessage()
                    return
                }
                Scrite.document.setBusyMessage("Splitting scene...")
                Utils.execLater(contentItem, 100, splitSceneImpl)
            }

            function splitSceneImpl() {
                screenplayTextDocument.syncEnabled = false
                postSplitElementTimer.newCurrentIndex = contentItem.theIndex+1
                var ret = screenplayAdapter.splitElement(contentItem.theElement, sceneDocumentBinder.currentElement, sceneDocumentBinder.currentElementCursorPosition)
                screenplayTextDocument.syncEnabled = true
                Scrite.document.clearBusyMessage()
                if(ret === null)
                    showCantSplitSceneMessage()
                else
                    postSplitElementTimer.start()
            }

            function showCantSplitSceneMessage() {
                showInformation({
                    "message": "Scene can be split only when cursor is placed at the start of a paragraph.",
                    "closeOnEscape": true
                })
            }

            function assumeFocus() {
                if(!sceneTextEditor.activeFocus)
                    sceneTextEditor.forceActiveFocus()
            }

            function assumeFocusAt(pos) {
                if(!sceneTextEditor.activeFocus)
                    sceneTextEditor.forceActiveFocus()
                if(pos < 0)
                    sceneTextEditor.cursorPosition = sceneDocumentBinder.lastCursorPosition()
                else
                    sceneTextEditor.cursorPosition = pos
            }

            function assumeFocusLater(pos, delay) {
                if(delay === 0)
                    Qt.callLater( assumeFocusAt, pos )
                else
                    Utils.execLater(contentItem, delay, function() { contentItem.assumeFocusAt(pos) })
            }

            function scrollToPreviousScene() {
                contentView.scrollingBetweenScenes = true
                var idx = screenplayAdapter.previousSceneElementIndex()
                if(idx === 0 && idx === theIndex) {
                    contentView.scrollToFirstScene()
                    assumeFocusAt(0)
                    contentView.scrollingBetweenScenes = false
                    return
                }

                contentView.scrollIntoView(idx)
                Qt.callLater( function(iidx) {
                    //contentView.positionViewAtIndex(iidx, ListView.Contain)
                    var item = contentView.loadedItemAtIndex(iidx)
                    item.assumeFocusAt(-1)
                    contentView.scrollingBetweenScenes = false
                }, idx)
            }

            function scrollToNextScene() {
                contentView.scrollingBetweenScenes = true
                var idx = screenplayAdapter.nextSceneElementIndex()
                if(idx === screenplayAdapter.elementCount-1 && idx === theIndex) {
                    contentView.positionViewAtEnd()
                    assumeFocusAt(-1)
                    contentView.scrollingBetweenScenes = false
                    return
                }

                contentView.scrollIntoView(idx)
                Qt.callLater( function(iidx) {
                    //contentView.positionViewAtIndex(iidx, ListView.Contain)
                    var item = contentView.loadedItemAtIndex(iidx)
                    item.assumeFocusAt(0)
                    contentView.scrollingBetweenScenes = false
                }, idx)
            }
        }
    }

    Component {
        id: sceneHeadingArea

        Rectangle {
            id: headingItem
            property Scene theScene
            property int theElementIndex: -1
            property bool sceneHasFocus: false
            property ScreenplayElement theElement
            property TextArea sceneTextEditor

            function edit() {
                if(theScene.heading.enabled) {
                    sceneHeadingField.forceActiveFocus()
                    contentView.ensureVisible(sceneHeadingField, sceneHeadingField.cursorRectangle)
                }
            }

            function editSceneNumber() {
                if(theScene.heading.enabled) {
                    sceneNumberField.forceActiveFocus()
                    contentView.ensureVisible(sceneHeadingField, sceneHeadingField.cursorRectangle)
                }
            }

            height: sceneHeadingLayout.height + 12*Math.min(zoomLevel,1)
            color: Qt.tint(theScene.color, "#E7FFFFFF")

            Item {
                width: ruler.leftMarginPx
                height: parent.height

                Row {
                    id: sceneNumberFieldRow
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width * 0.075
                    spacing: 20
//                    property bool headingFieldOnly: !screenplayEditorSettings.displaySceneCharacters && !screenplayEditorSettings.displaySceneSynopsis
//                    onHeadingFieldOnlyChanged: to = parent.mapFromItem(sceneHeadingField, 0, sceneHeadingField.height).y - height

                    SceneTypeImage {
                        width: headingFontMetrics.height
                        height: width
                        lightBackground: Scrite.app.isLightColor(headingItem.color)
                        anchors.verticalCenter: sceneNumberField.verticalCenter
                        anchors.verticalCenterOffset: -headingFontMetrics.descent
                        sceneType: headingItem.theScene.type
                    }

                    TextField2 {
                        id: sceneNumberField
                        label: cursorVisible ? "Scene No." : ""
                        labelAlwaysVisible: false
                        width: headingFontMetrics.averageCharacterWidth*maximumLength
                        text: headingItem.theElement.userSceneNumber
                        anchors.bottom: parent.bottom
                        font: headingFontMetrics.font
                        onTextChanged: headingItem.theElement.userSceneNumber = text
                        maximumLength: 5
                        background: Item { }
                        placeholderText: headingItem.theElement.sceneNumber
                        visible: headingItem.theElement.elementType === ScreenplayElement.SceneElementType &&
                                 headingItem.theScene.heading.enabled &&
                                 screenplayAdapter.isSourceScreenplay
                        onActiveFocusChanged: if(activeFocus) privateData.changeCurrentIndexTo(headingItem.theElementIndex)
                        tabItem: headingItem.sceneTextEditor
                    }

                    Component.onCompleted: {
                        Qt.callLater( () => {
                                       y = Qt.binding( () => {
                                                return parent.mapFromItem(sceneHeadingField, 0, sceneHeadingField.height).y - height
                                            } )
                                     })
                    }
                }
            }

            Column {
                id: sceneHeadingLayout
                spacing: 5
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: ruler.leftMarginPx
                anchors.rightMargin: ruler.rightMarginPx
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: screenplayEditorSettings.displaySceneCharacters ? 8 : 4

                Item {
                    property real spacing: 5
                    width: parent.width
                    height: Math.max(sceneHeadingFieldArea.height, sceneMenuButton.height)

                    Item {
                        id: sceneHeadingFieldArea
                        anchors.left: parent.left
                        anchors.right: sceneTaggingButton.visible ? sceneTaggingButton.left : sceneMenuButton.left
                        height: headingFontMetrics.lineSpacing * lineCount
                        property int lineCount: Math.ceil((sceneHeadingField.length * headingFontMetrics.averageCharacterWidth)/width)
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: headingFontMetrics.descent

                        TextField2 {
                            id: sceneHeadingField
                            width: parent.width
                            anchors.verticalCenter: parent.verticalCenter

                            property SceneHeading sceneHeading: headingItem.theScene.heading
                            function updateSceneHeading(text) {
                                if(readOnly)
                                    return
                                sceneHeading.parseFrom(text)
                            }

                            text: {
                                if(headingItem.theElement.omitted)
                                    return "[OMITTED] " + (hovered ? sceneHeading.displayText : "")
                                if(activeFocus)
                                    return sceneHeading.editText
                                return sceneHeading.displayText
                            }
                            hoverEnabled: headingItem.theElement.omitted
                            readOnly: Scrite.document.readOnly || !(sceneHeading.enabled && !headingItem.theElement.omitted)
                            label: ""
                            placeholderText: enabled ? "INT. SOMEPLACE - DAY" : "NO SCENE HEADING"
                            maximumLength: 140
                            font.family: headingFontMetrics.font.family
                            font.pointSize: headingFontMetrics.font.pointSize
                            font.bold: headingFontMetrics.font.bold
                            font.underline: headingFontMetrics.font.underline
                            font.italic: headingFontMetrics.font.italic
                            font.letterSpacing: headingFontMetrics.font.letterSpacing
                            font.capitalization: activeFocus ? (currentLanguage === TransliterationEngine.English ? Font.AllUppercase : Font.MixedCase) : Font.AllUppercase
                            color: headingItem.theElement.omitted ? "gray" : headingFontMetrics.format.textColor
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            background: Item { }
                            onEditingComplete: updateSceneHeading(text)
                            property int previouslyActiveLanguage: TransliterationEngine.English
                            onActiveFocusChanged: {
                                if(activeFocus) {
                                    privateData.changeCurrentIndexTo(headingItem.theElementIndex)
                                    previouslyActiveLanguage = Scrite.app.transliterationEngine.language
                                    const headingFormat = Scrite.document.displayFormat.elementFormat(SceneElement.Heading)
                                    headingFormat.activateDefaultLanguage()
                                } else {
                                    updateSceneHeading(text)
                                    Scrite.app.transliterationEngine.language = previouslyActiveLanguage
                                }
                            }
                            tabItem: headingItem.sceneTextEditor

                            enableTransliteration: true
                            property var currentLanguage: Scrite.app.transliterationEngine.language

                            property int dotPosition: text.indexOf(".")
                            property int dashPosition: text.lastIndexOf("-")
                            property bool editingLocationTypePart: dotPosition < 0 || cursorPosition < dotPosition
                            property bool editingMomentPart: dashPosition > 0 && cursorPosition >= dashPosition
                            property bool editingLocationPart: dotPosition > 0 ? (cursorPosition >= dotPosition && (dashPosition < 0 ? true : cursorPosition < dashPosition)) : false
                            singleClickAutoComplete: screenplayEditorSettings.singleClickAutoComplete
                            completionStrings: {
                                if(editingLocationPart)
                                    return Scrite.document.structure.allLocations()
                                if(editingLocationTypePart)
                                    return Scrite.document.structure.standardLocationTypes()
                                if(editingMomentPart)
                                    return Scrite.document.structure.standardMoments()
                                return []
                            }
                            completionPrefix: {
                                if(editingLocationPart)
                                    return text.substring(dotPosition+1, dashPosition < 0 ? text.length : dashPosition).trim()
                                if(editingLocationTypePart)
                                    return dotPosition < 0 ? text : text.substring(0, dotPosition).trim()
                                if(editingMomentPart)
                                    return text.substring(dashPosition+1).trim()
                                return ""
                            }
                            includeSuggestion: function(suggestion) {
                                if(editingLocationPart || editingLocationTypePart || editingMomentPart) {
                                    var one = editingLocationTypePart ? suggestion : text.substring(0, dotPosition).trim()
                                    var two = editingLocationPart ? suggestion : (dotPosition > 0 ? text.substring(dotPosition+1, dashPosition < 0 ? text.length : dashPosition).trim() : "")
                                    var three = editingMomentPart ? suggestion : (dashPosition < 0 ? "" : text.substring(dashPosition+1).trim())

                                    var cp = 0
                                    if(editingLocationTypePart)
                                        cp = one.length + 2
                                    else if(editingLocationPart)
                                        cp = one.length + 2 + two.length + 3
                                    else if(editingMomentPart)
                                        cp = one.length + two.length + three.length + 2 + 3

                                    Qt.callLater( function() {
                                        sceneHeadingField.cursorPosition = cp
                                    })

                                    var ret = one + ". "
                                    if(two.length > 0 || three.length > 0)
                                        ret += two + " - " + three
                                    return ret
                                }

                                return suggestion
                            }
                        }
                    }

                    ToolButton3 {
                        id: sceneTaggingButton
                        iconSource: "../icons/action/tag.png"
                        visible: screenplayEditorSettings.allowTaggingOfScenes && mainTabBar.currentIndex === 0
                        down: sceneTagMenuLoader.active
                        onClicked: sceneTagMenuLoader.show()
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: sceneMenuButton.left
                        anchors.rightMargin: parent.spacing
                        width: headingFontMetrics.lineSpacing
                        height: headingFontMetrics.lineSpacing
                        ToolTip.text: "Tag Scene"

                        MenuLoader {
                            id: sceneTagMenuLoader
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom

                            menu: StructureGroupsMenu {
                                height: 300
                                sceneGroup: SceneGroup {
                                    structure: Scrite.document.structure
                                }
                                onAboutToShow: {
                                    sceneGroup.clearScenes()
                                    sceneGroup.addScene(headingItem.theScene)
                                }
                                onClosed: sceneGroup.clearScenes()
                            }
                        }
                    }

                    ToolButton3 {
                        id: sceneMenuButton
                        iconSource: "../icons/navigation/menu.png"
                        ToolTip.text: "Click here to view scene options menu."
                        ToolTip.delay: 1000
                        onClicked: sceneMenu.visible = true
                        down: sceneMenu.visible
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        width: headingFontMetrics.lineSpacing
                        height: headingFontMetrics.lineSpacing
                        visible: enabled
                        enabled: !Scrite.document.readOnly

                        Item {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right

                            Menu2 {
                                id: sceneMenu

                                MenuItem2 {
                                    enabled: !headingItem.theElement.omitted
                                    action: Action {
                                        text: "Scene Heading"
                                        checkable: true
                                        checked: headingItem.theScene.heading.enabled
                                    }
                                    onTriggered: {
                                        headingItem.theScene.heading.enabled = action.checked
                                        sceneMenu.close()
                                    }
                                }

                                ColorMenu {
                                    title: "Color"
                                    onMenuItemClicked: {
                                        headingItem.theScene.color = color
                                        sceneMenu.close()
                                    }
                                }

                                MarkSceneAsMenu {
                                    title: "Mark Scene As"
                                    scene: headingItem.theScene
                                    enabled: !headingItem.theElement.omitted
                                }

                                Repeater {
                                    model: headingItem.theElement.omitted ? 0 : additionalSceneMenuItems.length ? 1 : 0

                                    MenuSeparator { }
                                }

                                Repeater {
                                    model: headingItem.theElement.omitted ? 0 : additionalSceneMenuItems

                                    MenuItem2 {
                                        text: modelData
                                        onTriggered: {
                                            Scrite.document.screenplay.currentElementIndex = headingItem.theElementIndex
                                            additionalSceneMenuItemClicked(headingItem.theScene, modelData)
                                        }
                                    }
                                }

                                Repeater {
                                    model: headingItem.theElement.omitted ? 0 : additionalSceneMenuItems.length ? 1 : 0

                                    MenuSeparator { }
                                }

                                MenuItem2 {
                                    text: headingItem.theElement.omitted ? "Include" : "Omit"
                                    enabled: screenplayAdapter.screenplay === Scrite.document.screenplay
                                    onClicked: {
                                        sceneMenu.close()
                                        headingItem.theElement.omitted = !headingItem.theElement.omitted
                                    }
                                }

                                MenuItem2 {
                                    text: "Remove"
                                    enabled: screenplayAdapter.screenplay === Scrite.document.screenplay
                                    onClicked: {
                                        sceneMenu.close()
                                        Scrite.document.screenplay.removeSceneElements(headingItem.theScene)
                                    }
                                }
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: 5*Math.min(zoomLevel,1)
                    visible: sceneCharactersListLoader.active
                }

                Loader {
                    id: sceneCharactersListLoader
                    width: parent.width
                    property bool editorHasActiveFocus: headingItem.sceneHasFocus
                    property int sceneEditorCursorPosition: headingItem.sceneTextEditor.cursorPosition
                    property Scene scene: headingItem.theScene
                    property bool allow: true
                    active: screenplayEditorSettings.displaySceneCharacters && allow
                    sourceComponent: headingItem.theElement.omitted ? null : sceneCharactersList

                    Announcement.onIncoming: (type,data) => {
                        var stype = "" + type
                        var sdata = "" + data
                        if(stype === "E69D2EA0-D26D-4C60-B551-FD3B45C5BE60" && sdata === headingItem.theScene.id) {
                            sceneCharactersListLoader.allow = false
                            Qt.callLater( function() {
                                sceneCharactersListLoader.allow = true
                            })
                        }
                    }

                    function reload() {
                        sceneCharactersListLoader.allow = false
                        Qt.callLater( function() {
                            sceneCharactersListLoader.allow = true
                        })
                    }

                    function reloadLater() {
                        Qt.callLater(reload)
                    }

                    Connections {
                        target: headingItem.theScene
                        function onSceneRefreshed() { sceneCharactersListLoader.reloadLater() }
                    }

                    property int cursorPositionWhenNewCharacterWasAdded: -1
                    Connections {
                        target: sceneCharactersListLoader.item
                        function onNewCharacterAdded(characterName, curPosition) {
                            headingItem.sceneTextEditor.forceActiveFocus()
                            sceneCharactersListLoader.cursorPositionWhenNewCharacterWasAdded = curPosition
                            if(curPosition >= 0) {
                                Utils.execLater(sceneCharactersListLoader, 250, function() {
                                    headingItem.sceneTextEditor.cursorPosition = sceneCharactersListLoader.cursorPositionWhenNewCharacterWasAdded
                                    sceneCharactersListLoader.cursorPositionWhenNewCharacterWasAdded = -1
                                })
                            }
                        }
                    }
                }

                Text {
                    id: sceneGroupTagsText
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pointSize: sceneHeadingFieldsFontPointSize
                    text: sceneCharactersListLoader.active ? Scrite.document.structure.presentableGroupNames(headingItem.theScene.groups) : ""
                    visible: !headingItem.theElement.omitted && sceneCharactersListLoader.active && headingItem.theScene.groups.length > 0
                    topPadding: 5*Math.min(zoomLevel,1)
                    bottomPadding: 5*Math.min(zoomLevel,1)
                    font.underline: sceneGroupTagsTextMouseArea.containsMouse && !sceneTagMenuLoader.active
                    color: font.underline ? "blue" : "black"
                    MouseArea {
                        id: sceneGroupTagsTextMouseArea
                        enabled: sceneTaggingButton.visible
                        hoverEnabled: true
                        anchors.fill: parent
                        onClicked: sceneTagMenuLoader.show()
                    }
                }

                Item {
                    width: parent.width
                    height: 10*Math.min(zoomLevel,1)
                    visible: !headingItem.theElement.omitted && !screenplayEditorSettings.displaySceneSynopsis
                }
            }
        }
    }

    FontMetrics {
        id: defaultFontMetrics
        readonly property SceneElementFormat format: Scrite.document.formatting.elementFormat(SceneElement.Action)
        font: format ? format.font2 : Scrite.document.formatting.defaultFont2
    }

    FontMetrics {
        id: headingFontMetrics
        readonly property SceneElementFormat format: Scrite.document.formatting.elementFormat(SceneElement.Heading)
        font: format.font2
    }

    Component {
        id: sceneCharactersList

        Flow {
            id: sceneCharacterListItem
            spacing: 5
            flow: Flow.LeftToRight

            signal newCharacterAdded(string characterName, int curPosition)

            Text {
                id: sceneCharactersListHeading
                text: "Characters: "
                font.bold: true
                topPadding: 5
                bottomPadding: 5
                font.pointSize: sceneHeadingFieldsFontPointSize
                visible: !scene.hasCharacters
            }

            Repeater {
                model: scene ? scene.characterNames : 0

                TagText {
                    id: characterNameLabel
                    property var colors: {
                        if(containsMouse)
                            return accentColors.c900
                        return editorHasActiveFocus ? accentColors.c600 : accentColors.c10
                    }
                    border.width: editorHasActiveFocus ? 0 : Math.max(0.5, 1 * zoomLevel)
                    border.color: colors.text
                    color: colors.background
                    textColor: colors.text
                    text: modelData
                    topPadding: Math.max(5, 5 * zoomLevel); bottomPadding: topPadding
                    leftPadding: Math.max(10, 10 * zoomLevel); rightPadding: leftPadding
                    font.family: headingFontMetrics.font.family
                    font.capitalization: headingFontMetrics.font.capitalization
                    font.pointSize: sceneHeadingFieldsFontPointSize
                    closable: scene.isCharacterMute(modelData) && !Scrite.document.readOnly
                    onClicked: requestCharacterMenu(modelData, characterNameLabel)
                    onCloseRequest: {
                        if(!Scrite.document.readOnly)
                            scene.removeMuteCharacter(modelData)
                    }
                }
            }

            Loader {
                id: newCharacterInput
                width: active && item ? Math.max(item.contentWidth, 100) : 0
                active: false
                property int sceneTextEditorCursorPosition: -1
                onActiveChanged: {
                    if(!active)
                        Qt.callLater( function() { newCharacterInput.sceneTextEditorCursorPosition = -1 } )
                }
                sourceComponent: Item {
                    property alias contentWidth: textViewEdit.contentWidth
                    height: textViewEdit.height
                    Component.onCompleted: contentView.ensureVisible(newCharacterInput, Qt.rect(0,0,width,height))

                    TextViewEdit {
                        id: textViewEdit
                        width: parent.width
                        y: fontDescent
                        readOnly: false
                        font.family: headingFontMetrics.font.family
                        font.capitalization: headingFontMetrics.font.capitalization
                        font.pointSize: sceneHeadingFieldsFontPointSize
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.NoWrap
                        completionStrings: Scrite.document.structure.characterNames
                        onEditingFinished: {
                            scene.addMuteCharacter(text)
                            sceneCharacterListItem.newCharacterAdded(text, newCharacterInput.sceneTextEditorCursorPosition)
                            newCharacterInput.active = false
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: parent.fontHeight - parent.fontAscent - parent.fontHeight*0.25
                            height: 1
                            color: accentColors.borderColor
                        }
                    }
                }
            }

            Image {
                source: "../icons/content/add_box.png"
                width: sceneCharactersListHeading.height
                height: width
                opacity: 0.5
                visible: enabled
                enabled: !Scrite.document.readOnly

                MouseArea {
                    ToolTip.text: "Click here to capture characters who don't have any dialogues in this scene, but are still required for the scene."
                    ToolTip.delay: 1000
                    ToolTip.visible: containsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                    onClicked: newCharacterInput.active = true
                }

                Announcement.onIncoming: (type,data) => {
                    if(!editorHasActiveFocus)
                        return

                    var sdata = "" + data
                    var stype = "" + type
                    if(stype === "2E3BBE4F-05FE-49EE-9C0E-3332825B72D8" && sdata === "Add Mute Character") {
                        newCharacterInput.sceneTextEditorCursorPosition = sceneEditorCursorPosition
                        newCharacterInput.active = true
                    }
                }
            }
        }
    }

    Item {
        id: sidePanels
        anchors.top: screenplayEditorWorkspace.top
        anchors.left: parent.left
        anchors.bottom: statusBar.top
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        width: sceneListSidePanel.visible ? sceneListSidePanel.width : 0
        property bool expanded: sceneListSidePanel.expanded
        onExpandedChanged: contentView.commentsExpandCounter = 0

        SidePanel {
            id: sceneListSidePanel
            height: parent.height
            buttonY: 20
            label: ""
            z: expanded ? 1 : 0

            content: Item {
                Text {
                    id: dragHotspotItem
                    font.family: "Courier Prime"
                    font.pixelSize: Scrite.app.idealFontPointSize + 4
                    visible: false
                }

                Text {
                    width: parent.width * 0.9
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: Scrite.app.idealFontPointSize
                    text: "Scene headings will be listed here as you add them into your screenplay."
                    anchors.horizontalCenter: sceneListView.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 50
                    visible: screenplayAdapter.elementCount === 0
                }

                Connections {
                    target: Scrite.document.screenplay
                    enabled: screenplayAdapter.isSourceScreenplay
                    function onElementMoved(element, from, to) {
                        Qt.callLater(sceneListView.forceLayout)
                    }
                }

                QtObject {
                    EventFilter.target: Scrite.app
                    EventFilter.active: screenplayAdapter.isSourceScreenplay && Scrite.document.screenplay.hasSelectedElements
                    EventFilter.events: [EventFilter.KeyPress]
                    EventFilter.onFilter: (object,event,result) => {
                                              if(event.key === Qt.Key_Escape) {
                                                  Scrite.document.screenplay.clearSelection()
                                                  result.acceptEvent = true
                                                  result.filter = true
                                              }
                                          }
                }

                ListView {
                    id: sceneListView
                    anchors.fill: parent
                    clip: true
                    model: screenplayAdapter
                    currentIndex: screenplayAdapter.currentIndex
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: ScrollBar2 { flickable: sceneListView }
                    highlightFollowsCurrentItem: true
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    keyNavigationEnabled: false
                    preferredHighlightEnd: height*0.8
                    preferredHighlightBegin: height*0.2
                    highlightRangeMode: ListView.NoHighlightRange
                    property bool hasEpisodes: screenplayAdapter.isSourceScreenplay ? screenplayAdapter.screenplay.episodeCount > 0 : false

                    FocusTracker.window: Scrite.window
                    FocusTracker.indicator.target: mainUndoStack
                    FocusTracker.indicator.property: "sceneListPanelActive"

                    header: Rectangle {
                        width: sceneListView.width-1
                        height: 40
                        color: screenplayAdapter.currentIndex < 0 ? accentColors.windowColor : Qt.rgba(0,0,0,0)

                        Text {
                            id: headingText
                            readonly property real iconWidth: 18
                            property real t: screenplayAdapter.hasNonStandardScenes ? 1 : 0
                            property real leftMargin: 6 + (iconWidth+12)*t
                            Behavior on t {
                                enabled: applicationSettings.enableAnimations
                                NumberAnimation { duration: 250 }
                            }

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                            anchors.right: parent.right
                            elide: Text.ElideRight
                            font.family: "Courier Prime"
                            font.pixelSize: Math.ceil(Scrite.app.idealFontPointSize * 1.2)
                            font.bold: true
                            text: Scrite.document.screenplay.title === "" ? "[#] TITLE PAGE" : Scrite.document.screenplay.title
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: headingText.truncated
                            ToolTip.text: headingText.text
                            ToolTip.delay: 1000
                            ToolTip.visible: headingText.truncated && containsMouse
                            onClicked: {
                                if(screenplayAdapter.isSourceScreenplay)
                                    screenplayAdapter.screenplay.clearSelection()
                                screenplayAdapter.currentIndex = -1
                                contentView.positionViewAtBeginning()
                            }
                        }
                    }

                    delegate: Rectangle {
                        id: delegateItem
                        width: sceneListView.width-1
                        height: 40
                        color: scene ? (screenplayAdapter.currentIndex === index || screenplayElement.selected) ? selectedColor : normalColor
                                     : screenplayAdapter.currentIndex === index ? Scrite.app.translucent(accentColors.windowColor, 0.25) : Qt.rgba(0,0,0,0.01)

                        property color selectedColor: Scrite.app.isVeryLightColor(scene.color) ? Qt.tint(primaryColors.highlight.background, "#9CFFFFFF") : Qt.tint(scene.color, "#9CFFFFFF")
                        property color normalColor: Qt.tint(scene.color, "#E7FFFFFF")
                        property int elementIndex: index
                        property bool elementIsBreak: screenplayElementType === ScreenplayElement.BreakElementType
                        property bool elementIsEpisodeBreak: screenplayElementType === ScreenplayElement.BreakElementType && breakType === Screenplay.Episode

                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            visible: screenplayAdapter.currentIndex === index
                            width: 8
                            color: accentColors.windowColor
                        }

                        SceneTypeImage {
                            id: sceneTypeImage
                            width: 18
                            height: 18
                            showTooltip: false
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            sceneType: scene ? scene.type : Scene.Standard
                            opacity: (screenplayAdapter.currentIndex === index ? 1 : 0.5) * t
                            visible: t > 0
                            lightBackground: Scrite.app.isLightColor(delegateItem.color)
                            property real t: screenplayAdapter.hasNonStandardScenes ? 1 : 0
                            Behavior on t {
                                enabled: applicationSettings.enableAnimations
                                NumberAnimation { duration: 250 }
                            }
                        }

                        Text {
                            id: delegateText
                            property real leftMargin: 11 + (sceneTypeImage.width+12)*sceneTypeImage.t
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                            anchors.right: parent.right
                            anchors.rightMargin: (sceneListView.contentHeight > sceneListView.height ? sceneListView.ScrollBar.vertical.width : 5) + 5
                            anchors.verticalCenter: parent.verticalCenter
                            font.family: "Courier Prime"
                            font.bold: screenplayAdapter.currentIndex === index || parent.elementIsBreak
                            font.pointSize: Math.ceil(Scrite.app.idealFontPointSize*(parent.elementIsBreak ? 1.2 : 1))
                            horizontalAlignment: Qt.AlignLeft
                            color: primaryColors.c10.text
                            font.capitalization: parent.elementIsBreak ? Font.MixedCase : Font.AllUppercase
                            text: {
                                var ret = ""
                                if(scene && scene.heading.enabled) {
                                    ret = screenplayElement.resolvedSceneNumber + ". "
                                    if(screenplayElement.omitted)
                                        ret += "[OMITTED] <font color=\"gray\">" + scene.heading.text + "</font>"
                                    else
                                        ret += scene.heading.text
                                    return ret
                                }
                                if(parent.elementIsBreak) {
                                    if(parent.elementIsEpisodeBreak)
                                        ret = screenplayElement.breakTitle
                                    else if(sceneListView.hasEpisodes)
                                        ret = "Ep " + (screenplayElement.episodeIndex+1) + ": " + screenplayElement.breakTitle
                                    else
                                        ret = screenplayElement.breakTitle
                                    ret +=  ": " + screenplayElement.breakSubtitle
                                    return ret
                                }
                                return "NO SCENE HEADING"
                            }
                            elide: Text.ElideMiddle
                        }

                        MouseArea {
                            id: delegateMouseArea
                            hoverEnabled: delegateText.truncated
                            ToolTip.text: delegateText.text
                            ToolTip.delay: 1000
                            ToolTip.visible: delegateText.truncated && containsMouse
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onDoubleClicked: (mouse) => {
                                                 screenplayAdapter.screenplay.clearSelection()
                                                 screenplayElement.toggleSelection()
                                                 screenplayAdapter.currentIndex = index
                                                 sceneListSidePanel.expanded = false
                                             }
                            onClicked: (mouse) => {
                                           if(mouse.button === Qt.RightButton) {
                                               if(screenplayElement.elementType === ScreenplayElement.BreakElementType) {
                                                   breakElementContextMenu.element = screenplayElement
                                                   breakElementContextMenu.popup(this)
                                               } else {
                                                   sceneElementsContextMenu.element = screenplayElement
                                                   sceneElementsContextMenu.popup(this)
                                               }

                                               Scrite.document.screenplay.currentElementIndex = index
                                               return
                                           }

                                           if(screenplayAdapter.isSourceScreenplay) {
                                               const isControlPressed = mouse.modifiers & Qt.ControlModifier
                                               const isShiftPressed = mouse.modifiers & Qt.ShiftModifier
                                               if(isControlPressed) {
                                                   screenplayElement.toggleSelection()
                                               } else if(isShiftPressed) {
                                                   const fromIndex = Math.min(screenplayAdapter.currentIndex, index)
                                                   const toIndex = Math.max(screenplayAdapter.currentIndex, index)
                                                   if(fromIndex === toIndex) {
                                                       screenplayElement.toggleSelection()
                                                   } else {
                                                       for(var i=fromIndex; i<=toIndex; i++) {
                                                           var element = screenplayAdapter.screenplay.elementAt(i)
                                                           if(element.elementType === ScreenplayElement.SceneElementType) {
                                                               element.selected = true
                                                           }
                                                       }
                                                   }
                                               } else {
                                                   screenplayAdapter.screenplay.clearSelection()
                                                   screenplayElement.toggleSelection()
                                               }
                                           }

                                           screenplayAdapter.currentIndex = index
                                       }
                            drag.target: screenplayAdapter.isSourceScreenplay && !Scrite.document.readOnly ? parent : null
                            drag.axis: Drag.YAxis
                        }

                        Drag.active: delegateMouseArea.drag.active
                        Drag.dragType: Drag.Automatic
                        Drag.supportedActions: Qt.MoveAction
                        Drag.hotSpot.x: dragHotspotItem.width/2
                        Drag.hotSpot.y: dragHotspotItem.height/2
                        Drag.source: screenplayElement
                        Drag.mimeData: {
                            "sceneListView/sceneID": screenplayElement.sceneID
                        }
                        Drag.onActiveChanged: {
                            if(!screenplayElement.selected)
                               Scrite.document.screenplay.clearSelection()
                            screenplayElement.selected = true
                            moveSelectedElementsAnimation.draggedElement = screenplayElement
                            if(screenplayElementType === ScreenplayElement.BreakElementType)
                                Scrite.document.screenplay.currentElementIndex = index
                        }
                        Drag.onDragFinished: sceneListView.forceLayout()

                        DropArea {
                            id: delegateDropArea
                            anchors.fill: parent
                            keys: ["sceneListView/sceneID"]
                            enabled: !screenplayElement.selected

                            onEntered: (drag) => {
                                           drag.acceptProposedAction()
                                           sceneListView.forceActiveFocus()
                                       }

                            onDropped: (drop) => {
                                           moveSelectedElementsAnimation.targetIndex = delegateItem.elementIndex
                                           drop.acceptProposedAction()
                                       }
                        }

                        Rectangle {
                            id: dropIndicator
                            width: parent.width
                            height: 2
                            color: primaryColors.borderColor
                            visible: delegateDropArea.containsDrag
                        }

                        Rectangle {
                            anchors.top: dropIndicator.visible ? dropIndicator.bottom : parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 1
                            color: parent.elementIsEpisodeBreak ? accentColors.c200.background : accentColors.c100.background
                            visible: parent.elementIsBreak
                        }
                    }

                    footer: Item {
                        width: sceneListView.width-1
                        height: 40

                        DropArea {
                            id: footerDropArea
                            anchors.fill: parent
                            keys: ["sceneListView/sceneID"]

                            onEntered: (drag) => {
                                           drag.acceptProposedAction()
                                           sceneListView.forceActiveFocus()
                                       }

                            onDropped: (drop) => {
                                           drop.acceptProposedAction()
                                           moveSelectedElementsAnimation.targetIndex = screenplayAdapter.elementCount
                                       }
                        }

                        Rectangle {
                            width: parent.width
                            height: 2
                            color: primaryColors.borderColor
                            visible: footerDropArea.containsDrag
                        }
                    }

                    SequentialAnimation {
                        id: moveSelectedElementsAnimation

                        property int targetIndex: -1
                        property ScreenplayElement draggedElement
                        onTargetIndexChanged: {
                            if(targetIndex >= 0)
                                start()
                        }

                        PauseAnimation { duration: 50 }

                        ScriptAction {
                            script: {
                                Scrite.document.screenplay.moveSelectedElements(moveSelectedElementsAnimation.targetIndex)
                                Qt.callLater(sceneListView.forceLayout)
                            }
                        }

                        PauseAnimation { duration: 50 }

                        ScriptAction {
                            script: sceneListView.forceLayout()
                        }

                        PauseAnimation { duration: 50 }

                        ScriptAction {
                            script: {
                                const draggedElement = moveSelectedElementsAnimation.draggedElement
                                const targetndex = draggedElement ? Scrite.document.screenplay.indexOfElement(draggedElement) : moveSelectedElementsAnimation.targetIndex

                                moveSelectedElementsAnimation.targetIndex = -1
                                moveSelectedElementsAnimation.draggedElement = null

                                contentView.positionViewAtIndex(targetndex, ListView.Beginning)
                                privateData.changeCurrentIndexTo(targetndex)

                                sceneListView.forceActiveFocus()
                            }
                        }
                    }
                }

                ScreenplayBreakElementsContextMenu {
                    id: breakElementContextMenu
                }

                ScreenplaySceneElementsContextMenu {
                    id: sceneElementsContextMenu
                }
            }
        }
    }

    function requestCharacterMenu(characterName, popupSource) {
        if(characterMenu.characterReports.length === 0) {
            var reports = Scrite.document.supportedReports
            var chReports = []
            reports.forEach( function(item) {
                if(item.name.indexOf('Character') >= 0)
                    chReports.push(item)
            })
            characterMenu.characterReports = chReports
        }

        characterMenu.popupSource = popupSource
        characterMenu.characterName = characterName
        characterMenu.popup()
    }

    Menu2 {
        id: characterMenu
        width: 300
        property Item popupSource
        property string characterName
        property var characterReports: []

        Repeater {
            model: characterMenu.characterReports

            MenuItem2 {
                leftPadding: 15
                rightPadding: 15
                topPadding: 5
                bottomPadding: 5
                width: reportsMenu.width
                height: 65
                contentItem: Column {
                    id: menuContent
                    width: characterMenu.width - 30
                    spacing: 5

                    Text {
                        font.bold: true
                        font.pixelSize: 16
                        text: modelData.name
                    }

                    Text {
                        text: modelData.description
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                        font.italic: true
                    }
                }

                onTriggered: {
                    reportGeneratorTimer.requestSource = this
                    reportGeneratorTimer.reportArgs = {"reportName": modelData.name, "configuration": {"characterNames": [characterMenu.characterName]}}
                    characterMenu.close()
                    characterMenu.characterName = ""
                }
            }
        }

        Repeater {
            model: characterMenu.characterReports.length > 0 ? additionalCharacterMenuItems : []

            MenuItem2 {
                leftPadding: 15
                rightPadding: 15
                topPadding: 5
                bottomPadding: 5
                width: reportsMenu.width
                height: 65
                contentItem: Column {
                    width: characterMenu.width - 30
                    spacing: 5

                    Text {
                        font.bold: true
                        font.pixelSize: 16
                        text: modelData.name
                    }

                    Text {
                        text: modelData.description
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                        font.italic: true
                    }
                }

                onTriggered: additionalCharacterMenuItemClicked(characterMenu.characterName, modelData.name)
            }
        }

        Repeater {
            model: characterMenu.characterReports.length > 0 ? 1 : 0

            MenuItem2 {
                leftPadding: 15
                rightPadding: 15
                topPadding: 5
                bottomPadding: 5
                width: reportsMenu.width
                height: 65
                contentItem: Column {
                    width: characterMenu.width - 30
                    spacing: 5

                    Text {
                        font.bold: true
                        font.pixelSize: 16
                        text: "Rename Character"
                    }

                    Text {
                        text: "Rename character across all scenes, notes, comments, titles and descriptions."
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                        font.italic: true
                    }
                }

                onTriggered: {
                    const character = Scrite.document.structure.addCharacter(characterMenu.characterName)
                    if(character) {
                        modalDialog.popupSource = characterMenu.popupSource
                        modalDialog.arguments = character
                        modalDialog.sourceComponent = renameCharacterDialog
                        modalDialog.active = true
                        characterMenu.close()
                    }
                }
            }
        }
    }

    Component {
        id: renameCharacterDialog

        RenameCharacterDialog { }
    }

    Component {
        id: titleCardComponent

        Item {
            readonly property int defaultFontSize: screenplayFormat.defaultFont2.pointSize
            readonly property real leftPadding: (ruler.leftMarginPx+ruler.rightMarginPx)/2
            readonly property real rightPadding: leftPadding
            height: titleCardContents.height

            signal editTitlePageRequest(Item sourceItem)

            Column {
                id: titleCardContents
                spacing: 10 * zoomLevel
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: parent.leftPadding
                anchors.rightMargin: parent.rightPadding

                Item { width: parent.width; height: 35 * zoomLevel }

                Image {
                    id: coverPicImage

                    width: {
                        switch(Scrite.document.screenplay.coverPagePhotoSize) {
                        case Screenplay.SmallCoverPhoto:
                            return parent.width / 4
                        case Screenplay.MediumCoverPhoto:
                            return parent.width / 2
                        }
                        return parent.width
                    }

                    cache: false
                    source: visible ? "file:///" + Scrite.document.screenplay.coverPagePhoto : ""
                    visible: Scrite.document.screenplay.coverPagePhoto !== ""
                    smooth: true; mipmap: true; asynchronous: true
                    fillMode: Image.PreserveAspectFit
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: -border.width - 4
                        color: Qt.rgba(1,1,1,0.1)
                        border { width: 2; color: titleLink.hoverColor }
                        visible: coverPicMouseArea.containsMouse
                    }

                    MouseArea {
                        id: coverPicMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: editTitlePageRequest(coverPicMouseArea)
                    }
                }

                Item { width: parent.width; height: Scrite.document.screenplay.coverPagePhoto !== "" ? 20 * zoomLevel : 0 }

                Link {
                    id: titleLink
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize + 2
                    font.underline: containsMouse
                    font.bold: true
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: Scrite.document.screenplay.title === "" ? "<untitled>" : Scrite.document.screenplay.title
                    onClicked: editTitlePageRequest(titleLink)
                }

                Link {
                    id: subtitleLink
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize
                    font.underline: containsMouse
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: Scrite.document.screenplay.subtitle
                    visible: Scrite.document.screenplay.subtitle !== ""
                    onClicked: editTitlePageRequest(subtitleLink)
                }

                Column {
                    width: parent.width
                    spacing: 0

                    Text {
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        horizontalAlignment: Text.AlignHCenter
                        text: "Written By"
                    }

                    Link {
                        id: authorsLink
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize
                        font.underline: containsMouse
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        horizontalAlignment: Text.AlignHCenter
                        text: (Scrite.document.screenplay.author === "" ? "<unknown author>" : Scrite.document.screenplay.author)
                        onClicked: editTitlePageRequest(authorsLink)
                    }
                }

                Link {
                    id: versionLink
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize
                    font.underline: containsMouse
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: Scrite.document.screenplay.version
                    onClicked: editTitlePageRequest(versionLink)
                }

                Link {
                    id: basedOnLink
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize
                    font.underline: containsMouse
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: Scrite.document.screenplay.basedOn
                    visible: Scrite.document.screenplay.basedOn !== ""
                    onClicked: editTitlePageRequest(basedOnLink)
                }

                Column {
                    spacing: parent.spacing/2
                    width: parent.width * 0.5
                    anchors.left: parent.left

                    Item {
                        width: parent.width
                        height: 20 * zoomLevel
                    }

                    Link {
                        id: contactLink
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize - 2
                        font.underline: containsMouse
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: Scrite.document.screenplay.contact
                        visible: text !== ""
                        onClicked: editTitlePageRequest(contactLink)
                    }

                    Link {
                        id: addressLink
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize - 2
                        font.underline: containsMouse
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: Scrite.document.screenplay.address
                        visible: text !== ""
                        onClicked: editTitlePageRequest(addressLink)
                    }

                    Link {
                        id: phoneNumberLink
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize - 2
                        font.underline: containsMouse
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: Scrite.document.screenplay.phoneNumber
                        visible: text !== ""
                        onClicked: editTitlePageRequest(phoneNumberLink)
                    }

                    Link {
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize - 2
                        font.underline: containsMouse
                        color: "blue"
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: Scrite.document.screenplay.email
                        visible: text !== ""
                        onClicked: Qt.openUrlExternally("mailto:" + text)
                    }

                    Link {
                        font.family: Scrite.document.formatting.defaultFont.family
                        font.pointSize: defaultFontSize - 2
                        font.underline: containsMouse
                        color: "blue"
                        width: parent.width
                        elide: Text.ElideRight
                        text: Scrite.document.screenplay.website
                        visible: text !== ""
                        onClicked: Qt.openUrlExternally(text)
                    }
                }

                Item { width: parent.width; height: 35 * zoomLevel }
            }
        }

    }

    Connections {
        target: Scrite.document
        function onAboutToSave() { saveLayoutDetails() }
        function onJustLoaded() { restoreLayoutDetails() }
    }

    Component.onCompleted: {
        restoreLayoutDetails()
        screenplayTextDocument.editor = screenplayEditor
        if(mainTabBar.currentIndex === 0)
            Scrite.user.logActivity1("screenplay")
    }
    Component.onDestruction: {
        saveLayoutDetails()
    }

    function saveLayoutDetails() {
        if(sceneListSidePanel.visible) {
            var userData = Scrite.document.userData
            userData["screenplayEditor"] = {
                "version": 0,
                "sceneListSidePanelExpaned": sceneListSidePanel.expanded
            }
            Scrite.document.userData = userData
        }
    }

    function restoreLayoutDetails() {
        if(sceneListSidePanel.visible) {
            var userData = Scrite.document.userData
            if(userData.screenplayEditor && userData.screenplayEditor.version === 0)
                sceneListSidePanel.expanded = userData.screenplayEditor.sceneListSidePanelExpaned
        }
    }

    property real sceneHeadingFieldsFontPointSize: Math.max(headingFontMetrics.font.pointSize*0.7, 6)

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0,0,0,0)
        border.width: 1
        border.color: primaryColors.borderColor
    }

    Connections {
        target: modalDialog
        function onActiveChanged() {
            if(modalDialog.active === false && mainTabBar.currentIndex === 0 && contentView.count === 1)
                contentView.itemAtIndex(0).item.assumeFocus()
        }
    }

    Connections {
        target: splashLoader
        function onActiveChanged() {
            if(splashLoader.active === false && mainTabBar.currentIndex === 0 && contentView.count === 1)
                contentView.itemAtIndex(0).item.assumeFocus()
        }
    }

    Component {
        id: contentViewDelegateComponent

        Loader {
            id: contentViewDelegateLoader
            property var componentData: modelData
            property int componentIndex: index
            z: contentViewModel.value.currentIndex === index ? 2 : 1
            width: contentView.width
            onComponentDataChanged: {
                if(componentData === undefined)
                    active = false
            }

            active: false
            onActiveChanged: Scrite.app.resetObjectProperty(contentViewDelegateLoader, "height")
            sourceComponent: {
                if(componentData) {
                    if(componentData.scene)
                        return componentData.screenplayElement.omitted ? omittedContentComponent : contentComponent
                    if(componentData.breakType === Screenplay.Episode)
                        return episodeBreakComponent
                    return actBreakComponent
                }
                return noContentComponent
            }
            onLoaded: {
                contentView.delegateWasLoaded()
                Qt.callLater(updateHeightHint)
                if(placeHolderSceneItem)
                    placeHolderSceneItem.destroy()
            }
            onHeightChanged: Qt.callLater(updateHeightHint)

            // Background for episode and act break components, when "Scene Blocks" is enabled.
            Rectangle {
                z: -1
                anchors.fill: parent
                anchors.leftMargin: -1
                anchors.rightMargin: -1
                anchors.topMargin: componentData.scene ? -1 : -contentView.spacing/2
                anchors.bottomMargin: componentData.scene ? -1 : -contentView.spacing/2
                visible: contentView.spacing > 0
                color: componentData.scene ? Qt.rgba(0,0,0,0) : (componentData.breakType === Screenplay.Episode ? accentColors.c100.background : accentColors.c50.background)
                border.width: componentData.scene ? 1 : 0
                border.color: componentData.scene ? (Scrite.app.isLightColor(componentData.scene.color) ? "black" : componentData.scene.color) : Qt.rgba(0,0,0,0)
                opacity: componentData.scene ? 0.25 : 1
            }

            property Item placeHolderSceneItem
            property bool initialized: false
            property bool isVisibleToUser: !contentView.moving && initialized && (index >= contentView.firstItemIndex && index <= contentView.lastItemIndex) && !contentView.ScrollBar.vertical.active
            onIsVisibleToUserChanged: {
                if(!active && isVisibleToUser)
                    Utils.execLater(contentViewDelegateLoader, 100, load)
            }

            function load() {
                if(active || componentData === undefined)
                    return

                contentView.movingChanged.disconnect(load)
                if(contentView.moving)
                    contentView.movingChanged.connect(load)
                else
                    active = true
            }

            Component.onCompleted: {
                const heightHint = componentData.screenplayElement.heightHint
                if( componentData.screenplayElementType === ScreenplayElement.BreakElementType ||
                    contentView.loadAllDelegates || screenplayEditorSettings.optimiseScrolling ||
                    componentData.scene.elementCount <= 1) {
                        active = true
                        initialized = true
                        return
                    }

                const placeHolderSceneProps = {
                    "spElementIndex": componentIndex,
                    "spElementData": componentData,
                    "spElementType": screenplayElementType,
                    "evaluateSuggestedSceneHeight": heightHint === 0
                };
                placeHolderSceneItem = placeholderSceneComponent.createObject(contentViewDelegateLoader, placeHolderSceneProps)
                placeHolderSceneItem.anchors.fill = contentViewDelegateLoader

                if(heightHint === 0)
                    height = Qt.binding( () => { return placeHolderSceneItem.suggestedSceneHeight } )
                else
                    height = heightHint * zoomLevel

                active = false
                initialized = true
                Utils.execLater(contentViewDelegateLoader, 100, () => { contentViewDelegateLoader.load() } )
            }

            Component.onDestruction: {
                if(!active || componentData.screenplayElementType === ScreenplayElement.BreakElementType)
                    return

                updateHeightHint()
            }

            function updateHeightHint() {
                if(zoomLevel > 0 && active)
                    componentData.screenplayElement.heightHint = height / zoomLevel
            }

            /*
            Profiler.context: "ScreenplayEditorContentDelegate"
            Profiler.active: true
            onStatusChanged: {
                if(status === Loader.Ready)
                    Profiler.active = false
            }
            */
        }
    }

    DockWidget {
        id: markupTools
        property SceneDocumentBinder sceneDocumentBinder
        property TextFormat textFormat: sceneDocumentBinder ? sceneDocumentBinder.textFormat : null
        property SceneElement sceneElement: sceneDocumentBinder ? sceneDocumentBinder.currentElement : null
        contentX: 20
        contentY: 20
        contentPadding: 20
        contentWidth: 426
        contentHeight: 70
        titleBarHeight: 32
        title: "Markup Tools"
        anchors.fill: parent
        closable: true
        visible: false
        onCloseRequest: screenplayEditorSettings.markupToolsDockVisible = false

        function adjustCoordinates() {
            const cx = markupToolsSettings.contentX
            const cy = markupToolsSettings.contentY
            contentX = Math.round(Math.min(Math.max(20, cx), parent.width-contentWidth-20))
            contentY = Math.round(Math.min(Math.max(20, cy), parent.height-contentHeight-20))
            visible = Qt.binding( () => { return screenplayEditorSettings.markupToolsDockVisible } )
        }

        Component.onCompleted: Utils.execLater(markupTools, 200, adjustCoordinates)
        Component.onDestruction: {
            markupToolsSettings.contentX = Math.round(contentX)
            markupToolsSettings.contentY = Math.round(contentY)
        }

        Settings {
            id: markupToolsSettings
            fileName: Scrite.app.settingsFilePath
            category: "Markup Tools"
            property real contentX: 20
            property real contentY: 20
        }

        Shortcut {
            sequence: "Ctrl+B"
            context: Qt.ApplicationShortcut
            enabled: markupTools.textFormat
            ShortcutsModelItem.title: "Bold"
            ShortcutsModelItem.shortcut: sequence
            ShortcutsModelItem.group: "Markup Tools"
            ShortcutsModelItem.enabled: enabled
            onActivated: markupTools.textFormat.toggleBold()
        }

        Shortcut {
            sequence: "Ctrl+I"
            context: Qt.ApplicationShortcut
            enabled: markupTools.textFormat
            ShortcutsModelItem.title: "Italics"
            ShortcutsModelItem.shortcut: sequence
            ShortcutsModelItem.group: "Markup Tools"
            ShortcutsModelItem.enabled: enabled
            onActivated: markupTools.textFormat.toggleItalics()
        }

        Shortcut {
            sequence: "Ctrl+U"
            context: Qt.ApplicationShortcut
            enabled: markupTools.textFormat
            ShortcutsModelItem.title: "Underline"
            ShortcutsModelItem.shortcut: sequence
            ShortcutsModelItem.group: "Markup Tools"
            ShortcutsModelItem.enabled: enabled
            onActivated: markupTools.textFormat.toggleUnderline()
        }

        Shortcut {
            sequence: "Shift+F3"
            context: Qt.ApplicationShortcut
            enabled: markupTools.sceneDocumentBinder
            ShortcutsModelItem.title: "All CAPS"
            ShortcutsModelItem.shortcut: sequence
            ShortcutsModelItem.group: "Markup Tools"
            ShortcutsModelItem.enabled: enabled
            onActivated: markupTools.sceneDocumentBinder.changeCase(SceneDocumentBinder.UpperCase)
        }

        Shortcut {
            sequence: "Ctrl+Shift+F3"
            context: Qt.ApplicationShortcut
            enabled: markupTools.sceneDocumentBinder
            ShortcutsModelItem.title: "All small"
            ShortcutsModelItem.shortcut: sequence
            ShortcutsModelItem.group: "Markup Tools"
            ShortcutsModelItem.enabled: enabled
            onActivated: markupTools.sceneDocumentBinder.changeCase(SceneDocumentBinder.LowerCase)
        }

        content: Rectangle {
            id: toolsContainer

            Row {
                id: toolsLayout
                anchors.centerIn: parent
                spacing: 2
                height: 48
                enabled: !Scrite.document.readOnly

                SimpleToolButton {
                    iconSource: "../icons/editor/format_bold.png"
                    enabled: markupTools.textFormat
                    checked: markupTools.textFormat ? markupTools.textFormat.bold : false
                    onClicked: if(markupTools.textFormat) markupTools.textFormat.toggleBold()
                    anchors.verticalCenter: parent.verticalCenter
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Bold\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+B")
                }

                SimpleToolButton {
                    iconSource: "../icons/editor/format_italics.png"
                    checked: markupTools.textFormat ? markupTools.textFormat.italics : false
                    enabled: markupTools.textFormat
                    onClicked: if(markupTools.textFormat) markupTools.textFormat.toggleItalics()
                    anchors.verticalCenter: parent.verticalCenter
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Italics\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+I")
                }

                SimpleToolButton {
                    iconSource: "../icons/editor/format_underline.png"
                    checked: markupTools.textFormat ? markupTools.textFormat.underline : false
                    enabled: markupTools.textFormat
                    onClicked: if(markupTools.textFormat) markupTools.textFormat.toggleUnderline()
                    anchors.verticalCenter: parent.verticalCenter
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Underline\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+U")
                }

                ColorButton {
                    id: textColorButton
                    anchors.verticalCenter: parent.verticalCenter
                    selectedColor: markupTools.textFormat ? markupTools.textFormat.textColor : transparentColor
                    enabled: markupTools.textFormat
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Text Color"
                    onColorPicked: (newColor) => {
                                       if(markupTools.textFormat)
                                            markupTools.textFormat.textColor = newColor
                                   }

                    Rectangle {
                        color: "white"
                        width: Math.min(parent.width,parent.height) * 0.8
                        height: width
                        anchors.centerIn: parent

                        Text {
                            anchors.centerIn: parent
                            font.pixelSize: parent.height * 0.70
                            font.bold: true
                            font.underline: true
                            text: "A"
                            color: textColorButton.selectedColor === transparentColor ? "black" : textColorButton.selectedColor
                        }
                    }
                }

                ColorButton {
                    id: bgColorButton
                    anchors.verticalCenter: parent.verticalCenter
                    selectedColor: markupTools.textFormat ? markupTools.textFormat.backgroundColor : transparentColor
                    enabled: markupTools.textFormat
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Background Color"
                    onColorPicked: (newColor) => {
                                       if(markupTools.textFormat)
                                            markupTools.textFormat.backgroundColor = newColor
                                   }

                    Rectangle {
                        border.width: 1
                        border.color: "black"
                        color: bgColorButton.selectedColor === transparentColor ? "white" : bgColorButton.selectedColor
                        width: Math.min(parent.width,parent.height) * 0.8
                        height: width
                        anchors.centerIn: parent

                        Text {
                            anchors.centerIn: parent
                            font.pixelSize: parent.height * 0.70
                            font.bold: true
                            text: "A"
                            color: textColorButton.selectedColor === transparentColor ? "black" : textColorButton.selectedColor
                        }
                    }
                }

                SimpleToolButton {
                    iconSource: "../icons/editor/format_clear.png"
                    checked: false
                    enabled: markupTools.textFormat
                    onClicked: if(markupTools.textFormat) markupTools.textFormat.reset()
                    anchors.verticalCenter: parent.verticalCenter
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Clear formatting"
                }

                Rectangle {
                    width: 1
                    height: parent.height
                    color: primaryColors.borderColor
                }

                SimpleToolButton {
                    iconSource: "../icons/editor/format_align_left.png"
                    checked: enabled ? markupTools.sceneElement.alignment === Qt.AlignLeft : false
                    enabled: markupTools.sceneElement && markupTools.sceneElement.type === SceneElement.Action
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: markupTools.sceneElement.alignment = markupTools.sceneElement.alignment === Qt.AlignLeft ? 0 : Qt.AlignLeft
                }

                SimpleToolButton {
                    iconSource: "../icons/editor/format_align_center.png"
                    checked: enabled ? markupTools.sceneElement.alignment === Qt.AlignHCenter : false
                    enabled: markupTools.sceneElement && markupTools.sceneElement.type === SceneElement.Action
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: markupTools.sceneElement.alignment = markupTools.sceneElement.alignment === Qt.AlignHCenter ? 0 : Qt.AlignHCenter
                }

                SimpleToolButton {
                    iconSource: "../icons/editor/format_align_right.png"
                    checked: enabled ? markupTools.sceneElement.alignment === Qt.AlignRight : false
                    enabled: markupTools.sceneElement && markupTools.sceneElement.type === SceneElement.Action
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: markupTools.sceneElement.alignment = markupTools.sceneElement.alignment === Qt.AlignRight ? 0 : Qt.AlignRight
                }

                Rectangle {
                    width: 1
                    height: parent.height
                    color: primaryColors.borderColor
                }

                SimpleToolButton {
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: markupTools.sceneDocumentBinder
                    onClicked: markupTools.sceneDocumentBinder.changeCase(SceneDocumentBinder.UpperCase)
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "All CAPS\t" + Scrite.app.polishShortcutTextForDisplay("Shift+F3")

                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: parent.height*0.5
                        text: "AB"
                    }
                }

                SimpleToolButton {
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: markupTools.sceneDocumentBinder
                    onClicked: markupTools.sceneDocumentBinder.changeCase(SceneDocumentBinder.LowerCase)
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: "All small\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+Shift+F3")

                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: parent.height*0.5
                        text: "ab"
                    }
                }
            }
        }
    }

    readonly property color transparentColor: "transparent"
    readonly property var availableColors: ["#e60000", "#ff9900", "#ffff00", "#008a00", "#0066cc", "#9933ff", "#ffffff", "#facccc", "#ffebcc", "#ffffcc", "#cce8cc", "#cce0f5", "#ebd6ff", "#bbbbbb", "#f06666", "#ffc266", "#ffff66", "#66b966", "#66a3e0", "#c285ff", "#888888", "#a10000", "#b26b00", "#b2b200", "#006100", "#0047b2", "#6b24b2", "#444444", "#5c0000", "#663d00", "#666600", "#003700", "#002966", "#3d1466"]

    component SimpleToolButton : Rectangle {
        width: 36
        height: 36
        radius: 4
        color: tbMouseArea.pressed || down ? primaryColors.button.background : (checked ? primaryColors.highlight.background : Qt.rgba(0,0,0,0))
        opacity: enabled ? 1 : 0.5

        property bool down: false
        property bool checked: false
        property alias pressed: tbMouseArea.pressed
        property alias hoverEnabled: tbMouseArea.hoverEnabled
        property alias containsMouse: tbMouseArea.containsMouse
        property alias iconSource: tbIcon.source
        signal clicked()

        Image {
            id: tbIcon
            anchors.fill: parent
            anchors.margins: 4
            mipmap: true
        }

        MouseArea {
            id: tbMouseArea
            anchors.fill: parent
            onClicked: parent.clicked()
        }
    }

    component AvailableColorsPalette : Grid {
        id: colorsGrid
        property int cellSize: width/columns
        readonly property int suggestedWidth: 280
        readonly property int suggestedHeight: 200
        columns: 7
        opacity: enabled ? 1 : 0.25

        property color selectedColor: transparentColor
        signal colorPicked(color newColor)

        Item {
            width: colorsGrid.cellSize
            height: colorsGrid.cellSize

            Image {
                source: "../icons/navigation/close.png"
                anchors.fill: parent
                anchors.margins: 5
                mipmap: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: colorPicked(transparentColor)
            }
        }

        Repeater {
            model: availableColors

            Item {
                required property color modelData
                required property int index
                width: colorsGrid.cellSize
                height: colorsGrid.cellSize

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 3
                    border.width: colorsGrid.selectedColor === modelData ? 3 : 0.5
                    border.color: "black"
                    color: modelData
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: colorPicked(modelData)
                }
            }
        }
    }

    component ColorButton : Item {
        id: colorButton
        width: 36
        height: 36
        property color selectedColor: transparentColor
        opacity: enabled ? 1 : 0.5
        property alias hoverEnabled: cbMouseArea.hoverEnabled
        property alias containsMouse: cbMouseArea.containsMouse

        signal colorPicked(color newColor)

        MouseArea {
            id: cbMouseArea
            anchors.fill: parent
            onClicked: colorsMenuLoader.active = true
        }

        Loader {
            id: colorsMenuLoader
            x: 0; y: parent.height
            active: false
            sourceComponent: Popup {
                id: colorsMenu
                x: 0; y: 0
                width: availableColorsPalette.suggestedWidth
                height: availableColorsPalette.suggestedHeight

                Component.onCompleted: open()
                onClosed: Qt.callLater(() => { colorsMenuLoader.active = false})

                contentItem: AvailableColorsPalette {
                    id: availableColorsPalette
                    selectedColor: colorButton.selectedColor
                    onColorPicked: (newColor) => {
                                       colorButton.colorPicked(newColor)
                                       colorsMenu.close()
                                   }
                }
            }
        }
    }

    Rectangle {
        id: screenplayEditorBusyOverlay
        anchors.fill: parent
        color: Qt.rgba(0,0,0,0.1)
        opacity: 1
        visible: RefCounter.isReffed
        onVisibleChanged: parent.enabled = !visible

        BusyIndicator {
            anchors.centerIn: parent
            running: parent.visible
        }

        function ref() {
            RefCounter.ref()
        }
        function deref() {
            RefCounter.deref()
        }
        function reset() {
            RefCounter.reset()
        }
    }
}
