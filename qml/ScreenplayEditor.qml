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
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/structure"
import "qrc:/qml/screenplay"
import "qrc:/qml/floatingdockpanels"

Rectangle {
    // This editor has to specialize in rendering scenes within a ScreenplayAdapter
    // The adapter may contain a single scene or an entire screenplay, that doesnt matter.
    // This way we can avoid having a SceneEditor and ScreenplayEditor as two distinct
    // QML components.

    id: screenplayEditor

    property ScreenplayFormat screenplayFormat: Scrite.document.displayFormat
    property ScreenplayPageLayout pageLayout: screenplayFormat.pageLayout
    property alias source: sourcePropertyAlias.value
    property alias searchBarVisible: searchBarArea.visible
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

    function toggleSearchBar(showReplace) {
        if(typeof showReplace === "boolean")
            searchBar.showReplace = showReplace

        if(searchBarArea.visible) {
            if(searchBar.hasFocus)
                searchBarArea.visible = false
            else
                searchBar.assumeFocus()
        } else {
            searchBarArea.visible = true
            searchBar.assumeFocus()
        }
    }

    color: Runtime.colors.primary.windowColor
    clip: true

    PropertyAlias {
        id: sourcePropertyAlias
        sourceObject: Runtime.screenplayAdapter
        sourceProperty: "source"
    }

    QtObject {
        id: privateData

        readonly property int _InternalSource: 0
        readonly property int _ExternalSource: 1
        property int currentIndexChangeSoruce: _ExternalSource

        function changeCurrentIndexTo(val) {
            currentIndexChangeSoruce = _InternalSource
            Runtime.screenplayAdapter.currentIndex = val
            privateData.currentIndexChangeSoruce = privateData._ExternalSource
        }
    }

    Connections {
        id: screenplayAdapterConnections
        target: Runtime.screenplayAdapter

        function internalSwitchToCurrentIndex() {
            screenplayEditorBusyOverlay.reset()
            forceContentViewPosition.stop()

            const currentIndex = Runtime.screenplayAdapter.currentIndex
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
            const currentIndex = Runtime.screenplayAdapter.currentIndex
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
        enabled: Runtime.screenplayAdapter.isSourceScreenplay
        function onRequestEditorAt(index) {
            contentView.positionViewAtIndex(index, ListView.Beginning)
        }
    }

    // Ctrl+Shift+N should result in the newly added scene to get keyboard focus
    Connections {
        target: Runtime.screenplayAdapter.isSourceScreenplay ? Scrite.document : null
        ignoreUnknownSignals: true
        function onNewSceneCreated(scene, screenplayIndex) {
            Utils.execLater(Runtime.screenplayAdapter.screenplay, 100, function() {
                contentView.positionViewAtIndex(screenplayIndex, ListView.Visible)
                var item = contentView.loadedItemAtIndex(screenplayIndex)
                if(Runtime.mainWindowTab === Runtime.e_ScreenplayTab || Runtime.undoStack.screenplayEditorActive)
                    item.assumeFocus()
            })
        }
        function onLoadingChanged() { zoomSlider.reset() }
    }

    Rectangle {
        id: searchBarArea

        width: ruler.width
        height: searchBar.height * opacity

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 1

        color: Runtime.colors.primary.c100.background
        border.width: 1
        border.color: Runtime.colors.primary.borderColor

        visible: false

        enabled: Runtime.screenplayAdapter.screenplay

        SearchBar {
            id: searchBar

            width: searchBarArea.width * 0.6

            anchors.horizontalCenter: parent.horizontalCenter

            searchEngine.objectName: "Screenplay Search Engine"

            showReplace: false
            allowReplace: !Scrite.document.readOnly

            onShowReplaceRequest: showReplace = flag

            Repeater {
                id: searchAgents
                model: Runtime.screenplayAdapter.screenplay ? 1 : 0

                Item {
                    property string searchString
                    property var searchResults: []
                    property int previousSceneIndex: -1

                    signal replaceCurrentRequest(string replacementText)

                    SearchAgent.onReplaceAll: {
                        Runtime.screenplayTextDocument.syncEnabled = false
                        Runtime.screenplayAdapter.screenplay.replace(searchString, replacementText, 0)
                        Runtime.screenplayTextDocument.syncEnabled = true
                    }
                    SearchAgent.onReplaceCurrent: replaceCurrentRequest(replacementText)

                    SearchAgent.engine: searchBar.searchEngine

                    SearchAgent.onSearchRequest: {
                        searchString = string
                        searchResults = Runtime.screenplayAdapter.screenplay.search(string, 0)
                        SearchAgent.searchResultCount = searchResults.length
                    }

                    SearchAgent.onCurrentSearchResultIndexChanged: {
                        if(SearchAgent.currentSearchResultIndex >= 0) {
                            var searchResult = searchResults[SearchAgent.currentSearchResultIndex]
                            var sceneIndex = searchResult["sceneIndex"]
                            if(sceneIndex !== previousSceneIndex)
                                clearPreviousElementUserData()
                            var sceneResultIndex = searchResult["sceneResultIndex"]
                            var screenplayElement = Runtime.screenplayAdapter.screenplay.elementAt(sceneIndex)
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
                        Runtime.screenplayAdapter.screenplay.currentElementIndex = previousSceneIndex
                        searchString = ""
                        searchResults = []
                        clearPreviousElementUserData()
                    }

                    function clearPreviousElementUserData() {
                        if(previousSceneIndex >= 0) {
                            var screenplayElement = Runtime.screenplayAdapter.screenplay.elementAt(previousSceneIndex)
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
        anchors.top: searchBarArea.visible ? searchBarArea.bottom : parent.top
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
                enabled: Runtime.applicationSettings.enableAnimations && contentView.commentsExpandCounter > 0
                NumberAnimation { duration: 50 }
            }

            Rectangle {
                id: contentArea
                anchors.top: ruler.visible ? ruler.bottom : parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.topMargin: ruler.visible ? 5 : 1
                color: Runtime.screenplayAdapter.elementCount === 0 || contentView.spacing === 0 ? "white" : Qt.rgba(0,0,0,0)

                TrackerPack {
                    id: trackerPack
                    property int counter: 0
                    TrackProperty { target: Runtime.screenplayEditorSettings; property: "displaySceneCharacters" }
                    // We shouldnt be tracking changes in elementCount as a reason to reset
                    // the model used by contentView. This causes too many delegate creation/deletions.
                    // Just not effective.
                    // TrackProperty { target: Runtime.screenplayAdapter; property: "elementCount" }
                    TrackProperty { target: Runtime.screenplayAdapter; property: "source" }
                    onTracked: counter = counter+1
                }

                ResetOnChange {
                    id: contentViewModel
                    trackChangesOn: trackerPack.counter
                    from: null
                    to: Runtime.screenplayAdapter
                    onJustReset: {
                        if(Runtime.screenplayAdapter.currentIndex < 0)
                            contentView.positionViewAtBeginning()
                        else
                            contentView.positionViewAtIndex(Runtime.screenplayAdapter.currentIndex, ListView.Beginning)
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
                            if(!Runtime.screenplayAdapter.isSourceScreenplay)
                                return contentView.spacing
                            var ret = logLineEditor.visible ? logLineEditor.contentHeight : 0;
                            if(Runtime.screenplayAdapter.isSourceScreenplay)
                                ret += titleCardLoader.active ? titleCardLoader.height : Math.max(ruler.topMarginPx,editTitlePageButton.height+20)
                            return ret + contentView.spacing
                        }
                        property real padding: width * 0.1

                        Rectangle {
                            anchors.fill: parent
                            anchors.bottomMargin: contentView.spacing
                            visible: Runtime.screenplayAdapter.elementCount > 0 && contentView.spacing > 0 && Runtime.screenplayAdapter.isSourceScreenplay
                        }

                        Connections {
                            target: titleCardLoader.item

                            function onEditTitlePageRequest(sourceItem) {
                                const dlg = TitlePageDialog.launch()
                                dlg.closed.connect(contentView.positionViewAtBeginning)
                            }
                        }

                        Loader {
                            id: titleCardLoader
                            active: Runtime.screenplayAdapter.isSourceScreenplay && (Scrite.document.screenplay.hasTitlePageAttributes || logLineEditor.visible || Scrite.document.screenplay.coverPagePhoto !== "")
                            sourceComponent: titleCardComponent
                            anchors.left: parent.left
                            anchors.right: parent.right

                            FlatToolButton {
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.rightMargin: ruler.rightMarginPx
                                iconSource: "qrc:/icons/action/edit_title_page.png"
                                onClicked: {
                                    const dlg = TitlePageDialog.launch()
                                    dlg.closed.connect(contentView.positionViewAtBeginning)
                                }
                                visible: parent.active && enabled
                                enabled: !Scrite.document.readOnly
                            }
                        }

                        VclToolButton {
                            id: editTitlePageButton
                            text: "Edit Title Page"
                            icon.source: "qrc:/icons/action/edit_title_page.png"
                            flat: false
                            width: implicitWidth * 1.5
                            height: implicitHeight * 1.25
                            visible: Runtime.screenplayAdapter.isSourceScreenplay && titleCardLoader.active === false && enabled
                            opacity: hovered ? 1 : 0.75
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: Runtime.screenplayAdapter.elementCount > 0 ? -contentView.spacing/2 : 0
                            onClicked: {
                                const dlg = TitlePageDialog.launch()
                                dlg.closed.connect(contentView.positionViewAtBeginning)
                            }
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
                            visible: Runtime.screenplayEditorSettings.showLoglineEditor && Runtime.screenplayAdapter.isSourceScreenplay && (Scrite.document.readOnly ? logLineField.text !== "" : true)

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

                                VclLabel {
                                    id: logLineFieldHeading
                                    text: logLineField.activeFocus ? ("Logline: (" + (loglineLimiter.limitReached ? "WARNING: " : "") + loglineLimiter.wordCount + "/" + loglineLimiter.maxWordCount + " words, " +
                                          loglineLimiter.letterCount + "/" + loglineLimiter.maxLetterCount + " letters)") : "Logline: "
                                    font.family: screenplayFormat.defaultFont2.family
                                    font.pointSize: screenplayFormat.defaultFont2.pointSize-2
                                    visible: logLineField.length > 0
                                    color: loglineLimiter.limitReached ? "darkred" : Runtime.colors.primary.a700.background
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
                            if(!Runtime.screenplayAdapter.isSourceScreenplay)
                                return contentView.spacing
                            return Math.max(ruler.bottomMarginPx, addEpisodeButton.height+20) + contentView.spacing
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.topMargin: contentView.spacing
                            visible: Runtime.screenplayAdapter.elementCount > 0 && contentView.spacing > 0 && Runtime.screenplayAdapter.isSourceScreenplay
                        }

                        Row {
                            id: addButtonsRow
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: Runtime.screenplayAdapter.elementCount > 0 ? contentView.spacing/2 : 0
                            visible: Runtime.screenplayAdapter.isSourceScreenplay && enabled
                            enabled: !Scrite.document.readOnly
                            spacing: 20

                            FlatToolButton {
                                id: addSceneButton
                                iconSource: "qrc:/icons/action/add_scene.png"
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

                            FlatToolButton {
                                id: addActBreakButton
                                iconSource: "qrc:/icons/action/add_act.png"
                                shortcutText: "Ctrl+Shift+B"
                                ToolTip.delay: 0
                                text: "Add Act Break"
                                suggestedWidth: 48
                                suggestedHeight: 48
                                onClicked: Scrite.document.screenplay.addBreakElement(Screenplay.Act)
                            }

                            FlatToolButton {
                                id: addEpisodeButton
                                iconSource: "qrc:/icons/action/add_episode.png"
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
                            active: Runtime.mainWindowTab === Runtime.e_ScreenplayTab && contentView.count === 1 && !Runtime.screenplayEditorSettings.screenplayEditorAddButtonsAnimationShown
                            anchors.fill: parent
                            sourceComponent: UiElementHighlight {
                                uiElement: addButtonsRow
                                uiElementBoxVisible: true
                                descriptionPosition: Item.Bottom
                                description: "Use these buttons to add new a scene, act or episode."
                                highlightAnimationEnabled: false
                                onDone: addButtonsAnimator.active = false
                                Component.onCompleted: Runtime.screenplayEditorSettings.screenplayEditorAddButtonsAnimationShown = true
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
                    spacing: Runtime.screenplayAdapter.elementCount > 0 ? Runtime.screenplayEditorSettings.spaceBetweenScenes*zoomLevel : 0
                    property int commentsExpandCounter: 0
                    property bool commentsExpanded: false
                    property bool scrollingBetweenScenes: false
                    readonly property bool loadAllDelegates: false // for future use
                    property real spaceForComments: {
                        if(Runtime.screenplayEditorSettings.displaySceneComments && commentsPanelAllowed)
                            return Math.round(screenplayEditorWorkspace.width - pageRulerArea.width - pageRulerArea.minLeftMargin - 20)
                        return 0
                    }
                    property int commentsPanelTabIndex: Runtime.screenplayEditorSettings.commentsPanelTabIndex
                    onCommentsPanelTabIndexChanged: Runtime.screenplayEditorSettings.commentsPanelTabIndex = commentsPanelTabIndex
                    onCommentsExpandedChanged: commentsExpandCounter = commentsExpandCounter+1
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                    function delegateWasLoaded() { Qt.callLater(delegateLoaded) }
                    signal delegateLoaded()

                    property bool allowContentYAnimation
                    Behavior on contentY {
                        enabled: Runtime.applicationSettings.enableAnimations && contentView.allowContentYAnimation
                        NumberAnimation {
                            duration: 100
                            onFinished: contentView.allowContentYAnimation = false
                        }
                    }

                    header: {
                        if(Runtime.screenplayEditorSettings.displayEmptyTitleCard)
                            return contentViewHeaderComponent
                        if(Runtime.screenplayAdapter.isSourceScreenplay) {
                            const logLineEditorVisible = Runtime.screenplayEditorSettings.showLoglineEditor && (Scrite.document.readOnly ? Runtime.screenplayAdapter.screenplay.logline !== "" : true)
                            if (Runtime.screenplayAdapter.screenplay.hasTitlePageAttributes || logLineEditorVisible)
                                return contentViewHeaderComponent
                        }
                        return contentViewDummyHeaderComponent
                    }

                    footer: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons ? contentViewFooterComponent : contentViewDummyFooterComponent

                    delegate: contentViewDelegateComponent

                    snapMode: ListView.NoSnap
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds
                    keyNavigationEnabled: false
                    ScrollBar.vertical: verticalScrollBar
                    property int numberOfWordsAddedToDict : 0

                    FocusTracker.window: Scrite.window
                    FocusTracker.indicator.target: Runtime.undoStack
                    FocusTracker.indicator.property: Runtime.screenplayAdapter.isSourceScreenplay ? "screenplayEditorActive" : "sceneEditorActive"
                    FocusTracker.onHasFocusChanged: {
                        Runtime.undoStack.screenplayEditorActive = FocusTracker.hasFocus && Runtime.screenplayAdapter.isSourceScreenplay
                        Runtime.undoStack.sceneEditorActive = !FocusTracker.hasFocus && Runtime.screenplayAdapter.isSourceScreenplay
                    }
                    Component.onDestruction: {
                        Runtime.undoStack.screenplayEditorActive = false
                        Runtime.undoStack.sceneEditorActive = false
                    }

                    property int defaultCacheBuffer: -1
                    function configureCacheBuffer() {
                        defaultCacheBuffer = cacheBuffer
                        cacheBuffer = Qt.binding( () => {
                                                     if(!model)
                                                        return defaultCacheBuffer
                                                     return (Runtime.screenplayEditorSettings.optimiseScrolling || contentView.loadAllDelegates) ? 2147483647 : defaultCacheBuffer
                                                 })
                    }

                    Component.onCompleted: {
                        if(Scrite.app.isMacOSPlatform)
                            flickDeceleration = 7500
                        positionViewAtIndex(Runtime.screenplayAdapter.currentIndex, ListView.Beginning)
                        configureCacheBuffer()
                    }

                    property point firstPoint: mapToItem(contentItem, width/2, 1)
                    property point lastPoint: mapToItem(contentItem, width/2, height-2)
                    property int firstItemIndex: Runtime.screenplayAdapter.elementCount > 0 ? Math.max(indexAt(firstPoint.x, firstPoint.y), 0) : 0
                    property int lastItemIndex: Runtime.screenplayAdapter.elementCount > 0 ? validOrLastIndex(indexAt(lastPoint.x, lastPoint.y)) : 0

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
                        var ci = Runtime.screenplayAdapter.currentIndex
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
                            if(ci >= 0 && ci <= Runtime.screenplayAdapter.elementCount-1)
                                privateData.changeCurrentIndexTo(ci)
                        }
                    }

                    function validOrLastIndex(val) { return val < 0 ? Runtime.screenplayAdapter.elementCount-1 : val }

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
                visible: Runtime.screenplayEditorSettings.displayRuler

                property real leftMarginPx: leftMargin * zoomLevel
                property real rightMarginPx: rightMargin * zoomLevel
                property real topMarginPx: pageLayout.topMargin * Screen.devicePixelRatio * zoomLevel
                property real bottomMarginPx: pageLayout.bottomMargin * Screen.devicePixelRatio * zoomLevel
            }
        }

        BusyIcon {
            anchors.centerIn: parent
            running: Scrite.document.loading || !Runtime.screenplayTextDocument.paused && Runtime.screenplayTextDocument.updating
            visible: running
        }
    }

    ScrollBar {
        id: verticalScrollBar
        anchors.top: screenplayEditorWorkspace.top
        anchors.right: parent.right
        anchors.bottom: statusBar.enabled ? statusBar.top : parent.bottom
        orientation: Qt.Vertical
        minimumSize: 0.1
        policy: Runtime.screenplayAdapter.elementCount > 0 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
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
        color: Runtime.colors.primary.windowColor
        border.width: 1
        border.color: Runtime.colors.primary.borderColor
        clip: true
        enabled: (width > (metricsDisplay.width + zoomSlider.width + 40))
        opacity: enabled ? 1 : 0

        Item {
            anchors.fill: metricsDisplay

            ToolTip.text: "Page count and time estimates are approximate, assuming " + Runtime.screenplayTextDocument.timePerPageAsString + " per page."
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
                        return "qrc:/icons/action/lock_outline.png"
                    if(Scrite.user.loggedIn)
                        return Scrite.document.hasCollaborators ? "qrc:/icons/file/protected.png" : "qrc:/icons/file/unprotected.png"
                    return Scrite.document.locked ? "qrc:/icons/action/lock_outline.png" : "qrc:/icons/action/lock_open.png"
                }
                scale: toggleLockMouseArea.containsMouse ? (toggleLockMouseArea.pressed ? 1 : 1.5) : 1
                visible: Runtime.mainWindowTab === Runtime.e_ScreenplayTab
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
                            CollaboratorsDialog.launch()
                        else
                            toggleLock()
                    }

                    function toggleLock() {
                        var locked = !Scrite.document.locked
                        Scrite.document.locked = locked

                        var message = ""
                        if(locked)
                            message = "Document LOCKED. You will be able to edit it only on this computer."
                        else
                            message = "Document unlocked. You will be able to edit it on this and any other computer."

                        MessageBox.information("Document Lock Status", message)
                    }
                }
            }

            Image {
                source: "qrc:/icons/navigation/refresh.png"
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: Runtime.screenplayTextDocument.paused ? 0.85 : 1
                scale: refreshMouseArea.containsMouse ? (refreshMouseArea.pressed ? 1 : 1.5) : 1
                visible: Runtime.mainWindowTab === Runtime.e_ScreenplayTab
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: refreshMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if(Runtime.screenplayTextDocument.paused)
                            Runtime.screenplayTextDocument.paused = false
                        else
                            Runtime.screenplayTextDocument.reload()
                    }
                    ToolTip.visible: containsMouse && !pressed
                    ToolTip.text: enabled ? "Computes page layout from scratch, thereby reevaluating page count and time." : "Resume page and time computation."
                    ToolTip.delay: 1000
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: Runtime.colors.primary.borderColor
                visible: Runtime.mainWindowTab === Runtime.e_ScreenplayTab
            }

            Image {
                id: pageCountButton
                source: "qrc:/icons/content/page_count.png"
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: Runtime.screenplayTextDocument.paused ? 0.85 : 1
                scale: pageCountMouseAra.containsMouse ? (pageCountMouseAra.pressed ? 1 : 1.5) : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: pageCountMouseAra
                    anchors.fill: parent
                    onClicked: Runtime.screenplayTextDocument.paused = !Runtime.screenplayTextDocument.paused
                    hoverEnabled: true
                    ToolTip.visible: containsMouse && !pressed
                    ToolTip.text: "Click here to toggle page computation, in case the app is not responding fast while typing."
                    ToolTip.delay: 1000
                }
            }

            VclText {
                font.pixelSize: statusBar.height * 0.5
                text: Runtime.screenplayTextDocument.paused ? "- of -" : (Runtime.screenplayTextDocument.currentPage + " of " + Runtime.screenplayTextDocument.pageCount)
                anchors.verticalCenter: parent.verticalCenter
                opacity: Runtime.screenplayTextDocument.paused ? 0.5 : 1
            }

            Rectangle {
                width: 1
                height: parent.height
                color: Runtime.colors.primary.borderColor
            }

            Image {
                source: "qrc:/icons/content/time.png"
                height: parent.height; width: height; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: Runtime.screenplayTextDocument.paused ? 0.85 : 1
                scale: timeMouseArea.containsMouse ? (timeMouseArea.pressed ? 1 : 1.5) : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: timeMouseArea
                    anchors.fill: parent
                    onClicked: Runtime.screenplayTextDocument.paused = !Runtime.screenplayTextDocument.paused
                    hoverEnabled: true
                    ToolTip.visible: containsMouse && !pressed
                    ToolTip.text: "Click here to toggle time computation, in case the app is not responding fast while typing."
                    ToolTip.delay: 1000
                }
            }

            VclText {
                font.pixelSize: statusBar.height * 0.5
                text: Runtime.screenplayTextDocument.paused ? "- of -" : (Runtime.screenplayTextDocument.currentTimeAsString + " of " + (Runtime.screenplayTextDocument.pageCount > 1 ? Runtime.screenplayTextDocument.totalTimeAsString : Runtime.screenplayTextDocument.timePerPageAsString))
                anchors.verticalCenter: parent.verticalCenter
                opacity: Runtime.screenplayTextDocument.paused ? 0.5 : 1
            }

            Rectangle {
                width: 1
                height: parent.height
                color: Runtime.colors.primary.borderColor
                visible: wordCountLabel.visible
            }

            VclText {
                id: wordCountLabel
                font.pixelSize: statusBar.height * 0.5
                text: {
                    const currentScene = Runtime.screenplayAdapter.currentScene
                    const currentSceneWordCount = currentScene ? currentScene.wordCount + " / " : ""
                    const totalWordCount = Runtime.screenplayAdapter.wordCount + (Runtime.screenplayAdapter.wordCount !== 1 ? " words" : " word")
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
                    if(Runtime.screenplayAdapter.isSourceScene || Runtime.screenplayAdapter.elementCount === 0)
                        return null

                    var element = null
                    if(contentView.isVisible(Runtime.screenplayAdapter.currentIndex)) {
                        element = Runtime.screenplayAdapter.currentElement
                    } else {
                        var data = Runtime.screenplayAdapter.at(contentView.firstItemIndex)
                        element = data ? data.screenplayElement : null
                    }

                    return element
                }
                property Scene currentScene: currentSceneElement ? currentSceneElement.scene : null
                property SceneHeading currentSceneHeading: currentScene && currentScene.heading.enabled ? currentScene.heading : null

                VclLabel {
                    id: currentSceneNumber
                    anchors.verticalCenter: currentSceneHeadingText.verticalCenter
                    anchors.left: currentSceneHeadingText.left
                    anchors.leftMargin: Math.min(-recommendedMargin, -contentWidth)
                    font: currentSceneHeadingText.font
                    text: parent.currentSceneHeading ? parent.currentSceneElement.resolvedSceneNumber + ". " : ''
                    property real recommendedMargin: headingFontMetrics.averageCharacterWidth*5 + ruler.leftMarginPx*0.075
                }

                VclText {
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

            FlatToolButton {
                iconSource: "qrc:/icons/action/layout_grouping.png"
                height: parent.height; width: height
                anchors.verticalCenter: parent.verticalCenter
                down: taggingMenu.active
                onClicked: taggingMenu.show()
                ToolTip.text: "Grouping Options"
                visible: Runtime.screenplayEditorSettings.allowTaggingOfScenes && Runtime.mainWindowTab === Runtime.e_ScreenplayTab

                MenuLoader {
                    id: taggingMenu
                    anchors.left: parent.left
                    anchors.bottom: parent.top
                    menu: VclMenu {
                        id: layoutGroupingMenu
                        width: 350

                        VclMenuItem {
                            text: "None"
                            icon.source: font.bold ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                            font.bold: Scrite.document.structure.preferredGroupCategory === ""
                            onTriggered: Scrite.document.structure.preferredGroupCategory = ""
                        }

                        MenuSeparator { }

                        Repeater {
                            model: Scrite.document.structure.groupCategories

                            VclMenuItem {
                                text: Scrite.app.camelCased(modelData)
                                icon.source: font.bold ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
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
                color: Runtime.colors.primary.borderColor
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
            zoomSliderVisible: Runtime.mainWindowTab === Runtime.e_ScreenplayTab
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
            color: Runtime.colors.primary.c10.background

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

            VclTextField {
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

            FlatToolButton {
                id: deleteBreakButton
                iconSource: "qrc:/icons/action/delete.png"
                width: headingFontMetrics.lineSpacing
                height: headingFontMetrics.lineSpacing
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: ruler.rightMarginPx
                onClicked: Runtime.screenplayAdapter.screenplay.removeElement(episodeBreakItem.theElement)
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
            color: Runtime.colors.primary.c10.background

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

            VclTextField {
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

            FlatToolButton {
                id: deleteBreakButton
                iconSource: "qrc:/icons/action/delete.png"
                width: headingFontMetrics.lineSpacing
                height: headingFontMetrics.lineSpacing
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: ruler.rightMarginPx
                onClicked: Runtime.screenplayAdapter.screenplay.removeElement(actBreakItem.theElement)
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
            border.color: screenplayElement.scene ? screenplayElement.scene.color : Runtime.colors.primary.c400.background
            color: screenplayElement.scene ? Qt.tint(screenplayElement.scene.color, "#E7FFFFFF") : Runtime.colors.primary.c300.background

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

            VclLabel {
                font: sceneHeadingText.font
                anchors.verticalCenter: sceneHeadingText.verticalCenter
                anchors.right: sceneHeadingText.left
                anchors.rightMargin: 20
                width: headingFontMetrics.averageCharacterWidth*5
                color: screenplayElement.hasUserSceneNumber ? "black" : "gray"
                text: screenplayElement.resolvedSceneNumber
            }

            VclLabel {
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
                    if(screenplayElementType === ScreenplayElement.BreakElementType)
                        return screenplayElement.breakTitle
                    if(screenplayElement.omitted)
                        return "[OMITTED]"
                    if(scene && scene.heading.enabled)
                        return scene.heading.text
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
                source: "qrc:/images/sample_scene.png"
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
            property bool isCurrent: theElement === Runtime.screenplayAdapter.currentElement
            z: isCurrent ? 2 : 1

            width: contentArea.width
            height: omittedContentItemLayout.height
            color: Scrite.app.isVeryLightColor(theScene.color) ? Runtime.colors.primary.highlight.background : Qt.tint(theScene.color, "#9CFFFFFF")

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
            property bool isCurrent: theElement === Runtime.screenplayAdapter.currentElement
            z: isCurrent ? 2 : 1

            width: contentArea.width
            height: contentItemLayout.height
            color: "white"
            readonly property var binder: sceneDocumentBinder
            readonly property var editor: sceneTextEditor
            property bool canSplitScene: sceneTextEditor.activeFocus && !Scrite.document.readOnly && sceneDocumentBinder.currentElement && sceneDocumentBinder.currentElementCursorPosition === 0 && Runtime.screenplayAdapter.isSourceScreenplay
            property bool canJoinToPreviousScene: sceneTextEditor.activeFocus && !Scrite.document.readOnly && sceneTextEditor.cursorPosition === 0 && contentItem.theIndex > 0

            FocusTracker.window: Scrite.window
            FocusTracker.onHasFocusChanged: {
                contentItem.theScene.undoRedoEnabled = FocusTracker.hasFocus
                sceneHeadingAreaLoader.item.sceneHasFocus = FocusTracker.hasFocus
            }

            SceneDocumentBinder {
                id: sceneDocumentBinder
                objectName: "ScreenplayEditor.Scene" + contentItem.theElement.resolvedSceneNumber + ""
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
                autoCapitalizeSentences: !Scrite.document.readOnly && Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                autoPolishParagraphs: !Scrite.document.readOnly && Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                liveSpellCheckEnabled: sceneTextEditor.activeFocus
                property bool firstInitializationDone: false
                onDocumentInitialized: {
                    if(!firstInitializationDone && !contentView.scrollingBetweenScenes)
                        sceneTextEditor.cursorPosition = 0
                    firstInitializationDone = true
                }
                onRequestCursorPosition: (position) => {
                                             /* Upon receipt of this signal, lets immediately reset cursor position.
                                                if there is a need for delayed setting of cursor position, let that be
                                                a separate signal emission from the backend. */
                                             // if(position >= 0)
                                             //    contentItem.assumeFocusLater(position, 100)
                                             contentItem.assumeFocusAt(position)
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
                applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
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
                to: Runtime.screenplayEditorSettings.enableSpellCheck
                delay: 100
            }

            SidePanel {
                id: commentsSidePanel

                property color theSceneDarkColor: Scrite.app.isLightColor(contentItem.theScene.color) ? Runtime.colors.primary.c500.background : contentItem.theScene.color
                property real screenY: screenplayEditor.mapFromItem(parent, 0, 0).y
                property real maxTopMargin: contentItem.height-height-20

                z: contentItem.isCurrent ? 1 : 0

                anchors.top: parent.top
                anchors.left: parent.right
                anchors.topMargin: screenY < 0 ? Math.min(-screenY,maxTopMargin) : -1

                buttonColor: expanded ? Qt.tint(contentItem.theScene.color, "#C0FFFFFF") : Qt.tint(contentItem.theScene.color, "#D7EEEEEE")
                backgroundColor: buttonColor

                borderColor: expanded ? Runtime.colors.primary.borderColor : (contentView.spacing > 0 ? Scrite.app.translucent(theSceneDarkColor,0.25) : Qt.rgba(0,0,0,0))
                borderWidth: 0 // contentItem.isCurrent ? 2 : 1

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
                            property bool hasIndexCardFields: contentItem.theScene.indexCardFieldValues.length > 0
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
                                    return "qrc:/icons/content/note.png"

                                if(sceneFeaturedImage)
                                    return "qrc:/icons/filetype/photo.png"

                                if(hasIndexCardFields)
                                    return "qrc:/icons/content/form.png"

                                return ""
                            }
                        }
                    }
                }

                Component {
                    id: commentsExpandedSidePanelCornerComponent

                    Column {
                        spacing: 1

                        FlatToolButton {
                            iconSource: down ? "qrc:/icons/content/comments_panel_inverted.png" : "qrc:/icons/content/comments_panel.png"
                            suggestedWidth: parent.width
                            suggestedHeight: parent.width
                            down: contentView.commentsPanelTabIndex === 0
                            downIndicatorColor: commentsSidePanel.theSceneDarkColor
                            onClicked: contentView.commentsPanelTabIndex = 0
                            ToolTip.visible: hovered
                            ToolTip.text: "View/edit scene comments."
                        }

                        FlatToolButton {
                            iconSource: down ? "qrc:/icons/filetype/photo_inverted.png" : "qrc:/icons/filetype/photo.png"
                            suggestedWidth: parent.width
                            suggestedHeight: parent.width
                            down: contentView.commentsPanelTabIndex === 1
                            downIndicatorColor: commentsSidePanel.theSceneDarkColor
                            onClicked: contentView.commentsPanelTabIndex = 1
                            ToolTip.visible: hovered
                            ToolTip.text: "View/edit scene featured image."
                        }

                        FlatToolButton {
                            iconSource: down ? "qrc:/icons/content/form_inverted.png" : "qrc:/icons/content/form.png"
                            suggestedWidth: parent.width
                            suggestedHeight: parent.width
                            visible: Runtime.screenplayEditorSettings.displayIndexCardFields
                            down: contentView.commentsPanelTabIndex === 2
                            downIndicatorColor: commentsSidePanel.theSceneDarkColor
                            onClicked: contentView.commentsPanelTabIndex = 2
                            ToolTip.visible: hovered
                            ToolTip.text: "View/edit index card fields."
                            onVisibleChanged: {
                                if(!visible && contentView.commentsPanelTabIndex === 2)
                                    contentView.commentsPanelTabIndex = 0
                            }
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
                visible: width >= 100 && Runtime.screenplayEditorSettings.displaySceneComments && Runtime.mainWindowTab === Runtime.e_ScreenplayTab
                opacity: expanded ? (Runtime.screenplayAdapter.currentIndex < 0 || Runtime.screenplayAdapter.currentIndex === contentItem.theIndex ? 1 : 0.75) : 1
                Behavior on opacity {
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }
                content: TrapeziumTabView {
                    id: commentsSidePanelTabView
                    tabBarVisible: false
                    tabColor: commentsSidePanel.theSceneDarkColor
                    currentTabIndex: contentView.commentsPanelTabIndex
                    currentTabContent: [commentsEditComponent,featuredPhotoComponent,indexCardFieldsComponent][currentTabIndex%3]

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
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 1
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

                            Transliterator.spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck

                            SpecialSymbolsSupport {
                                anchors.top: parent.bottom
                                anchors.left: parent.left
                                textEditor: commentsEdit
                                textEditorHasCursorInterface: true
                                enabled: !Scrite.document.readOnly
                            }

                            TextAreaSpellingSuggestionsMenu { }

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

                    Component {
                        id: indexCardFieldsComponent

                        Item {
                            id: icfItem

                            property Scene scene: contentItem.theScene
                            property int sceneIndex: contentItem.theIndex

                            TabSequenceManager {
                                id: icfTabSequence
                                wrapAround: true
                                enabled: Runtime.appFeatures.structure.enabled
                            }

                            Flickable {
                                id: icfFlickable
                                anchors.fill: parent
                                anchors.margins: 5
                                anchors.rightMargin: 0

                                clip: interactive
                                contentY: 0
                                contentWidth: icfLayout.width
                                contentHeight: icfLayout.height
                                flickableDirection: Flickable.VerticalFlick
                                interactive: contentHeight > height

                                ScrollBar.vertical: VclScrollBar { }

                                ColumnLayout {
                                    id: icfLayout

                                    width: icfFlickable.ScrollBar.vertical.needed ? icfFlickable.width-20 : icfFlickable.width

                                    enabled: Runtime.appFeatures.structure.enabled
                                    opacity: enabled ? true : false

                                    TextAreaInput {
                                        id: icfSynopsis

                                        Layout.fillWidth: true
                                        Layout.preferredHeight: {
                                            if(icfIcf.hasFields)
                                                return Math.max(icfItem.height * 0.5,
                                                         contentHeight + Runtime.idealFontMetrics.lineSpacing*2,
                                                         icfFlickable.height-icfIcf.height-parent.spacing)
                                            return Math.max(contentHeight + Runtime.idealFontMetrics.lineSpacing*2,
                                                        icfFlickable.height-icfEditButton.height-parent.spacing)
                                        }

                                        visible: !Runtime.screenplayEditorSettings.displaySceneSynopsis
                                        readOnly: Scrite.document.readOnly

                                        TabSequenceItem.manager: icfTabSequence
                                        TabSequenceItem.enabled: visible
                                        TabSequenceItem.sequence: 0

                                        text: icfItem.scene.synopsis
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        placeholderText: "Synopsis"
                                        background: Item { }

                                        onTextChanged: Qt.callLater(commitSynopsis)

                                        function commitSynopsis() {
                                            icfItem.scene.synopsis = text
                                        }
                                    }

                                    IndexCardFields {
                                        id: icfIcf

                                        Layout.fillWidth: true

                                        lod: eHIGH
                                        visible: hasFields
                                        wrapMode: TextInput.WrapAtWordBoundaryOrAnywhere
                                        structureElement: icfItem.scene.structureElement
                                        startTabSequence: 1
                                        tabSequenceManager: icfTabSequence
                                        tabSequenceEnabled: true
                                    }

                                    VclToolButton {
                                        id: icfEditButton

                                        Layout.alignment: Qt.AlignLeft
                                        Layout.fillWidth: !icfIcf.hasFields

                                        ToolTip.visible: icfIcf.hasFields ? hovered : false
                                        ToolTip.text: "Edit Index Card Fields"

                                        text: icfIcf.hasFields ? "" : ToolTip.text
                                        icon.source: "qrc:/icons/action/edit.png"
                                        visible: icfItem.sceneIndex === Runtime.screenplayAdapter.currentIndex

                                        onClicked: StructureIndexCardFieldsDialog.launch()
                                    }
                                }

                                FocusTracker.window: Scrite.window
                                FocusTracker.onHasFocusChanged: {
                                    if(FocusTracker.hasFocus)
                                        privateData.changeCurrentIndexTo(sceneIndex)
                                }

                                property Item activeFocusItem: Scrite.window.activeFocusItem
                                onActiveFocusItemChanged: {
                                    if(FocusTracker.hasFocus && activeFocusItem)
                                        ensureVisible(activeFocusItem)
                                    else
                                        contentY = 0
                                }

                                function ensureVisible(item) {
                                    const cr = item.mapToItem(icfLayout, 0,0,item.width,item.height)
                                    let cy = contentY
                                    let ch = height
                                    if(cr.y < cy)
                                        cy = Math.max(cr.y, 0)
                                    else if(cr.y + cr.height > cy + ch)
                                        cy = Math.min(cr.y + cr.height - ch, contentHeight-ch)
                                    else
                                        return

                                    contentY = cy
                                }
                            }

                            DisabledFeatureNotice {
                                color: Qt.rgba(0,0,0,0)
                                anchors.fill: parent
                                visible: !Runtime.appFeatures.structure.enabled
                                featureName: "Index Card Fields"
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
                        if(stype === Runtime.announcementIds.focusRequest) {
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
                    visible: Runtime.screenplayEditorSettings.displaySceneSynopsis

                    Column {
                        id: synopsisEditorLayout
                        width: parent.width-10
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: ruler.leftMarginPx
                        anchors.rightMargin: ruler.rightMarginPx

                        VclLabel {
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
                            Transliterator.spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck
                            onTextChanged: contentItem.theScene.synopsis = text
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            placeholderText: "Enter the synopsis of your scene here."
                            background: Item { }

                            TextAreaSpellingSuggestionsMenu { }

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
                                if(!sceneTextEditor.activeFocus || !Runtime.screenplayEditorSettings.displaySceneSynopsis)
                                    return
                                var sdata = "" + data
                                var stype = "" + type
                                if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.sceneSynopsis) {
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
                    height: Math.ceil(contentHeight + topPadding + bottomPadding + Runtime.sceneEditorFontMetrics.lineSpacing)
                    topPadding: Runtime.sceneEditorFontMetrics.height
                    bottomPadding: Runtime.sceneEditorFontMetrics.height
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
                            to: Runtime.screenplayTextDocument.paused ? null : Runtime.screenplayTextDocument
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
                                // color: Runtime.colors.primary.c400.background

                                PageNumberBubble {
                                    x: -width - 20
                                    pageNumber: modelData.pageNumber
                                }
                            }
                        }

                        Rectangle {
                            visible: sceneTextEditor.cursorVisible && sceneTextEditor.activeFocus && Runtime.screenplayEditorSettings.highlightCurrentLine && Scrite.app.usingMaterialTheme
                            x: 0; y: sceneTextEditor.cursorRectangle.y-2*zoomLevel
                            width: parent.width
                            height: sceneTextEditor.cursorRectangle.height+4*zoomLevel
                            color: Runtime.colors.primary.c100.background

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
                    onActiveFocusChanged: Qt.callLater(respondToActiveFocusChange)

                    function respondToActiveFocusChange() {
                        if(!Scrite.window.active)
                            return

                        if(activeFocus) {
                            completionModel.actuallyEnable = true
                            contentView.ensureVisible(sceneTextEditor, cursorRectangle)
                            privateData.changeCurrentIndexTo(contentItem.theIndex)
                            Runtime.screenplayEditorToolbar.set(sceneTextEditor, sceneDocumentBinder)
                            FloatingMarkupToolsDock.sceneDocumentBinder = sceneDocumentBinder
                            justReceivedFocus = true
                            Announcement.shout(Runtime.announcementIds.sceneTextEditorReceivedFocus, sceneTextEditor)
                        } else {
                            Runtime.screenplayEditorToolbar.reset(sceneTextEditor, sceneDocumentBinder)
                            if(FloatingMarkupToolsDock.sceneDocumentBinder === sceneDocumentBinder)
                                FloatingMarkupToolsDock.sceneDocumentBinder = null
                        }
                    }

                    function reload() {
                        Utils.execLater(sceneDocumentBinder, 1000, function() {
                            sceneDocumentBinder.preserveScrollAndReload()
                        } )
                    }

                    Announcement.onIncoming: (type,data) => {
                                                 if(sceneTextEditor.activeFocus && contentItem.isCurrent)
                                                    return

                                                 if(type === Runtime.announcementIds.sceneTextEditorReceivedFocus && data !== null && data !== sceneTextEditor) {
                                                     select(0,0)
                                                     return
                                                 }

                                                 if(""+type === Runtime.announcementIds.focusRequest && ""+data === Runtime.announcementData.focusOptions.scene) {
                                                     synopsisEditorField.forceActiveFocus()
                                                     return
                                                 }
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
                                color: Scrite.document.readOnly ? Runtime.colors.primary.borderColor : "black"
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
                            contentItem: ListView {
                                id: completionView
                                model: completionModel
                                clip: true
                                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                                height: Math.min(contentHeight, 7*(defaultFontMetrics.lineSpacing+2*5))
                                interactive: true
                                ScrollBar.vertical: VclScrollBar {
                                    flickable: completionView
                                }
                                delegate: VclLabel {
                                    width: completionView.width-(completionView.contentHeight > completionView.height ? 20 : 1)
                                    text: string
                                    padding: 5
                                    font: defaultFontMetrics.font
                                    color: index === completionView.currentIndex ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.c10.text
                                    MouseArea {
                                        property bool singleClickAutoComplete: Runtime.screenplayEditorSettings.singleClickAutoComplete
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
                                    color: Runtime.colors.primary.highlight.background
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
                            menu: VclMenu {
                                property int sceneTextEditorCursorPosition: -1
                                property SceneElement sceneCurrentElement
                                property TextFormat sceneTextFormat: sceneDocumentBinder.textFormat
                                onAboutToShow: {
                                    sceneCurrentElement = sceneDocumentBinder.currentElement
                                    sceneTextEditorCursorPosition = sceneTextEditor.cursorPosition
                                    sceneTextEditor.persistentSelection = true
                                }
                                onAboutToHide: sceneTextEditor.persistentSelection = false

                                VclMenuItem {
                                    focusPolicy: Qt.NoFocus
                                    text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
                                    enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
                                    onClicked: { sceneTextEditor.cut2(); editorContextMenu.close() }
                                }

                                VclMenuItem {
                                    focusPolicy: Qt.NoFocus
                                    text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
                                    enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
                                    onClicked: { sceneTextEditor.copy2(); editorContextMenu.close() }
                                }

                                VclMenuItem {
                                    focusPolicy: Qt.NoFocus
                                    text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
                                    enabled: sceneTextEditor.canPaste
                                    onClicked: { sceneTextEditor.paste2(); editorContextMenu.close() }
                                }

                                MenuSeparator {  }

                                VclMenuItem {
                                    focusPolicy: Qt.NoFocus
                                    text: "Split Scene"
                                    enabled: contentItem.canSplitScene
                                    onClicked: {
                                        sceneTextEditor.splitSceneAt(sceneTextEditorCursorPosition)
                                        editorContextMenu.close()
                                    }
                                }

                                VclMenuItem {
                                    focusPolicy: Qt.NoFocus
                                    text: "Join Previous Scene"
                                    enabled: contentItem.canJoinToPreviousScene
                                    onClicked: {
                                        sceneTextEditor.mergeWithPreviousScene()
                                        editorContextMenu.close()
                                    }
                                }

                                MenuSeparator {  }

                                VclMenu {
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

                                        VclMenuItem {
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

                                VclMenu {
                                    title: "Translate"
                                    enabled: sceneTextEditor.hasSelection

                                    Repeater {
                                        model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                                        VclMenuItem {
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
                        ShortcutsModelItem.group: "Edit"
                        ShortcutsModelItem.title: "Split Scene"
                        ShortcutsModelItem.shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Shift+Return" : "Ctrl+Shift+Enter"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: contentItem.canJoinToPreviousScene
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Edit"
                        ShortcutsModelItem.title: "Join Previous Scene"
                        ShortcutsModelItem.shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Shift+Delete" : "Ctrl+Shift+Backspace"
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

                        if(event.modifiers & Qt.ControlModifier && event.modifiers & Qt.ShiftModifier) {
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

                        if(event.modifiers & Qt.ControlModifier && event.modifiers & Qt.ShiftModifier) {
                            if( (Scrite.app.isMacOSPlatform && event.key === Qt.Key_Delete) || (event.key === Qt.Key_Backspace) ) {
                                event.accepted = true
                                if(sceneTextEditor.cursorPosition === 0)
                                    contentItem.mergeWithPreviousScene()
                                else
                                    contentItem.showCantMergeSceneMessage()
                            }

                            return
                        }

                        if(event.modifiers === Qt.ControlModifier) {
                            switch(event.key) {
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
                        target: Runtime.screenplayTextDocument
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
                color: Scrite.app.isVeryLightColor(contentItem.theScene.color) ? Runtime.colors.primary.highlight.background : Qt.tint(contentItem.theScene.color, "#9CFFFFFF")
                visible: Runtime.screenplayAdapter.currentIndex === contentItem.theIndex
            }

            function mergeWithPreviousScene() {
                if(Scrite.document.readOnly) {
                    event.accepted = false
                    return
                }

                if(!contentItem.canJoinToPreviousScene) {
                    showCantMergeSceneMessage()
                    return
                }
                Scrite.document.setBusyMessage("Merging scene...")
                Utils.execLater(contentItem, 100, mergeWithPreviousSceneImpl)
            }

            function mergeWithPreviousSceneImpl() {
                if(Scrite.document.readOnly) {
                    event.accepted = false
                    return
                }

                Runtime.screenplayTextDocument.syncEnabled = false
                var ret = Runtime.screenplayAdapter.mergeElementWithPrevious(contentItem.theElement)
                Runtime.screenplayTextDocument.syncEnabled = true
                Scrite.document.clearBusyMessage()
                if(ret === null)
                    showCantMergeSceneMessage()
                contentView.scrollIntoView(Runtime.screenplayAdapter.currentIndex)
            }

            function showCantMergeSceneMessage() {
                MessageBox.information("Merge Scene Error",
                    "Scene can be merged only when cursor is placed at the start of the first paragraph in a scene."
                )
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
                Runtime.screenplayTextDocument.syncEnabled = false
                postSplitElementTimer.newCurrentIndex = contentItem.theIndex+1
                var ret = Runtime.screenplayAdapter.splitElement(contentItem.theElement, sceneDocumentBinder.currentElement, sceneDocumentBinder.currentElementCursorPosition)
                Runtime.screenplayTextDocument.syncEnabled = true
                Scrite.document.clearBusyMessage()
                if(ret === null)
                    showCantSplitSceneMessage()
                else
                    postSplitElementTimer.start()
            }

            function showCantSplitSceneMessage() {
                MessageBox.information("Split Scene Error",
                    "Scene can be split only when cursor is placed at the start of a paragraph.")
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
                var idx = Runtime.screenplayAdapter.previousSceneElementIndex()
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
                var idx = Runtime.screenplayAdapter.nextSceneElementIndex()
                if(idx === Runtime.screenplayAdapter.elementCount-1 && idx === theIndex) {
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
//                    property bool headingFieldOnly: !Runtime.screenplayEditorSettings.displaySceneCharacters && !Runtime.screenplayEditorSettings.displaySceneSynopsis
//                    onHeadingFieldOnlyChanged: to = parent.mapFromItem(sceneHeadingField, 0, sceneHeadingField.height).y - height

                    SceneTypeImage {
                        width: headingFontMetrics.height
                        height: width
                        lightBackground: Scrite.app.isLightColor(headingItem.color)
                        anchors.verticalCenter: sceneNumberField.verticalCenter
                        anchors.verticalCenterOffset: -headingFontMetrics.descent
                        sceneType: headingItem.theScene.type
                    }

                    VclTextField {
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
                                 Runtime.screenplayAdapter.isSourceScreenplay
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
                anchors.verticalCenterOffset: Runtime.screenplayEditorSettings.displaySceneCharacters ? 8 : 4

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

                        VclTextField {
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
                                if(sceneHeading.enabled) {
                                    if(activeFocus)
                                        return sceneHeading.editText
                                    return sceneHeading.displayText
                                }
                                return ""
                            }
                            hoverEnabled: headingItem.theElement.omitted
                            readOnly: Scrite.document.readOnly || !(sceneHeading.enabled && !headingItem.theElement.omitted)
                            label: ""
                            placeholderText: sceneHeading.enabled ? "INT. SOMEPLACE - DAY" : "NO SCENE HEADING"
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

                                    Utils.execLater(sceneHeadingField, 50, sceneHeadingField.acctivateSceneHeadingLanguage)
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
                            singleClickAutoComplete: Runtime.screenplayEditorSettings.singleClickAutoComplete
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

                            function acctivateSceneHeadingLanguage() {
                                let headingFormat = Scrite.document.displayFormat.elementFormat(SceneElement.Heading)
                                headingFormat.activateDefaultLanguage()
                            }
                        }
                    }

                    FlatToolButton {
                        id: sceneTaggingButton
                        iconSource: "qrc:/icons/action/tag.png"
                        visible: Runtime.appFeatures.structure.enabled && Runtime.screenplayEditorSettings.allowTaggingOfScenes && Runtime.mainWindowTab === Runtime.e_ScreenplayTab
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
                                height: 400
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

                    FlatToolButton {
                        id: sceneMenuButton
                        iconSource: "qrc:/icons/navigation/menu.png"
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

                            VclMenu {
                                id: sceneMenu

                                VclMenuItem {
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

                                    VclMenuItem {
                                        text: modelData
                                        onTriggered: {
                                            Scrite.document.screenplay.currentElementIndex = headingItem.theElementIndex
                                            additionalSceneMenuItemClicked(headingItem.theScene, modelData)
                                        }
                                    }
                                }

                                MenuSeparator { }

                                VclMenuItem {
                                    text: "Copy"
                                    enabled: Runtime.screenplayAdapter.isSourceScreenplay
                                    onClicked: {
                                        Scrite.document.screenplay.clearSelection()
                                        headingItem.theElement.selected = true
                                        Scrite.document.screenplay.copySelection()
                                    }
                                }

                                VclMenuItem {
                                    text: "Paste After"
                                    enabled: Runtime.screenplayAdapter.isSourceScreenplay && Scrite.document.screenplay.canPaste
                                    onClicked: Scrite.document.screenplay.pasteAfter( headingItem.theElementIndex )
                                }

                                MenuSeparator { }

                                VclMenuItem {
                                    text: headingItem.theElement.omitted ? "Include" : "Omit"
                                    enabled: Runtime.screenplayAdapter.screenplay === Scrite.document.screenplay
                                    onClicked: {
                                        sceneMenu.close()
                                        headingItem.theElement.omitted = !headingItem.theElement.omitted
                                    }
                                }

                                VclMenuItem {
                                    text: "Remove"
                                    enabled: Runtime.screenplayAdapter.screenplay === Scrite.document.screenplay
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
                    active: Runtime.screenplayEditorSettings.displaySceneCharacters && allow
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

                VclLabel {
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
                    visible: !headingItem.theElement.omitted && !Runtime.screenplayEditorSettings.displaySceneSynopsis
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

            VclLabel {
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
                            return Runtime.colors.accent.c900
                        return editorHasActiveFocus ? Runtime.colors.accent.c600 : Runtime.colors.accent.c10
                    }
                    border.width: editorHasActiveFocus ? 0 : Math.max(0.5, 1 * zoomLevel)
                    border.color: colors.text
                    color: colors.background
                    textColor: colors.text
                    text: modelData
                    enabled: !Scrite.document.readOnly
                    topPadding: Math.max(5, 5 * zoomLevel); bottomPadding: topPadding
                    leftPadding: Math.max(10, 10 * zoomLevel); rightPadding: leftPadding
                    font.family: headingFontMetrics.font.family
                    font.capitalization: headingFontMetrics.font.capitalization
                    font.pointSize: sceneHeadingFieldsFontPointSize
                    onClicked: requestCharacterMenu(modelData, characterNameLabel)
                    onCloseRequest: {
                        if(!Scrite.document.readOnly)
                            scene.removeMuteCharacter(modelData)
                    }

                    function determineFlags() {
                        const chMute = scene.isCharacterMute(modelData)
                        closable = chMute

                        const chVisible = Runtime.screenplayEditorSettings.captureInvisibleCharacters ? (chMute || scene.isCharacterVisible(modelData)) : true
                        font.italic = !chVisible
                        opacity = chVisible ? 1 : 0.65
                    }

                    Component.onCompleted: {
                        determineFlags()
                        scene.sceneChanged.connect(determineFlags)
                        Runtime.screenplayEditorSettings.captureInvisibleCharactersChanged.connect(determineFlags)
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
                            color: Runtime.colors.accent.borderColor
                        }
                    }
                }
            }

            Image {
                source: "qrc:/icons/content/add_box.png"
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
                    if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.addMuteCharacter) {
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
                VclLabel {
                    width: parent.width * 0.9
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: "Scene headings will be listed here as you add them into your screenplay."
                    anchors.horizontalCenter: sceneListView.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 50
                    visible: Runtime.screenplayAdapter.elementCount === 0
                }

                Connections {
                    target: Scrite.document.screenplay
                    enabled: Runtime.screenplayAdapter.isSourceScreenplay
                    function onElementMoved(element, from, to) {
                        Qt.callLater(sceneListView.forceLayout)
                    }
                }

                QtObject {
                    EventFilter.target: Scrite.app
                    EventFilter.active: Runtime.screenplayAdapter.isSourceScreenplay && Scrite.document.screenplay.hasSelectedElements
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

                    ScrollBar.vertical: VclScrollBar { flickable: sceneListView }
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                    FocusTracker.window: Scrite.window
                    FocusTracker.indicator.target: Runtime.undoStack
                    FocusTracker.indicator.property: "sceneListPanelActive"

                    anchors.fill: parent

                    clip: true
                    model: Runtime.screenplayAdapter
                    currentIndex: Runtime.screenplayAdapter.currentIndex

                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    highlightFollowsCurrentItem: true

                    highlightRangeMode: ListView.ApplyRange
                    keyNavigationEnabled: false
                    preferredHighlightEnd: height*0.8
                    preferredHighlightBegin: height*0.2

                    property bool hasEpisodes: Runtime.screenplayAdapter.isSourceScreenplay ? Runtime.screenplayAdapter.screenplay.episodeCount > 0 : false

                    headerPositioning: ListView.OverlayHeader
                    header: Rectangle {
                        width: sceneListView.width-1
                        height: 40
                        z: 10
                        color: Runtime.colors.accent.windowColor

                        RowLayout {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: headingText.leftMargin
                            anchors.right: parent.right
                            anchors.rightMargin: (sceneListView.contentHeight > sceneListView.height) ? 10 : 0

                            VclText {
                                id: headingText
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                readonly property real iconWidth: 18
                                property real t: Runtime.screenplayAdapter.hasNonStandardScenes ? 1 : 0
                                property real leftMargin: 6 + (iconWidth+12)*t
                                Behavior on t {
                                    enabled: Runtime.applicationSettings.enableAnimations
                                    NumberAnimation { duration: 250 }
                                }

                                elide: Text.ElideRight
                                font.family: "Courier Prime"
                                font.pointSize: Math.ceil(Runtime.idealFontMetrics.font.pointSize * 1.2)
                                font.bold: true
                                text: Scrite.document.screenplay.title === "" ? "[#] TITLE PAGE" : Scrite.document.screenplay.title

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: headingText.truncated
                                    ToolTip.text: headingText.text
                                    ToolTip.delay: 1000
                                    ToolTip.visible: headingText.truncated && containsMouse
                                    onClicked: {
                                        if(Runtime.screenplayAdapter.isSourceScreenplay)
                                            Runtime.screenplayAdapter.screenplay.clearSelection()
                                        Runtime.screenplayAdapter.currentIndex = -1
                                        contentView.positionViewAtBeginning()
                                    }
                                }
                            }

                            VclToolButton {
                                icon.source: "qrc:/icons/content/view_options.png"
                                Layout.alignment: Qt.AlignVCenter
                                onClicked: sceneListPanelMenu.open()
                                down: sceneListPanelMenu.visible
                                ToolTip.text: "Scene List Options"

                                Item {
                                    anchors.bottom: parent.bottom
                                    width: parent.width

                                    VclMenu {
                                        id: sceneListPanelMenu

                                        VclMenu {
                                            title: "Text"

                                            VclMenuItem {
                                                text: "Scene Heading"

                                                readonly property string option: "HEADING"
                                                icon.source: Runtime.sceneListPanelSettings.sceneTextMode === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                                                onClicked: Runtime.sceneListPanelSettings.sceneTextMode = option
                                            }

                                            VclMenuItem {
                                                text: "Scene Summary"

                                                readonly property string option: "SUMMARY"
                                                icon.source: Runtime.sceneListPanelSettings.sceneTextMode === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                                                onClicked: Runtime.sceneListPanelSettings.sceneTextMode = option
                                            }

                                            MenuSeparator { }

                                            VclMenuItem {
                                                text: "Show Tooltip"

                                                icon.source: Runtime.sceneListPanelSettings.showTooltip ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                                                onClicked: Runtime.sceneListPanelSettings.showTooltip = !Runtime.sceneListPanelSettings.showTooltip
                                            }
                                        }

                                        VclMenu {
                                            title: "Length"

                                            VclMenuItem {
                                                text: "Scene Duration"

                                                readonly property string option: "TIME"
                                                icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                                                onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
                                                enabled: !Runtime.screenplayTextDocument.paused
                                            }

                                            VclMenuItem {
                                                text: "Page Length"

                                                readonly property string option: "PAGE"
                                                icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                                                onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
                                                enabled: !Runtime.screenplayTextDocument.paused
                                            }

                                            VclMenuItem {
                                                text: "None"

                                                readonly property string option: "NO"
                                                icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                                                onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
                                                enabled: !Runtime.screenplayTextDocument.paused
                                            }
                                        }                                        
                                    }
                                }
                            }
                        }
                    }

                    delegate: Rectangle {
                        id: delegateItem

                        required property int index
                        required property string id
                        required property Scene scene
                        required property int breakType
                        required property var modelData
                        required property ScreenplayElement screenplayElement
                        required property int screenplayElementType

                        property color selectedColor: Scrite.app.isVeryLightColor(scene.color) ? Qt.tint(Runtime.colors.primary.highlight.background, "#9CFFFFFF") : Qt.tint(scene.color, "#9CFFFFFF")
                        property color normalColor: Qt.tint(scene.color, "#E7FFFFFF")
                        property int elementIndex: index
                        property bool elementIsBreak: screenplayElementType === ScreenplayElement.BreakElementType
                        property bool elementIsEpisodeBreak: screenplayElementType === ScreenplayElement.BreakElementType && breakType === Screenplay.Episode
                        property bool elementIsSelected: (Runtime.screenplayAdapter.currentIndex === index || screenplayElement.selected)

                        width: sceneListView.width-1
                        height: delegateText.height + 16
                        color: scene ? elementIsSelected ? selectedColor : (Runtime.screenplayAdapter.isSourceScreenplay && Runtime.screenplayAdapter.screenplay.selectedElementsCount > 1 ? Qt.tint(normalColor, "#40FFFFFF") : normalColor)
                                     : Runtime.screenplayAdapter.currentIndex === index ? Scrite.app.translucent(Runtime.colors.accent.windowColor, 0.25) : Qt.rgba(0,0,0,0.01)

                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            visible: elementIsSelected
                            width: Runtime.screenplayAdapter.isSourceScreenplay && Runtime.screenplayAdapter.screenplay.selectedElementsCount > 1 ?
                                       (Runtime.screenplayAdapter.currentIndex === index ? 10 : 5) :
                                       8
                            color: Runtime.colors.accent.windowColor
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
                            opacity: (Runtime.screenplayAdapter.currentIndex === index ? 1 : 0.5) * t
                            visible: t > 0
                            lightBackground: Scrite.app.isLightColor(delegateItem.color)
                            property real t: Runtime.screenplayAdapter.hasNonStandardScenes ? 1 : 0
                            Behavior on t {
                                enabled: Runtime.applicationSettings.enableAnimations
                                NumberAnimation { duration: 250 }
                            }
                        }

                        RowLayout {
                            property real leftMargin: 11 + (sceneTypeImage.width+12)*sceneTypeImage.t
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                            anchors.right: parent.right
                            anchors.rightMargin: (sceneListView.contentHeight > sceneListView.height ? sceneListView.ScrollBar.vertical.width : 5) + 5
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 5

                            VclLabel {
                                id: delegateText

                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter

                                property bool textIsSceneHeading: Runtime.sceneListPanelSettings.sceneTextMode === "HEADING"

                                font.family: headingFontMetrics.font.family
                                font.bold: Runtime.screenplayAdapter.currentIndex === index || delegateItem.elementIsBreak
                                // font.pointSize: Math.ceil(Runtime.idealFontMetrics.font.pointSize*(delegateItem.elementIsBreak ? 1.2 : 1))
                                horizontalAlignment: Qt.AlignLeft
                                color: Runtime.colors.primary.c10.text
                                font.capitalization: delegateItem.elementIsBreak ||textIsSceneHeading ? Font.AllUppercase : Font.MixedCase

                                elide: textIsSceneHeading ? Text.ElideMiddle : Text.ElideRight
                                wrapMode: textIsSceneHeading ? Text.NoWrap : Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: wrapMode === Text.NoWrap ? 1 : 2

                                text: {
                                    let ret = "UNKNOWN"
                                    if(scene) {
                                        if(textIsSceneHeading) {
                                            if(scene.heading.enabled) {
                                                ret = screenplayElement.resolvedSceneNumber + ". "
                                                if(screenplayElement.omitted)
                                                    ret += "[OMITTED] <font color=\"gray\">" + scene.heading.text + "</font>"
                                                else
                                                    ret += scene.heading.text
                                            } else if(screenplayElement.omitted)
                                                ret = "[OMITTED]"
                                            else
                                                ret = "NO SCENE HEADING"
                                        } else {
                                            let summary = scene.summary
                                            if(scene.heading.enabled) {
                                                ret = screenplayElement.resolvedSceneNumber + ". "
                                                if(screenplayElement.omitted)
                                                    ret += "[OMITTED] <font color=\"gray\">" + summary + "</font>"
                                                else
                                                    ret += summary
                                            } else if(screenplayElement.omitted)
                                                ret = "[OMITTED]" + summary
                                            else
                                                ret = summary
                                        }

                                        return ret
                                    }

                                    if(delegateItem.elementIsBreak) {
                                        if(delegateItem.elementIsEpisodeBreak)
                                            ret = screenplayElement.breakTitle
                                        else if(sceneListView.hasEpisodes)
                                            ret = "Ep " + (screenplayElement.episodeIndex+1) + ": " + screenplayElement.breakTitle
                                        else
                                            ret = screenplayElement.breakTitle
                                        if(screenplayElement.breakSubtitle !== "")
                                            ret +=  ": " + screenplayElement.breakSubtitle
                                        return ret
                                    }

                                    return ret
                                }
                            }

                            Image {
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredWidth: height
                                Layout.preferredHeight: delegateText.height

                                smooth: true
                                mipmap: true
                                source: "qrc:/icons/content/empty_scene.png"
                                visible: !delegateItem.elementIsBreak && !scene.hasContent
                                fillMode: Image.PreserveAspectFit

                                MouseArea {
                                    anchors.fill: parent

                                    enabled: parent.visible
                                    hoverEnabled: enabled

                                    ToolTip.text: "This scene is empty."
                                    ToolTip.visible: containsMouse
                                }
                            }

                            VclLabel {
                                id: sceneLengthText
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize-3
                                color: Runtime.colors.primary.c10.text
                                text: evaluateText()
                                visible: !Runtime.screenplayTextDocument.paused && (Runtime.sceneListPanelSettings.displaySceneLength === "PAGE" || Runtime.sceneListPanelSettings.displaySceneLength === "TIME")
                                opacity: 0.5
                                Layout.alignment: Qt.AlignVCenter

                                function evaluateText() {
                                    if(scene) {
                                        if(Runtime.sceneListPanelSettings.displaySceneLength === "PAGE") {
                                            const pl = Runtime.screenplayTextDocument.lengthInPages(screenplayElement, null)
                                            return Math.round(pl*100,2)/100
                                        }
                                        if(Runtime.sceneListPanelSettings.displaySceneLength === "TIME")
                                            return Runtime.screenplayTextDocument.lengthInTimeAsString(screenplayElement, null)
                                    }
                                    return ""
                                }

                                function updateText() {
                                    text = evaluateText()
                                }

                                function updateTextLater() {
                                    Qt.callLater(updateText)
                                }

                                Connections {
                                    target: Runtime.screenplayTextDocument
                                    enabled: !Runtime.screenplayTextDocument.paused
                                    function onUpdateFinished() { sceneLengthText.updateTextLater() }
                                }

                                property string option: Runtime.sceneListPanelSettings.displaySceneLength
                                onOptionChanged: updateTextLater()
                            }
                        }

                        MouseArea {
                            id: delegateMouseArea

                            ToolTip.text: delegateText.text
                            ToolTip.delay: 1000
                            ToolTip.visible: Runtime.sceneListPanelSettings.showTooltip && delegateText.truncated && containsMouse

                            anchors.fill: parent

                            hoverEnabled: delegateText.truncated
                            preventStealing: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onDoubleClicked: (mouse) => {
                                                 Runtime.screenplayAdapter.screenplay.clearSelection()
                                                 screenplayElement.toggleSelection()
                                                 Runtime.screenplayAdapter.currentIndex = index
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

                                           if(Runtime.screenplayAdapter.isSourceScreenplay) {
                                               const isControlPressed = mouse.modifiers & Qt.ControlModifier
                                               const isShiftPressed = mouse.modifiers & Qt.ShiftModifier
                                               if(isControlPressed) {
                                                   screenplayElement.toggleSelection()
                                               } else if(isShiftPressed) {
                                                   const fromIndex = Math.min(Runtime.screenplayAdapter.currentIndex, index)
                                                   const toIndex = Math.max(Runtime.screenplayAdapter.currentIndex, index)
                                                   if(fromIndex === toIndex) {
                                                       screenplayElement.toggleSelection()
                                                   } else {
                                                       for(var i=fromIndex; i<=toIndex; i++) {
                                                           var element = Runtime.screenplayAdapter.screenplay.elementAt(i)
                                                           if(element.elementType === ScreenplayElement.SceneElementType) {
                                                               element.selected = true
                                                           }
                                                       }
                                                   }
                                               } else {
                                                   Runtime.screenplayAdapter.screenplay.clearSelection()
                                                   screenplayElement.toggleSelection()
                                               }
                                           }

                                           Runtime.screenplayAdapter.currentIndex = index
                                       }
                            drag.target: Runtime.screenplayAdapter.isSourceScreenplay && !Scrite.document.readOnly ? parent : null
                            drag.axis: Drag.YAxis
                        }

                        Drag.active: delegateMouseArea.drag.active
                        Drag.dragType: Drag.Automatic
                        Drag.supportedActions: Qt.MoveAction
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
                            color: Runtime.colors.primary.borderColor
                            visible: delegateDropArea.containsDrag
                        }

                        Rectangle {
                            anchors.top: dropIndicator.visible ? dropIndicator.bottom : parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 1
                            color: parent.elementIsEpisodeBreak ? Runtime.colors.accent.c200.background : Runtime.colors.accent.c100.background
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
                                           moveSelectedElementsAnimation.targetIndex = Runtime.screenplayAdapter.elementCount
                                       }
                        }

                        Rectangle {
                            width: parent.width
                            height: 2
                            color: Runtime.colors.primary.borderColor
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
        characterMenu.popupSource = popupSource
        characterMenu.characterName = characterName
        characterMenu.popup()
    }

    VclMenu {
        id: characterMenu

        property Item popupSource
        property string characterName

        width: 350

        onAboutToHide: {
            popupSource = null
            characterName = ""
        }

        Repeater {
            model: Runtime.characterReports

            VclMenuItem {
                required property var modelData

                text: modelData.name
                icon.source: "qrc" + modelData.icon

                onTriggered: ReportConfigurationDialog.launch(modelData.name, {"characterNames": [characterMenu.characterName]})
            }
        }

        Repeater {
            model: Runtime.characterReports.length > 0 ? additionalCharacterMenuItems : []

            VclMenuItem {
                required property var modelData
                text: modelData.name
                icon.source: "qrc" + modelData.icon

                onTriggered: additionalCharacterMenuItemClicked(characterMenu.characterName, modelData.name)
            }
        }

        Repeater {
            model: Runtime.characterReports.length > 0 ? 1 : 0

            VclMenuItem {
                text: "Rename/Merge Character"
                icon.source: "qrc:/icons/screenplay/character.png"

                onTriggered: {
                    const character = Scrite.document.structure.addCharacter(characterMenu.characterName)
                    if(character)
                        RenameCharacterDialog.launch(character)
                }
            }
        }
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

                    VclLabel {
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

        Runtime.screenplayEditor = screenplayEditor
        if(Runtime.mainWindowTab === Runtime.e_ScreenplayTab)
            Scrite.user.logActivity1("screenplay")

        if(Runtime.mainWindowTab === Runtime.e_ScreenplayTab && contentView.count === 1)
            contentView.itemAtIndex(0).item.assumeFocus()
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
        border.color: Runtime.colors.primary.borderColor
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
                color: componentData.scene ? Qt.rgba(0,0,0,0) : (componentData.breakType === Screenplay.Episode ? Runtime.colors.accent.c100.background : Runtime.colors.accent.c50.background)
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
                    contentView.loadAllDelegates || Runtime.screenplayEditorSettings.optimiseScrolling ||
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

    Loader {
        id: pausePaginationAnimator
        anchors.fill: parent
        active: false
        visible: active
        sourceComponent: UiElementHighlight {
            uiElement: pageCountButton
            onDone: pausePaginationAnimator.active = false
            description: "Time & Page Count: " + (Runtime.screenplayTextDocument.paused ? "Disbled" : "Enabled")
            descriptionPosition: Item.TopRight
            property bool scaleDone: false
            onScaleAnimationDone: scaleDone = true
            Component.onDestruction: {
                if(scaleDone)
                    pausePaginationAnimator.active = false
            }
        }

        Connections {
            target: Scrite.document

            function onJustLoaded() {
                if(Runtime.screenplayEditorSettings.pausePaginationForEachDocument)
                    Runtime.screenplayTextDocument.paused = true
                Utils.execLater(pausePaginationAnimator, 100, () => {
                                    pausePaginationAnimator.active = true
                                })
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
