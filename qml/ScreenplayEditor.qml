/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

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
    color: primaryColors.windowColor
    clip: true

    PropertyAlias {
        id: sourcePropertyAlias
        sourceObject: screenplayAdapter
        sourceProperty: "source"
    }

    Connections {
        target: screenplayAdapter

        function swithToCurrentIndex() {
            var currentIndex = screenplayAdapter.currentIndex
            if(currentIndex < 0) {
                contentView.scrollToFirstScene()
                return
            }

            var originIsContentView = mainTabBar.currentIndex === 0 || Scrite.app.hasActiveFocus(Scrite.window,contentView)
            if(!originIsContentView) {
                var gp = Scrite.app.cursorPosition()
                var pos = Scrite.app.mapGlobalPositionToItem(contentView,gp)
                originIsContentView = pos.x >= 0 && pos.x < contentView.width && pos.y >= 0 && pos.y < contentView.height
            }
            if(originIsContentView)
                Scrite.app.execLater(contentView, 100, function() {
                    contentView.scrollIntoView(currentIndex)
                })
            else
                contentView.positionViewAtIndex(currentIndex, ListView.Beginning)
        }

        function onCurrentIndexChanged(val) {
            swithToCurrentIndex()
            Scrite.app.execLater(screenplayEditor, 500, swithToCurrentIndex)
        }

        function onSourceChanged() {
            contentView.commentsExpandCounter = 0
            contentView.commentsExpanded = false
        }
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
            Scrite.app.execLater(screenplayAdapter.screenplay, 100, function() {
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
            enabled: screenplayEditorSettings.enableAnimations
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

        EventFilter.events: [31]
        EventFilter.onFilter: {
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
            property real leftMargin: contentView.commentsExpanded && sidePanels.expanded ? 80 : (parent.width-width)/2
            Behavior on leftMargin {
                enabled: screenplayEditorSettings.enableAnimations && contentView.commentsExpandCounter > 0
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
                        Scrite.app.execLater(postSplitElementTimer, 250, function() {
                            var item = contentView.itemAtIndex(postSplitElementTimer.newCurrentIndex)
                            if(item)
                                item.item.assumeFocus()
                            postSplitElementTimer.newCurrentIndex = -1
                        })
                    }
                }

                ListView {
                    id: contentView
                    anchors.fill: parent
                    model: contentViewModel.value
                    spacing: screenplayAdapter.elementCount > 0 ? screenplayEditorSettings.spaceBetweenScenes*zoomLevel : 0
                    property int commentsExpandCounter: 0
                    property bool commentsExpanded: false
                    property real spaceForComments: screenplayEditorSettings.displaySceneComments && commentsPanelAllowed ? ((sidePanels.expanded ? (screenplayEditorWorkspace.width - pageRulerArea.width - 80) : (screenplayEditorWorkspace.width - pageRulerArea.width)/2) - 20) : 0
                    onCommentsExpandedChanged: commentsExpandCounter = commentsExpandCounter+1
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

                    property bool allowContentYAnimation
                    Behavior on contentY {
                        enabled: screenplayEditorSettings.enableAnimations && contentView.allowContentYAnimation
                        NumberAnimation {
                            duration: 100
                            onFinished: contentView.allowContentYAnimation = false
                        }
                    }

                    delegate: Loader {
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
                        sourceComponent: componentData ? (componentData.scene ? contentComponent : (componentData.breakType === Screenplay.Episode ? episodeBreakComponent : actBreakComponent)) : noContentComponent

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

                        // Placeholder item for when scrolling is rapid.
                        Loader {
                            anchors.fill: parent
                            readonly property int spElementIndex: componentIndex
                            readonly property var spElementData: componentData
                            readonly property int spElementType: screenplayElementType

                            active: componentData.scene && !parent.active
                            sourceComponent: placeholderSceneComponent
                        }

                        property bool initialized: false
                        property bool isVisibleToUser: !contentView.moving && initialized && (index >= contentView.firstItemIndex && index <= contentView.lastItemIndex) && !contentView.ScrollBar.vertical.active
                        onIsVisibleToUserChanged: {
                            if(!active && isVisibleToUser)
                                Scrite.app.execLater(contentViewDelegateLoader, 100, load)
                        }

                        function load() {
                            if(active || componentData === undefined)
                                return
                            if(contentView.moving)
                                contentView.movingChanged.connect(load)
                            else {
                                active = true
                                Scrite.app.resetObjectProperty(contentViewDelegateLoader, "height")
                            }
                        }

                        Component.onCompleted: {
                            var editorHints = componentData.screenplayElement.editorHints
                            if( componentData.screenplayElementType === ScreenplayElement.BreakElementType ||
                                !editorHints ||
                                editorHints.displaySceneCharacters !== screenplayEditorSettings.displaySceneCharacters ||
                                editorHints.displaySceneSynopsis !== screenplayEditorSettings.displaySceneSynopsis ||
                                componentData.scene.elementCount <= 1) {
                                    active = true
                                    initialized = true
                                    return
                                }

                            height = editorHints.height * zoomLevel
                            active = false
                            initialized = true
                            Scrite.app.execLater(contentViewDelegateLoader, 400, load)
                        }

                        Component.onDestruction: {
                            if(!active || componentData.screenplayElementType === ScreenplayElement.BreakElementType)
                                return
                            var editorHints = {
                                "height": height / zoomLevel,
                                "displaySceneCharacters": screenplayEditorSettings.displaySceneCharacters,
                                "displaySceneSynopsis": screenplayEditorSettings.displaySceneSynopsis
                            }
                            componentData.screenplayElement.editorHints = editorHints
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
                    snapMode: ListView.NoSnap
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds
                    keyNavigationEnabled: false
                    ScrollBar.vertical: verticalScrollBar
                    property int numberOfWordsAddedToDict : 0
                    header: Item {
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

                        Loader {
                            id: titleCardLoader
                            active: screenplayAdapter.isSourceScreenplay && Scrite.document.screenplay.hasTitlePageAttributes
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

                            Column {
                                id: logLineEditorLayout
                                width: parent.width
                                anchors.centerIn: parent
                                spacing: 13

                                Text {
                                    id: logLineFieldHeading
                                    text: "Logline:"
                                    font.family: screenplayFormat.defaultFont2.family
                                    font.pointSize: screenplayFormat.defaultFont2.pointSize-2
                                    visible: logLineField.length > 0
                                    color: primaryColors.a700.background
                                }

                                TextArea {
                                    id: logLineField
                                    width: parent.width
                                    font: screenplayFormat.defaultFont2
                                    readOnly: Scrite.document.readOnly
                                    palette: Scrite.app.palette
                                    selectByMouse: true
                                    selectByKeyboard: true
                                    text: Scrite.document.screenplay.logline
                                    Transliterator.textDocument: textDocument
                                    Transliterator.cursorPosition: cursorPosition
                                    Transliterator.hasActiveFocus: activeFocus
                                    Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
                                    onTextChanged: Scrite.document.screenplay.logline = text
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    placeholderText: "Enter the logline of your screenplay here.."
                                }
                            }
                        }
                    }
                    footer: Item {
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

                    FocusTracker.window: Scrite.window
                    FocusTracker.indicator.target: mainUndoStack
                    FocusTracker.indicator.property: screenplayAdapter.isSourceScreenplay ? "screenplayEditorActive" : "sceneEditorActive"

                    Component.onCompleted: positionViewAtIndex(screenplayAdapter.currentIndex, ListView.Beginning)

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
                                screenplayAdapter.currentIndex = ci
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
                            contentView.contentY = pt.y
                        else
                            contentView.contentY = (pt.y + 2*rect.height) - contentView.height
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

            onValueChanged: {
                if(mainTabBar.currentIndex === 0)
                    screenplayEditorSettings.mainEditorZoomValue = value
                else
                    screenplayEditorSettings.embeddedEditorZoomValue = value
                screenplayFormat.fontZoomLevelIndex = value
            }
            Component.onCompleted: {
                var _value = -1
                if(mainTabBar.currentIndex === 0)
                    _value = screenplayEditorSettings.mainEditorZoomValue
                else
                    _value = screenplayEditorSettings.embeddedEditorZoomValue
                if(_value >= from && _value <= to)
                    value = _value
                else
                    value = screenplayFormat.fontZoomLevelIndex + zoomLevelModifier
                screenplayFormat.fontZoomLevelIndex = value
            }

            Connections {
                target: screenplayFormat
                function onFontZoomLevelIndexChanged() {
                    zoomSlider.value = screenplayFormat.fontZoomLevelIndex
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
                font.capitalization: Font.AllUppercase
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

            Text {
                id: actBreakPrefix
                font: actBreakTitle.font
                anchors.right: actBreakTitle.left
                anchors.rightMargin: ruler.leftMarginPx * 0.075
                anchors.verticalCenter: actBreakTitle.verticalCenter
                width: headingFontMetrics.averageCharacterWidth*maximumLength
                visible: parent.theElement.breakType === Screenplay.Act && screenplayAdapter.isSourceScreenplay && screenplayAdapter.screenplay.episodeCount > 0
                text: "Ep " + (parent.theElement.episodeIndex+1)
                color: actBreakTitle.color
                readonly property int maximumLength: 5
            }

            Text {
                id: actBreakTitle
                font.family: headingFontMetrics.font.family
                font.bold: true
                font.capitalization: Font.AllUppercase
                font.pointSize: headingFontMetrics.font.pointSize
                anchors.left: parent.left
                anchors.right: deleteBreakButton.left
                anchors.leftMargin: ruler.leftMarginPx
                anchors.rightMargin: 5
                topPadding: headingFontMetrics.lineSpacing*0.15
                bottomPadding: topPadding
                color:  primaryColors.c10.text
                text: parent.theElement.breakTitle
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
            border.width: 1
            border.color: screenplayElement.scene ? screenplayElement.scene.color : primaryColors.c400.background
            color: screenplayElement.scene ? Qt.tint(screenplayElement.scene.color, "#E7FFFFFF") : primaryColors.c300.background

            readonly property ScreenplayElement screenplayElement: spElementData.screenplayElement
            readonly property Scene scene: spElementData.scene
            readonly property int screenplayElementType: spElementData.screenplayElementType

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
                        return scene.heading.text
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
                anchors.topMargin: 20
                anchors.bottomMargin: 20
                fillMode: Image.TileVertically
                source: "../images/sample_scene.png"
                opacity: 0.5
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
        id: contentComponent

        Rectangle {
            id: contentItem
            property int theIndex: componentIndex
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            property bool isCurrent: theElement === screenplayAdapter.currentElement

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
                cursorPosition: sceneTextEditor.activeFocus ? sceneTextEditor.cursorPosition : -1
                characterNames: Scrite.document.structure.characterNames
                screenplayFormat: screenplayEditor.screenplayFormat
                forceSyncDocument: !sceneTextEditor.activeFocus
                spellCheckEnabled: !Scrite.document.readOnly && spellCheckEnabledFlag.value
                liveSpellCheckEnabled: sceneTextEditor.activeFocus
                onDocumentInitialized: sceneTextEditor.cursorPosition = 0
                onRequestCursorPosition: Scrite.app.execLater(contentItem, 100, function() { contentItem.assumeFocusAt(position) })
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
                onSpellingMistakesDetected: refreshNoticeBoxLoader.showSpellCheckNotice()

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
                property color theSceneDarkColor: Scrite.app.isLightColor(contentItem.theScene.color) ? "black" : contentItem.theScene.color
                buttonColor: expanded ? Qt.tint(contentItem.theScene.color, "#C0FFFFFF") : Qt.tint(contentItem.theScene.color, "#D7EEEEEE")
                backgroundColor: buttonColor
                borderColor: expanded ? primaryColors.borderColor : (contentView.spacing > 0 ? Scrite.app.translucent(theSceneDarkColor,0.25) : Qt.rgba(0,0,0,0))
                borderWidth: contentItem.isCurrent ? 2 : 1
                anchors.top: parent.top
                anchors.left: parent.right

                property real screenY: screenplayEditor.mapFromItem(parent, 0, 0).y
                property real maxTopMargin: contentItem.height-height-20
                anchors.topMargin: screenY < 0 ? Math.min(-screenY,maxTopMargin) : -1

                Connections {
                    target: contentView
                    function onContentYChanged() {
                        commentsSidePanel.screenY = screenplayEditor.mapFromItem(commentsSidePanel.parent, 0, 0).y
                    }
                }

                // anchors.leftMargin: expanded ? 0 : -minPanelWidth
                label: expanded && anchors.topMargin > 0 ? ("Scene " + contentItem.theElement.resolvedSceneNumber + " Comments") : ""
                height: {
                    if(expanded) {
                        if(contentItem.isCurrent)
                            return contentInstance ? Math.min(contentItem.height, Math.max(contentInstance.contentHeight+60, 350)) : 300
                        return Math.min(300, parent.height)
                    }
                    return sceneHeadingAreaLoader.height
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
                    enabled: screenplayEditorSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }
                content: TextArea {
                    id: commentsEdit
                    background: Rectangle {
                        color: Qt.tint(contentItem.theScene.color, "#E7FFFFFF")
                    }
                    font.pointSize: Scrite.app.idealFontPointSize + 1
                    onTextChanged: contentItem.theScene.comments = text
                    wrapMode: Text.WordWrap
                    text: contentItem.theScene.comments
                    selectByMouse: true
                    selectByKeyboard: true
                    leftPadding: 10
                    rightPadding: 10
                    topPadding: 10
                    bottomPadding: 10
                    readOnly: Scrite.document.readOnly
                    onActiveFocusChanged: {
                        if(activeFocus)
                            screenplayAdapter.currentIndex = contentItem.theIndex
                    }

                    Transliterator.textDocument: textDocument
                    Transliterator.cursorPosition: cursorPosition
                    Transliterator.hasActiveFocus: activeFocus
                    Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts

                    SpecialSymbolsSupport {
                        anchors.top: parent.bottom
                        anchors.left: parent.left
                        textEditor: commentsEdit
                        textEditorHasCursorInterface: true
                        enabled: !Scrite.document.readOnly
                    }

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

                        TextArea {
                            id: synopsisEditorField
                            width: parent.width
                            font.pointSize: sceneHeadingFieldsFontPointSize
                            readOnly: Scrite.document.readOnly
                            palette: Scrite.app.palette
                            selectByMouse: true
                            selectByKeyboard: true
                            text: contentItem.theScene.title
                            Transliterator.textDocument: textDocument
                            Transliterator.cursorPosition: cursorPosition
                            Transliterator.hasActiveFocus: activeFocus
                            Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
                            onTextChanged: contentItem.theScene.title = text
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            placeholderText: "Enter the synopsis of your scene here."
                            background: Item { }
                            onActiveFocusChanged: {
                                if(activeFocus) {
                                    contentView.ensureVisible(synopsisEditorField, Qt.rect(0, -10, cursorRectangle.width, cursorRectangle.height+20))
                                    screenplayAdapter.currentIndex = contentItem.theIndex
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
                            visible: sceneTextEditor.cursorVisible && sceneTextEditor.activeFocus && screenplayEditorSettings.highlightCurrentLine
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
                            completionModel.allowEnable = true
                            contentView.ensureVisible(sceneTextEditor, cursorRectangle)
                            screenplayAdapter.currentIndex = contentItem.theIndex
                            globalScreenplayEditorToolbar.sceneEditor = contentItem
                            justReceivedFocus = true
                        } else if(globalScreenplayEditorToolbar.sceneEditor === contentItem)
                            globalScreenplayEditorToolbar.sceneEditor = null
                    }

                    function reload() {
                        Scrite.app.execLater(sceneDocumentBinder, 1000, function() {
                            sceneDocumentBinder.preserveScrollAndReload()
                        } )
                    }

                    onCursorRectangleChanged: {
                        if(activeFocus /*&& contentView.isVisible(contentItem.theIndex)*/) {
                            var buffer = Math.max(contentView.height * 0.2, cursorRectangle.height*3)
                            var cr = Qt.rect(cursorRectangle.x, cursorRectangle.y-buffer*0.3,
                                             cursorRectangle.width, buffer)
                            contentView.allowContentYAnimation = true
                            contentView.ensureVisible(sceneTextEditor, cr)
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
                            completionModel.allowEnable = event.hasText
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
                    Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
                    Transliterator.onAboutToTransliterate: {
                        contentItem.theScene.beginUndoCapture(false)
                        contentItem.theScene.undoRedoEnabled = false
                    }
                    Transliterator.onFinishedTransliterating: {
                        Scrite.app.execLater(Transliterator, 0, function() {
                            contentItem.theScene.endUndoCapture()
                            contentItem.theScene.undoRedoEnabled = true
                        })
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

                        CompletionModel {
                            id: completionModel
                            property bool allowEnable: true
                            property string suggestion: currentCompletion
                            property bool hasSuggestion: count > 0
                            enabled: allowEnable && sceneTextEditor.activeFocus
                            strings: sceneDocumentBinder.autoCompleteHints
                            sortStrings: false
                            completionPrefix: sceneDocumentBinder.completionPrefix
                            filterKeyStrokes: sceneTextEditor.activeFocus
                            onRequestCompletion: {
                                sceneTextEditor.acceptCompletionSuggestion()
                                Announcement.shout("E69D2EA0-D26D-4C60-B551-FD3B45C5BE60", contentItem.theScene.id)
                            }
                            minimumCompletionPrefixLength: 0
                            property bool hasItems: count > 0
                            onHasItemsChanged: {
                                if(hasItems)
                                    completionViewPopup.open()
                                else
                                    completionViewPopup.close()
                            }
                        }

                        Popup {
                            id: completionViewPopup
                            x: -Scrite.app.boundingRect(completionModel.completionPrefix, defaultFontMetrics.font).width
                            y: parent.height
                            width: Scrite.app.largestBoundingRect(completionModel.strings, defaultFontMetrics.font).width + leftInset + rightInset + leftPadding + rightPadding + 20
                            height: completionView.contentHeight + topInset + bottomInset + topPadding + bottomPadding
                            focus: false
                            closePolicy: Popup.NoAutoClose
                            contentItem: ListView {
                                id: completionView
                                model: completionModel
                                FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                                interactive: false
                                delegate: Text {
                                    width: completionView.width-1
                                    text: string
                                    padding: 5
                                    font: defaultFontMetrics.font
                                    color: index === completionView.currentIndex ? primaryColors.highlight.text : primaryColors.c10.text
                                }
                                highlight: Rectangle {
                                    color: primaryColors.highlight.background
                                }
                                currentIndex: completionModel.currentRow
                                height: contentHeight
                            }
                        }

                        // Context menus must ideally show up directly below the cursor
                        // So, we keep the menu loaders inside the cursorOverlay
                        MenuLoader {
                            id: spellingSuggestionsMenu
                            anchors.bottom: parent.bottom
                            enabled: !Scrite.document.readOnly

                            function replace(cursorPosition, suggestion) {
                                sceneDocumentBinder.replaceWordAt(cursorPosition, suggestion)
                                sceneDocumentBinder.preserveScrollAndReload()
                                if(cursorPosition >= 0)
                                    sceneTextEditor.cursorPosition = cursorPosition
                            }

                            menu: Menu2 {
                                property int cursorPosition: -1
                                onAboutToShow: {
                                    cursorPosition = sceneTextEditor.cursorPosition
                                    sceneTextEditor.persistentSelection = true
                                }
                                onAboutToHide: {
                                    sceneTextEditor.persistentSelection = false
                                    sceneTextEditor.forceActiveFocus()
                                    sceneTextEditor.cursorPosition = cursorPosition
                                }

                                Repeater {
                                    model: sceneDocumentBinder.spellingSuggestions

                                    MenuItem2 {
                                        text: modelData
                                        focusPolicy: Qt.NoFocus
                                        onClicked: {
                                            Qt.callLater(function() {
                                                spellingSuggestionsMenu.replace(cursorPosition, modelData)
                                            })
                                            spellingSuggestionsMenu.close()
                                        }
                                    }
                                }

                                MenuSeparator { }

                                MenuItem2 {
                                    text: "Add to dictionary"
                                    focusPolicy: Qt.NoFocus
                                    onClicked: {
                                        spellingSuggestionsMenu.close()
                                        sceneDocumentBinder.addWordUnderCursorToDictionary()
                                        ++contentView.numberOfWordsAddedToDict
                                    }
                                }

                                MenuItem2 {
                                    text: "Ignore"
                                    focusPolicy: Qt.NoFocus
                                    onClicked: {
                                        spellingSuggestionsMenu.close()
                                        sceneDocumentBinder.addWordUnderCursorToIgnoreList()
                                        ++contentView.numberOfWordsAddedToDict
                                    }
                                }
                            }
                        }

                        MenuLoader {
                            id: editorContextMenu
                            anchors.bottom: parent.bottom
                            enabled: !Scrite.document.readOnly
                            menu: Menu2 {
                                property int sceneTextEditorCursorPosition: -1
                                onAboutToShow: {
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
                                            enabled: sceneDocumentBinder.currentElement !== null
                                            onClicked: {
                                                sceneDocumentBinder.currentElement.type = modelData.value
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
                            remove(sceneDocumentBinder.currentBlockPosition(), cursorPosition)
                            insert(cursorPosition, suggestion)
                            userIsTyping = true
                            Transliterator.enableFromNextWord()
                            completionModel.allowEnable = false
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
                            mouse.accept = true
                            sceneTextEditor.persistentSelection = true
                            if(!sceneTextEditor.hasSelection && sceneDocumentBinder.spellCheckEnabled) {
                                sceneTextEditor.cursorPosition = sceneTextEditor.positionAt(mouse.x, mouse.y)
                                if(sceneDocumentBinder.wordUnderCursorIsMisspelled) {
                                    spellingSuggestionsMenu.popup()
                                    return
                                }
                            }
                            editorContextMenu.popup()
                        }

                        DelayedPropertyBinder {
                            id: contextMenuEnableBinder
                            initial: false
                            set: !editorContextMenu.active && !spellingSuggestionsMenu.active && sceneTextEditor.activeFocus
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

                        function highlightSearchResultTextSnippet() {
                            if(selection.start >= 0 && selection.end >= 0) {
                                if(sceneTextEditor.selectionStart === selection.start && sceneTextEditor.selectionEnd === selection.end )
                                    return;

                                var rect = Scrite.app.uniteRectangles( sceneTextEditor.positionToRectangle(selection.start),
                                                               sceneTextEditor.positionToRectangle(selection.end) )
                                rect = Scrite.app.adjustRectangle(rect, -20, -50, 20, 50)
                                contentView.ensureVisible(contentItem, rect)

                                sceneTextEditor.select(selection.start, selection.end)
                                sceneTextEditor.update()
                            } else {
                                sceneTextEditor.deselect()
                            }
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
                color: Qt.tint(contentItem.theScene.color, "#A7FFFFFF")
                visible: screenplayAdapter.currentIndex === contentItem.theIndex
            }

            function mergeWithPreviousScene() {
                if(!contentItem.canJoinToPreviousScene) {
                    showCantMergeSceneMessage()
                    return
                }
                Scrite.document.setBusyMessage("Merging scene...")
                Scrite.app.execLater(contentItem, 100, mergeWithPreviousSceneImpl)
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
                Scrite.app.execLater(contentItem, 100, splitSceneImpl)
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

            function scrollToPreviousScene() {
                var idx = screenplayAdapter.previousSceneElementIndex()
                if(idx === 0 && idx === theIndex) {
                    contentView.scrollToFirstScene()
                    assumeFocusAt(0)
                    return
                }

                contentView.scrollIntoView(idx)
                Qt.callLater( function(iidx) {
                    //contentView.positionViewAtIndex(iidx, ListView.Contain)
                    var item = contentView.loadedItemAtIndex(iidx)
                    item.assumeFocusAt(-1)
                }, idx)
            }

            function scrollToNextScene() {
                var idx = screenplayAdapter.nextSceneElementIndex()
                if(idx === screenplayAdapter.elementCount-1 && idx === theIndex) {
                    contentView.positionViewAtEnd()
                    assumeFocusAt(-1)
                    return
                }

                contentView.scrollIntoView(idx)
                Qt.callLater( function(iidx) {
                    //contentView.positionViewAtIndex(iidx, ListView.Contain)
                    var item = contentView.loadedItemAtIndex(iidx)
                    item.assumeFocusAt(0)
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
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width * 0.075
                    anchors.top: parent.top
                    anchors.topMargin: parent.mapFromItem(sceneHeadingField, 0, sceneHeadingField.height).y - height
                    spacing: 20
//                    property bool headingFieldOnly: !screenplayEditorSettings.displaySceneCharacters && !screenplayEditorSettings.displaySceneSynopsis
//                    onHeadingFieldOnlyChanged: to = parent.mapFromItem(sceneHeadingField, 0, sceneHeadingField.height).y - height

                    SceneTypeImage {
                        width: headingFontMetrics.height
                        height: width
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
                        onActiveFocusChanged: screenplayAdapter.currentIndex = headingItem.theElementIndex
                        tabItem: headingItem.sceneTextEditor
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

                            text: sceneHeading.text
                            enabled: sceneHeading.enabled
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
                            color: headingFontMetrics.format.textColor
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            readOnly: Scrite.document.readOnly
                            background: Item { }
                            onEditingComplete: sceneHeading.parseFrom(text)
                            onActiveFocusChanged: {
                                if(activeFocus)
                                    screenplayAdapter.currentIndex = headingItem.theElementIndex
                                else
                                    sceneHeading.parseFrom(text)
                            }
                            tabItem: headingItem.sceneTextEditor

                            enableTransliteration: true
                            property var currentLanguage: Scrite.app.transliterationEngine.language

                            property int dotPosition: text.indexOf(".")
                            property int dashPosition: text.lastIndexOf("-")
                            property bool editingLocationTypePart: dotPosition < 0 || cursorPosition < dotPosition
                            property bool editingMomentPart: dashPosition > 0 && cursorPosition >= dashPosition
                            property bool editingLocationPart: dotPosition > 0 ? (cursorPosition >= dotPosition && (dashPosition < 0 ? true : cursorPosition < dashPosition)) : false
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
                                    return text.substring(0, dotPosition).trim()
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
                                }

                                Repeater {
                                    model: additionalSceneMenuItems.length ? 1 : 0

                                    MenuSeparator { }
                                }

                                Repeater {
                                    model: additionalSceneMenuItems

                                    MenuItem2 {
                                        text: modelData
                                        onTriggered: {
                                            Scrite.document.screenplay.currentElementIndex = headingItem.theElementIndex
                                            additionalSceneMenuItemClicked(headingItem.theScene, modelData)
                                        }
                                    }
                                }

                                Repeater {
                                    model: additionalSceneMenuItems.length ? 1 : 0

                                    MenuSeparator { }
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
                    sourceComponent: sceneCharactersList

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
                                Scrite.app.execLater(sceneCharactersListLoader, 250, function() {
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
                    visible: sceneCharactersListLoader.active && headingItem.theScene.groups.length > 0
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
                    visible: !screenplayEditorSettings.displaySceneSynopsis
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
                    font.pixelSize: 16
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
                                enabled: screenplayEditorSettings.enableAnimations
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
                            font.capitalization: Font.AllUppercase
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
                        color: scene ? Qt.tint(scene.color, (screenplayElement.selected ? "#9CFFFFFF" : "#E7FFFFFF"))
                                     : screenplayAdapter.currentIndex === index ? Scrite.app.translucent(accentColors.windowColor, 0.25) : Qt.rgba(0,0,0,0.01)

                        property int elementIndex: index
                        property bool elementIsBreak: screenplayElementType === ScreenplayElement.BreakElementType
                        property bool elementIsEpisodeBreak: screenplayElementType === ScreenplayElement.BreakElementType && breakType === Screenplay.Episode

                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            visible: screenplayElement.selected
                            width: 5
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
                            property real t: screenplayAdapter.hasNonStandardScenes ? 1 : 0
                            Behavior on t {
                                enabled: screenplayEditorSettings.enableAnimations
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
                            horizontalAlignment: parent.elementIsBreak & !sceneListView.hasEpisodes ? Qt.AlignHCenter : Qt.AlignLeft
                            color: primaryColors.c10.text
                            font.capitalization: Font.AllUppercase
                            text: {
                                if(scene && scene.heading.enabled)
                                    return screenplayElement.resolvedSceneNumber + ". " + scene.heading.text
                                if(parent.elementIsBreak) {
                                    if(parent.elementIsEpisodeBreak)
                                        return screenplayElement.breakTitle + ": " + screenplayElement.breakSubtitle
                                    if(sceneListView.hasEpisodes)
                                        return "Ep " + (screenplayElement.episodeIndex+1) + ": " + screenplayElement.breakTitle
                                    return screenplayElement.breakTitle
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
                            onClicked: (mouse) => {
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

                                           navigateToScreenplayElement()
                                           Scrite.app.execLater(delegateMouseArea, 500, navigateToScreenplayElement)
                                       }
                            drag.target: screenplayAdapter.isSourceScreenplay && !Scrite.document.readOnly ? parent : null
                            drag.axis: Drag.YAxis

                            function navigateToScreenplayElement() {
                                contentView.positionViewAtIndex(index, ListView.Beginning)
                                screenplayAdapter.currentIndex = index
                            }
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
                                screenplayAdapter.currentIndex = targetndex

                                sceneListView.forceActiveFocus()
                            }
                        }
                    }
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

        Column {
            property int defaultFontSize: screenplayFormat.defaultFont2.pointSize
            property real maxWidth: parent.width - 2*ruler.leftMarginPx
            spacing: 10 * zoomLevel

            Item { width: parent.width; height: 35 * zoomLevel }

            Image {
                width: {
                    switch(Scrite.document.screenplay.coverPagePhotoSize) {
                    case Screenplay.SmallCoverPhoto:
                        return parent.maxWidth / 4
                    case Screenplay.MediumCoverPhoto:
                        return parent.maxWidth / 2
                    }
                    return parent.maxWidth
                }

                source: visible ? "file:///" + Scrite.document.screenplay.coverPagePhoto : ""
                visible: Scrite.document.screenplay.coverPagePhoto !== ""
                smooth: true; mipmap: true
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item { width: parent.width; height: Scrite.document.screenplay.coverPagePhoto !== "" ? 20 * zoomLevel : 0 }

            Text {
                font.family: Scrite.document.formatting.defaultFont.family
                font.pointSize: defaultFontSize + 2
                font.bold: true
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: Scrite.document.screenplay.title === "" ? "<untitled>" : Scrite.document.screenplay.title
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
            }

            Text {
                font.family: Scrite.document.formatting.defaultFont.family
                font.pointSize: defaultFontSize
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: Scrite.document.screenplay.subtitle
                visible: Scrite.document.screenplay.subtitle !== ""
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
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
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: (Scrite.document.screenplay.author === "" ? "<unknown author>" : Scrite.document.screenplay.author)
                    anchors.horizontalCenter: parent.horizontalCenter
                    leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                    rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
                }
            }

            Text {
                font.family: Scrite.document.formatting.defaultFont.family
                font.pointSize: defaultFontSize
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: Scrite.document.screenplay.version === "" ? "Initial Version" : Scrite.document.screenplay.version
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
            }

            Text {
                font.family: Scrite.document.formatting.defaultFont.family
                font.pointSize: defaultFontSize
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: Scrite.document.screenplay.basedOn
                visible: Scrite.document.screenplay.basedOn !== ""
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
            }

            Column {
                spacing: parent.spacing/2
                width: parent.width * 0.5
                anchors.left: parent.left
                anchors.leftMargin: ruler.leftMarginPx

                Item {
                    width: parent.width
                    height: 20 * zoomLevel
                }

                Text {
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: Scrite.document.screenplay.contact
                    visible: text !== ""
                }

                Text {
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: Scrite.document.screenplay.address
                    visible: text !== ""
                }

                Text {
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: Scrite.document.screenplay.phoneNumber
                    visible: text !== ""
                }

                Text {
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    font.underline: true
                    color: "blue"
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: Scrite.document.screenplay.email
                    visible: text !== ""

                    MouseArea {
                        anchors.fill: parent
                        onClicked: Qt.openUrlExternally("mailto:" + parent.text)
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                Text {
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    font.underline: true
                    color: "blue"
                    width: parent.width
                    elide: Text.ElideRight
                    text: Scrite.document.screenplay.website
                    visible: text !== ""

                    MouseArea {
                        anchors.fill: parent
                        onClicked: Qt.openUrlExternally(parent.text)
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }

            Item { width: parent.width; height: 35 * zoomLevel }
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

    Loader {
        id: refreshNoticeBoxLoader
        anchors.fill: parent
        active: false

        property int intent: -1 // 0 = for language, 1 = for spell check

        function showLanguageNotice() {
            if(!screenplayEditorSettings.showLanguageRefreshNoticeBox)
                return

            var timestamp = (new Date()).getTime()
            if(timestamp - screenplayEditorSettings.lastLanguageRefreshNoticeBoxTimestamp < 60*1000)
                return

            screenplayEditorSettings.lastLanguageRefreshNoticeBoxTimestamp = timestamp

            intent = 0
            show()
        }

        function showSpellCheckNotice() {
            if(!screenplayEditorSettings.showSpellCheckRefreshNoticeBox)
                return

            var timestamp = (new Date()).getTime()
            if(timestamp - screenplayEditorSettings.lastSpellCheckRefreshNoticeBoxTimestamp < 60*1000)
                return

            screenplayEditorSettings.lastSpellCheckRefreshNoticeBoxTimestamp = timestamp

            intent = 1
            show()
        }

        function show() {
            if(!contentView.FocusTracker.hasFocus)
                return

            active = true
        }

        property bool writingInEnglishLanguage: Scrite.app.transliterationEngine.language === TransliterationEngine.English
        onWritingInEnglishLanguageChanged: {
            if(!writingInEnglishLanguage)
                showLanguageNotice()
        }

        sourceComponent: Rectangle {
            color: Scrite.app.translucent(accentColors.c50.background, 0.8)

            BoxShadow {
                anchors.fill: refreshNoticeBox
            }

            MouseArea {
                anchors.fill: parent
            }

            Rectangle {
                id: refreshNoticeBox
                color: "white"
                anchors.centerIn: parent
                width: Math.min(parent.width*0.8, 640)
                height: Math.min(parent.width*0.8, 480)

                Column {
                    anchors.centerIn: parent
                    width: parent.width * 0.75
                    spacing: 20

                    Image {
                        source: "../icons/navigation/refresh.png"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        font.pointSize: Scrite.app.idealFontPointSize + 2
                        text: {
                            var prefix = ""
                            if(refreshNoticeBoxLoader.intent === 0)
                                prefix = "While typing in " + Scrite.app.transliterationEngine.languageAsString + ", sometimes <b>paragraphs may overlap</b> and render inaccurately. "
                            else
                                prefix = "Sometimes entire paragraphs or even empty paragraphs may be flagged as a spelling mistake. "
                            return prefix + "This is a <i>known issue</i> and we are trying to fix it.<br/><br/>In the meantime, if you notice it please hit the <b>Refresh Icon</b> on the toolbar or press the <b>F5 key</b>, while the cursor is blinking on the scene."
                        }
                    }

                    Column {
                        id: dismissControls
                        spacing: 5
                        enabled: false
                        opacity: enabled ? 1 : 0.5
                        anchors.horizontalCenter: parent.horizontalCenter

                        CheckBox2 {
                            id: dontShowCheckBox
                            text: "Do not show this message again."
                            checked: false
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Button2 {
                            text: "Dismiss"
                            anchors.horizontalCenter: parent.horizontalCenter
                            onClicked: {
                                if(refreshNoticeBoxLoader.intent === 0)
                                    screenplayEditorSettings.showLanguageRefreshNoticeBox = !dontShowCheckBox.checked
                                else
                                    screenplayEditorSettings.showSpellCheckRefreshNoticeBox = !dontShowCheckBox.checked
                                refreshNoticeBoxLoader.active = false
                            }
                        }
                    }

                    Text {
                        text: "Enabling dismiss button in " + dismissTimer.nrSecondsRemaining + " seconds."
                        font.pointSize: Scrite.app.idealFontPointSize-2
                        visible: dismissTimer.nrSecondsRemaining > 0
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                }

                Timer {
                    id: dismissTimer
                    interval: 1000
                    running: nrSecondsRemaining >= 0
                    property int nrSecondsRemaining: 3
                    onTriggered: {
                        nrSecondsRemaining = nrSecondsRemaining-1
                        dismissControls.enabled = nrSecondsRemaining <= 0
                    }
                }
            }
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
}
