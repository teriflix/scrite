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

import Scrite 1.0
import QtQuick 2.13
import QtQuick.Window 2.13
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13

Rectangle {
    // This editor has to specialize in rendering scenes within a ScreenplayAdapter
    // The adapter may contain a single scene or an entire screenplay, that doesnt matter.
    // This way we can avoid having a SceneEditor and ScreenplayEditor as two distinct
    // QML components.

    id: screenplayEditor
    property ScreenplayFormat screenplayFormat: scriteDocument.displayFormat
    property ScreenplayPageLayout pageLayout: screenplayFormat.pageLayout
    property alias source: screenplayAdapter.source
    property bool toolBarVisible: toolbar.visible
    property bool commentsPanelAllowed: true
    property var additionalCharacterMenuItems: []
    property var additionalSceneMenuItems: []
    signal additionalCharacterMenuItemClicked(string characterName, string menuItemName)
    signal additionalSceneMenuItemClicked(Scene scene, string menuItemName)

    property alias zoomLevel: zoomSlider.zoomLevel
    property int zoomLevelModifier: 0
    color: primaryColors.windowColor
    border.width: 1
    border.color: primaryColors.borderColor
    clip: true

    ScreenplayAdapter {
        id: screenplayAdapter
        source: scriteDocument.loading ? null : scriteDocument.screenplay
        onCurrentIndexChanged: {
            if(currentIndex < 0) {
                contentView.scrollToFirstScene()
                return
            }
            if(mainUndoStack.screenplayEditorActive || mainUndoStack.sceneEditorActive)
                app.execLater(contentView, 100, function() {
                    contentView.scrollIntoView(currentIndex)
                })
            else
                contentView.positionViewAtIndex(currentIndex, ListView.Beginning)
        }
        onSourceChanged: {
            globalScreenplayEditorToolbar.showScreenplayPreview = false
            contentView.commentsExpandCounter = 0
            contentView.commentsExpanded = false
        }
    }

    ScreenplayTextDocument {
        id: screenplayTextDocument
        screenplay: scriteDocument.loading ? null : screenplayAdapter.screenplay
        formatting: scriteDocument.loading ? null : scriteDocument.printFormat
        syncEnabled: true
        secondsPerPage: scriteDocument.printFormat.secondsPerPage
        Component.onCompleted: globalTimeDisplay.screenplayTextDocument = screenplayTextDocument
    }

    // Ctrl+Shift+N should result in the newly added scene to get keyboard focus
    Connections {
        target: screenplayAdapter.isSourceScreenplay ? scriteDocument : null
        ignoreUnknownSignals: true
        onNewSceneCreated: {
            app.execLater(screenplayAdapter.screenplay, 100, function() {
                contentView.positionViewAtIndex(screenplayIndex, ListView.Visible)
                var item = contentView.loadedItemAtIndex(screenplayIndex)
                item.assumeFocus()
            })
        }
        onLoadingChanged: zoomSlider.reset()
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
        opacity: globalScreenplayEditorToolbar.showFindAndReplace ? 1 : 0
        Behavior on opacity {
            enabled: screenplayEditorSettings.enableAnimations
            NumberAnimation { duration: 100 }
        }

        SearchBar {
            id: screenplaySearchBar
            searchEngine.objectName: "Screenplay Search Engine"
            anchors.horizontalCenter: parent.horizontalCenter
            allowReplace: !scriteDocument.readOnly
            width: toolbar.width * 0.6
            enabled: !screenplayPreview.active

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
                anchors.top: ruler.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.topMargin: 5
                color: "white"

                TrackerPack {
                    id: trackerPack
                    property int counter: 0
                    TrackProperty { target: screenplayEditorSettings; property: "displaySceneCharacters" }
                    TrackProperty { target: screenplayAdapter; property: "elementCount" }
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

                ListView {
                    id: contentView
                    anchors.fill: parent
                    model: contentViewModel.value
                    property int commentsExpandCounter: 0
                    property bool commentsExpanded: false
                    property real spaceForComments: screenplayEditorSettings.displaySceneComments && commentsPanelAllowed ? ((sidePanels.expanded ? (screenplayEditorWorkspace.width - pageRulerArea.width - 80) : (screenplayEditorWorkspace.width - pageRulerArea.width)/2) - 20) : 0
                    onCommentsExpandedChanged: commentsExpandCounter = commentsExpandCounter+1
                    delegate: Loader {
                        id: contentViewDelegateLoader
                        property var componentData: modelData

                        z: contentViewModel.value.currentIndex === index ? 2 : 1
                        width: contentView.width

                        active: false
                        sourceComponent: modelData.scene ? contentComponent : breakComponent

                        /*
                        Profiler.context: "ScreenplayEditorContentDelegate"
                        Profiler.active: true
                        onStatusChanged: {
                            if(status === Loader.Ready)
                                Profiler.active = false
                        }
                        */

                        property bool initialized: false
                        property bool isVisibleToUser: !contentView.moving && initialized && (index >= contentView.firstItemIndex && index <= contentView.lastItemIndex)
                        onIsVisibleToUserChanged: {
                            if(!active && isVisibleToUser)
                                Qt.callLater(load)
                        }

                        function load() {
                            if(active)
                                return
                            active = true
                            app.resetObjectProperty(contentViewDelegateLoader, "height")
                        }

                        Rectangle {
                            anchors.fill: parent
                            visible: !parent.active
                            border.width: 1
                            border.color: scene.color
                            color: modelData.screenplayElement.scene ? Qt.tint(modelData.screenplayElement.scene.color, "#E7FFFFFF") : primaryColors.c300.background

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

                        Component.onCompleted: {
                            var editorHints = componentData.screenplayElement.editorHints
                            if( componentData.screenplayElementType === ScreenplayElement.BreakElementType ||
                                !editorHints ||
                                editorHints.displaySceneCharacters !== screenplayEditorSettings.displaySceneCharacters ||
                                editorHints.displaySceneSynopsis !== screenplayEditorSettings.displaySceneSynopsis) {
                                active = true
                                initialized = true
                                return
                            }

                            height = editorHints.height * zoomLevel
                            active = false
                            initialized = true
                            app.execLater(contentViewDelegateLoader, 2000, load)
                        }

                        Component.onDestruction: {
                            if(!active || componentData.screenplayElementType === ScreenplayElement.BreakElementType )
                                return
                            var editorHints = {
                                "height": height / zoomLevel,
                                "displaySceneCharacters": screenplayEditorSettings.displaySceneCharacters,
                                "displaySceneSynopsis": screenplayEditorSettings.displaySceneSynopsis
                            }
                            componentData.screenplayElement.editorHints = editorHints
                        }
                    }
                    snapMode: ListView.NoSnap
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds
                    ScrollBar.vertical: verticalScrollBar
                    property int numberOfWordsAddedToDict : 0
                    header: Item {
                        width: contentView.width
                        height: logLineEditor.contentHeight + (screenplayAdapter.isSourceScreenplay ? (titleCardLoader.active ? titleCardLoader.height : ruler.topMarginPx) : 0)
                        property real padding: width * 0.1

                        function editTitlePage(source) {
                            modalDialog.arguments = {"activeTabIndex": 2}
                            modalDialog.popupSource = source
                            modalDialog.sourceComponent = optionsDialogComponent
                            modalDialog.active = true
                        }

                        Loader {
                            id: titleCardLoader
                            active: screenplayAdapter.isSourceScreenplay && scriteDocument.screenplay.hasTitlePageAttributes
                            sourceComponent: titleCardComponent
                            anchors.left: parent.left
                            anchors.right: parent.right

                            ToolButton3 {
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.rightMargin: ruler.rightMarginPx
                                iconSource: "../icons/action/edit.png"
                                onClicked: editTitlePage(this)
                                visible: parent.active && enabled
                                enabled: !scriteDocument.readOnly
                            }
                        }

                        ToolButton2 {
                            id: editTitlePageButton
                            text: "Edit Title Page"
                            icon.source: "../icons/action/edit.png"
                            flat: false
                            width: implicitWidth * 1.5
                            height: implicitHeight * 1.25
                            visible: screenplayAdapter.isSourceScreenplay && titleCardLoader.active === false && enabled
                            opacity: hovered ? 1 : 0.75
                            anchors.top: parent.top
                            anchors.topMargin: (ruler.topMarginPx-height)/2
                            anchors.horizontalCenter: parent.horizontalCenter
                            onClicked: editTitlePage(this)
                            enabled: !scriteDocument.readOnly
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
                            visible: screenplayAdapter.isSourceScreenplay

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
                                    height: Math.max(contentHeight+defaultFontMetrics.height, 2*logLineFieldHeading.height)
                                    font: screenplayFormat.defaultFont2
                                    readOnly: scriteDocument.readOnly
                                    palette: app.palette
                                    selectByMouse: true
                                    selectByKeyboard: true
                                    text: scriteDocument.screenplay.logline
                                    Transliterator.textDocument: textDocument
                                    Transliterator.cursorPosition: cursorPosition
                                    Transliterator.hasActiveFocus: activeFocus
                                    onTextChanged: scriteDocument.screenplay.logline = text
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    placeholderText: "Enter the logline of your screenplay here.."
                                }
                            }
                        }
                    }
                    footer: Item {
                        width: contentView.width
                        height: ruler.bottomMarginPx

                        Row {
                            anchors.centerIn: parent
                            visible: screenplayAdapter.screenplay === scriteDocument.screenplay && enabled
                            enabled: !scriteDocument.readOnly
                            spacing: 20

                            ToolButton2 {
                                id: addSceneButton
                                text: "Add Scene"
                                shortcutText: app.polishShortcutTextForDisplay("Ctrl+Shift+N")
                                icon.source: "../icons/content/add_circle_outline.png"
                                ToolTip.text: "Adds a new scene at the end of the screenplay."
                                width: implicitWidth * 1.5
                                height: implicitHeight * 1.5
                                flat: false
                                onClicked: {
                                    scriteDocument.screenplay.currentElementIndex = -1
                                    if(!scriteDocument.readOnly)
                                        scriteDocument.createNewScene()
                                }
                            }

                            ToolButton2 {
                                id: addBreakButton
                                text: "Add Break"
                                shortcutText: app.polishShortcutTextForDisplay("Ctrl+Shift+B")
                                icon.source: "../icons/content/add_box.png"
                                ToolTip.text: "Adds an act break at the end of the screenplay."
                                width: implicitWidth * 1.5
                                height: implicitHeight * 1.5
                                flat: false
                                onClicked: {
                                    scriteDocument.screenplay.addBreakElement(Screenplay.Act)
                                }
                            }
                        }
                    }

                    FocusTracker.window: qmlWindow
                    FocusTracker.indicator.target: mainUndoStack
                    FocusTracker.indicator.property: screenplayAdapter.isSourceScreenplay ? "screenplayEditorActive" : "sceneEditorActive"

                    Component.onCompleted: positionViewAtIndex(screenplayAdapter.currentIndex, ListView.Beginning)

                    property point firstPoint: mapToItem(contentItem, width/2, 1)
                    property point lastPoint: mapToItem(contentItem, width/2, height-2)
                    property int firstItemIndex: screenplayAdapter.elementCount > 0 ? Math.max(indexAt(firstPoint.x, firstPoint.y), 0) : 0
                    property int lastItemIndex: screenplayAdapter.elementCount > 0 ? validOrLastIndex(indexAt(lastPoint.x, lastPoint.y)) : 0

                    onContentYChanged: Qt.callLater(evaluateFirstAndLastPoint)
                    onOriginYChanged: Qt.callLater(evaluateFirstAndLastPoint)

                    function evaluateFirstAndLastPoint() {
                        firstPoint = mapToItem(contentItem, width/2, 1)
                        lastPoint = mapToItem(contentItem, width/2, height-2)
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

                        var newContentY = 0
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
                resolution: scriteDocument.displayFormat.pageLayout.resolution

                property real leftMarginPx: leftMargin * zoomLevel
                property real rightMarginPx: rightMargin * zoomLevel
                property real topMarginPx: pageLayout.topMargin * Screen.devicePixelRatio * zoomLevel
                property real bottomMarginPx: pageLayout.bottomMargin * Screen.devicePixelRatio * zoomLevel
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: scriteDocument.loading || screenplayTextDocument.updating
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

        ToolButton3 {
            id: toggleLockButton
            width: parent.height - 10
            height: width
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            enabled: !scriteDocument.readOnly
            iconSource: scriteDocument.readOnly ? "../icons/action/lock_outline.png" : (scriteDocument.locked ? "../icons/action/lock_outline.png" : "../icons/action/lock_open.png")
            ToolTip.text: "Lock to allow editing of this document only on this computer."
            onClicked: {
                var question = ""
                if(scriteDocument.locked)
                    question = "By unlocking this document, you will be able to edit it on this and any other computer. Do you want to unlock?"
                else
                    question = "By locking this document, you will be able to edit it only on this computer. Do you want to lock?"

                askQuestion({
                                "question": question,
                                "okButtonText": "Yes",
                                "cancelButtonText": "No",
                                "callback": function(val) {
                                    if(val) {
                                        scriteDocument.locked = !scriteDocument.locked
                                    }
                                }
                            }, this)
            }
        }

        Text {
            id: pageNumberDisplay
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: toggleLockButton.right
            anchors.leftMargin: 20
            font.family: headingFontMetrics.font.family
            font.pixelSize: parent.height * 0.5
            text: {
                var ret = "Page " + screenplayTextDocument.currentPage + "/" + screenplayTextDocument.pageCount + " | "
                if(screenplayTextDocument.pageCount > 1) {
                    if(globalTimeDisplay.visibleToUser) {
                        ret += "Time Est: ~" + screenplayTextDocument.totalTimeAsString
                    } else {
                        ret += "Time: ~" + screenplayTextDocument.currentTimeAsString + "/" + screenplayTextDocument.totalTimeAsString
                    }
                } else
                    ret += "Time Est: < ~" + screenplayTextDocument.timePerPageAsString
                return ret
            }

            ToolTip.text: "Page count and time estimates are approximate, assuming " + screenplayTextDocument.timePerPageAsString + " per page."
            ToolTip.delay: 1000
            ToolTip.visible: pageNumberDisplayMouseArea.containsMouse

            MouseArea {
                id: pageNumberDisplayMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Item {
            width: pageRulerArea.width
            height: parent.height
            anchors.centerIn: parent
            visible: parent.width - pageNumberDisplay.width - zoomSlider.width > width

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: ruler.leftMarginPx
                anchors.rightMargin: ruler.rightMarginPx
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: height*0.1
                font.family: headingFontMetrics.font.family
                font.pixelSize: parent.height * 0.6
                elide: Text.ElideRight
                text: {
                    if(screenplayAdapter.isSourceScene || screenplayAdapter.elementCount === 0)
                        return ""

                    var scene = null
                    var element = null
                    if(contentView.isVisible(screenplayAdapter.currentIndex)) {
                        scene = screenplayAdapter.currentScene
                        element = screenplayAdapter.currentElement
                    } else {
                        var data = screenplayAdapter.at(contentView.firstItemIndex)
                        scene = data ? data.scene : null
                        element = data ? data.screenplayElement : null
                    }
                    return scene && scene.heading.enabled ? "[" + element.resolvedSceneNumber + "] " + scene.heading.text : ''
                }
            }
        }

        ZoomSlider {
            id: zoomSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            property var zoomLevels: screenplayFormat.fontZoomLevels
            zoomLevel: zoomLevels[value]
            from: 0; to: zoomLevels.length-1
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
        }
    }

    Component {
        id: breakComponent

        Item {
            property int theIndex: componentData.rowNumber
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            height: breakTitle.height+20

            Rectangle {
                anchors.fill: breakTitle
                anchors.margins: -4
                color: accentColors.windowColor
                border.width: 1
                border.color: accentColors.borderColor
                opacity: 0.25
            }

            TextField2 {
                id: breakTitle
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width-8
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: headingFontMetrics.font.pointSize + 2
                font.family: headingFontMetrics.font.family
                font.bold: true
                text: parent.theElement.breakTitle
                onTextEdited: parent.theElement.breakTitle = text
                maximumLength: 50
            }
        }
    }

    Component {
        id: contentComponent

        Rectangle {
            id: contentItem
            property int theIndex: componentData.rowNumber
            property Scene theScene: componentData.scene
            property ScreenplayElement theElement: componentData.screenplayElement
            property bool isCurrent: theElement === screenplayAdapter.currentElement

            width: contentArea.width
            height: contentItemLayout.height
            color: "white"
            readonly property var binder: sceneDocumentBinder
            readonly property var editor: sceneTextEditor
            property bool canSplitScene: sceneTextEditor.activeFocus && !scriteDocument.readOnly && sceneDocumentBinder.currentElement && sceneDocumentBinder.currentElementCursorPosition === 0 && screenplayAdapter.isSourceScreenplay
            property bool canJoinToPreviousScene: sceneTextEditor.activeFocus && !scriteDocument.readOnly && sceneTextEditor.cursorPosition === 0 && contentItem.theIndex > 0

            SceneDocumentBinder {
                id: sceneDocumentBinder
                scene: contentItem.theScene
                textDocument: sceneTextEditor.textDocument
                cursorPosition: sceneTextEditor.activeFocus ? sceneTextEditor.cursorPosition : -1
                characterNames: scriteDocument.structure.characterNames
                screenplayFormat: screenplayEditor.screenplayFormat
                forceSyncDocument: !sceneTextEditor.activeFocus
                spellCheckEnabled: !scriteDocument.readOnly && spellCheckEnabledFlag.value
                liveSpellCheckEnabled: sceneTextEditor.activeFocus
                onDocumentInitialized: sceneTextEditor.cursorPosition = 0
                onRequestCursorPosition: app.execLater(contentItem, 100, function() { contentItem.assumeFocusAt(position) })
                property var currentParagraphType: currentElement ? currentElement.type : SceneHeading.Action
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

            BoxShadow {
                visible: screenplayAdapter.currentIndex === contentItem.theIndex && commentsSidePanel.expanded && commentsSidePanel.visible
                anchors.fill: commentsSidePanel
                anchors.leftMargin: 9
                opacity: 1
            }

            SidePanel {
                id: commentsSidePanel
                buttonColor: expanded ? Qt.tint(contentItem.theScene.color, "#C0FFFFFF") : Qt.tint(contentItem.theScene.color, "#D7EEEEEE")
                backgroundColor: buttonColor
                borderColor: expanded ? primaryColors.borderColor : Qt.rgba(0,0,0,0)
                anchors.top: parent.top
                anchors.left: parent.right
                // anchors.leftMargin: expanded ? 0 : -minPanelWidth
                buttonText: contentItem.isCurrent && expanded ? ("Scene #" + contentItem.theElement.sceneNumber + " Comments") : ""
                height: {
                    if(expanded) {
                        if(contentItem.isCurrent)
                            return contentInstance ? Math.max(contentInstance.contentHeight+40, 350) : 300
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
                    id: synopsisEdit
                    background: Rectangle {
                        color: Qt.tint(contentItem.theScene.color, "#E7FFFFFF")
                    }
                    font.pointSize: app.idealFontPointSize + 1
                    onTextChanged: contentItem.theScene.comments = text
                    wrapMode: Text.WordWrap
                    text: contentItem.theScene.comments
                    selectByMouse: true
                    selectByKeyboard: true
                    leftPadding: 10
                    rightPadding: 10
                    topPadding: 10
                    bottomPadding: 10
                    readOnly: scriteDocument.readOnly
                    onActiveFocusChanged: {
                        if(activeFocus)
                            screenplayAdapter.currentIndex = contentItem.theIndex
                    }

                    Transliterator.textDocument: textDocument
                    Transliterator.cursorPosition: cursorPosition
                    Transliterator.hasActiveFocus: activeFocus

                    SpecialSymbolsSupport {
                        anchors.top: parent.bottom
                        anchors.left: parent.left
                        textEditor: synopsisEdit
                        textEditorHasCursorInterface: true
                        enabled: !scriteDocument.readOnly
                    }
                }
            }

            Column {
                id: contentItemLayout
                width: parent.width

                Loader {
                    id: sceneHeadingAreaLoader
                    width: parent.width
                    active: contentItem.theScene !== null
                    sourceComponent: sceneHeadingArea
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
                }

                Rectangle {
                    width: parent.width
                    height: synopsisEditorLayout.height + 10
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
                            text: "<b>Synopsis:</b> " + (scriteDocument.structure.canvasUIMode === Structure.IndexCardUI ? "(The text you type below will be synced with this scene's index card on the Structure Canvas.)" : "")
                            font.pointSize: 12
                            visible: logLineField.length > 0
                            width: parent.width
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }

                        TextArea {
                            id: synopsisEditorField
                            width: parent.width
                            font.pointSize: screenplayFormat.defaultFont2.pointSize-2
                            readOnly: scriteDocument.readOnly
                            palette: app.palette
                            selectByMouse: true
                            selectByKeyboard: true
                            text: contentItem.theScene.title
                            Transliterator.textDocument: textDocument
                            Transliterator.cursorPosition: cursorPosition
                            Transliterator.hasActiveFocus: activeFocus
                            onTextChanged: contentItem.theScene.title = text
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            placeholderText: "Enter the synopsis of your scene here."
                            onActiveFocusChanged: {
                                if(activeFocus) {
                                    contentView.ensureVisible(synopsisEditorField, cursorRectangle)
                                    screenplayAdapter.currentIndex = contentItem.theIndex
                                }
                                sceneHeadingAreaLoader.item.sceneHasFocus = activeFocus
                                contentItem.theScene.undoRedoEnabled = activeFocus
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
                    palette: app.palette
                    selectByMouse: true
                    selectByKeyboard: true
                    property bool hasSelection: selectionStart >= 0 && selectionEnd >= 0 && selectionEnd > selectionStart
                    property Scene scene: contentItem.theScene
                    readOnly: scriteDocument.readOnly
                    background: Item {
                        id: sceneTextEditorBackground

                        ResetOnChange {
                            id: document
                            trackChangesOn: sceneDocumentBinder.documentLoadCount + zoomSlider.value
                            from: null
                            to: screenplayTextDocument
                            delay: 100
                        }

                        ScreenplayElementPageBreaks {
                            id: pageBreaksEvaluator
                            screenplayElement: contentItem.theElement
                            screenplayDocument: scriteDocument.loading ? null : document.value
                        }

                        Repeater {
                            model: pageBreaksEvaluator.pageBreaks

                            Item {
                                id: pageBreakLine
                                property rect cursorRect: modelData.position >= 0 ? sceneTextEditor.positionToRectangle(modelData.position) : Qt.rect(0,0,0,0)
                                x: 0
                                y: (modelData.position >= 0 ? cursorRect.y : -sceneHeadingAreaLoader.height) - height/2
                                width: sceneTextEditorBackground.width
                                height: 1
                                // color: primaryColors.c400.background

                                PageNumberBubble {
                                    x: -width - 20
                                    pageNumber: modelData.pageNumber
                                }
                            }
                        }
                    }
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font: screenplayFormat.defaultFont2
                    placeholderText: activeFocus ? "" : "Click here to type your scene content..."
                    onActiveFocusChanged: {
                        if(activeFocus) {
                            contentView.ensureVisible(sceneTextEditor, cursorRectangle)
                            screenplayAdapter.currentIndex = contentItem.theIndex
                            globalScreenplayEditorToolbar.sceneEditor = contentItem
                            justReceivedFocus = true
                        } else if(globalScreenplayEditorToolbar.sceneEditor === contentItem)
                            globalScreenplayEditorToolbar.sceneEditor = null
                        sceneHeadingAreaLoader.item.sceneHasFocus = activeFocus
                        contentItem.theScene.undoRedoEnabled = activeFocus
                    }

                    onCursorRectangleChanged: {
                        if(activeFocus /*&& contentView.isVisible(contentItem.theIndex)*/)
                            contentView.ensureVisible(sceneTextEditor, cursorRectangle)
                    }

                    property bool justReceivedFocus: false
                    cursorDelegate: Item {
                        x: sceneTextEditor.cursorRectangle.x
                        y: sceneTextEditor.cursorRectangle.y
                        width: sceneTextEditor.cursorRectangle.width
                        height: sceneTextEditor.cursorRectangle.height
                        visible: sceneTextEditor.activeFocus

                        Rectangle {
                            id: cursorRectangle
                            width: parent.width*Screen.devicePixelRatio
                            height: parent.height
                            anchors.centerIn: parent
                            color: scriteDocument.readOnly ? primaryColors.borderColor : "black"
                        }

                        SequentialAnimation {
                            running: sceneTextEditor.justReceivedFocus /*&& screenplayEditorSettings.enableAnimations*/

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
                                    cursorRectangle.width = sceneTextEditor.cursorRectangle.width*1.5
                                    cursorRectangle.height = sceneTextEditor.cursorRectangle.height
                                }
                            }
                        }

                        SequentialAnimation {
                            running: !sceneTextEditor.justReceivedFocus && sceneTextEditor.activeFocus
                            loops: Animation.Infinite
                            ScriptAction { script: cursorRectangle.opacity = 0.1 }
                            PauseAnimation { duration: 500 }
                            ScriptAction { script: cursorRectangle.opacity = 1 }
                            PauseAnimation { duration: 500 }
                        }
                    }

                    // Support for transliteration.
                    property bool userIsTyping: false
                    EventFilter.active: sceneTextEditor.activeFocus
                    EventFilter.events: [51,6] // Wheel, ShortcutOverride
                    EventFilter.onFilter: {
                        if(event.type === 51) {
                            // We want to avoid TextArea from processing Ctrl+Z
                            // and other such shortcuts.
                            result.acceptEvent = false
                            result.filter = (event.key === Qt.Key_Z || event.key === Qt.Key_Y)
                        } else if(event.type === 6) {
                            // Enter, Tab and other keys must not trigger
                            // Transliteration. Only space should.
                            sceneTextEditor.userIsTyping = event.hasText
                        }
                    }
                    Transliterator.enabled: contentItem.theScene && !contentItem.theScene.isBeingReset && userIsTyping
                    Transliterator.textDocument: textDocument
                    Transliterator.cursorPosition: cursorPosition
                    Transliterator.hasActiveFocus: activeFocus
                    Transliterator.onAboutToTransliterate: {
                        contentItem.theScene.beginUndoCapture(false)
                        contentItem.theScene.undoRedoEnabled = false
                    }
                    Transliterator.onFinishedTransliterating: {
                        app.execLater(Transliterator, 0, function() {
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
                        ToolTip.text: '<font name="' + sceneDocumentBinder.currentFont.family + '"><font color="lightgray">' + sceneDocumentBinder.completionPrefix.toUpperCase() + '</font>' + completer.suggestion.toUpperCase() + '</font>';
                        ToolTip.visible: completer.hasSuggestion

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
                            includeEmojis: app.isWindowsPlatform || app.isLinuxPlatform
                            textEditorHasCursorInterface: true
                            enabled: !scriteDocument.readOnly
                        }

                        Completer {
                            id: completer
                            strings: sceneDocumentBinder.autoCompleteHints
                            completionPrefix: sceneDocumentBinder.completionPrefix
                        }

                        // Context menus must ideally show up directly below the cursor
                        // So, we keep the menu loaders inside the cursorOverlay
                        MenuLoader {
                            id: spellingSuggestionsMenu
                            anchors.bottom: parent.bottom
                            enabled: !scriteDocument.readOnly

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
                            enabled: !scriteDocument.readOnly
                            menu: Menu2 {
                                onAboutToShow: sceneTextEditor.persistentSelection = true
                                onAboutToHide: sceneTextEditor.persistentSelection = false

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Cut\t" + app.polishShortcutTextForDisplay("Ctrl+X")
                                    enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
                                    onClicked: { sceneTextEditor.cut(); editorContextMenu.close() }
                                }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Copy\t" + app.polishShortcutTextForDisplay("Ctrl+C")
                                    enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
                                    onClicked: { sceneTextEditor.copy2(); editorContextMenu.close() }
                                }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Paste\t" + app.polishShortcutTextForDisplay("Ctrl+V")
                                    enabled: sceneTextEditor.canPaste
                                    onClicked: { sceneTextEditor.paste2(); editorContextMenu.close() }
                                }

                                MenuSeparator {  }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Split Scene"
                                    enabled: contentItem.canSplitScene
                                    onClicked: {
                                        contentItem.splitScene()
                                        editorContextMenu.close()
                                    }
                                }

                                MenuItem2 {
                                    focusPolicy: Qt.NoFocus
                                    text: "Join Previous Scene"
                                    enabled: contentItem.canJoinToPreviousScene
                                    onClicked: {
                                        contentItem.mergeWithPreviousScene()
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
                                            text: modelData.display + "\t" + app.polishShortcutTextForDisplay("Ctrl+" + (index+1))
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
                                        model: app.enumerationModel(app.transliterationEngine, "Language")

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
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: sceneTextEditor.activeFocus && !scriteDocument.readOnly
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: completer.hasSuggestion ? "Auto-complete" : sceneDocumentBinder.nextTabFormatAsString
                        ShortcutsModelItem.shortcut: "Tab"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: sceneTextEditor.activeFocus && !scriteDocument.readOnly
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: "Create New Paragraph"
                        ShortcutsModelItem.shortcut: app.isMacOSPlatform ? "Return" : "Enter"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: contentItem.canSplitScene
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: "Split Scene"
                        ShortcutsModelItem.shortcut: app.isMacOSPlatform ? "Ctrl+Return" : "Ctrl+Enter"
                    }

                    QtObject {
                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.enabled: contentItem.canJoinToPreviousScene
                        ShortcutsModelItem.visible: sceneTextEditor.activeFocus
                        ShortcutsModelItem.group: "Formatting"
                        ShortcutsModelItem.title: "Join Previous Scene"
                        ShortcutsModelItem.shortcut: app.isMacOSPlatform ? "Ctrl+Delete" : "Ctrl+Backspace"
                    }

                    Keys.onTabPressed: {
                        if(!scriteDocument.readOnly) {
                            if(completer.suggestion !== "") {
                                userIsTyping = false
                                insert(cursorPosition, completer.suggestion)
                                userIsTyping = true
                                Transliterator.enableFromNextWord()
                            } else
                                sceneDocumentBinder.tab()
                            event.accepted = true
                        }
                    }
                    Keys.onBacktabPressed: {
                        if(!scriteDocument.readOnly)
                            sceneDocumentBinder.backtab()
                    }

                    // split-scene handling.
                    Keys.onReturnPressed: {
                        if(scriteDocument.readOnly) {
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
                        enabled: !scriteDocument.readOnly && contextMenuEnableBinder.get
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

                        switch(event.key) {
                        case Qt.Key_PageUp:
                            event.accepted = true
                            contentItem.scrollToPreviousScene()
                            break
                        case Qt.Key_PageDown:
                            event.accepted = true
                            contentItem.scrollToNextScene()
                            break
                        }

                        if(event.modifiers === Qt.ControlModifier) {
                            switch(event.key) {
                            case Qt.Key_Delete:
                                if(app.isMacOSPlatform && sceneTextEditor.cursorPosition === 0) {
                                    event.accepted = true
                                    contentItem.mergeWithPreviousScene()
                                }
                                break
                            case Qt.Key_Backspace:
                                if(sceneTextEditor.cursorPosition === 0) {
                                    event.accepted = true
                                    contentItem.mergeWithPreviousScene()
                                }
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

                        onLoadCountChanged: highlightSearchResultTextSnippet()
                        onSelectionChanged: highlightSearchResultTextSnippet()

                        function highlightSearchResultTextSnippet() {
                            if(selection.start >= 0 && selection.end >= 0) {
                                var rect = app.uniteRectangles( sceneTextEditor.positionToRectangle(selection.start),
                                                               sceneTextEditor.positionToRectangle(selection.end) )
                                rect = app.adjustRectangle(rect, -20, -50, 20, 50)
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
                        onReplaceCurrentRequest: {
                            if(textDocumentSearch.currentResultIndex >= 0) {
                                contentItem.theScene.beginUndoCapture()
                                textDocumentSearch.replace(replacementText)
                                contentItem.theScene.endUndoCapture()
                            }
                        }
                    }

                    // Custom Copy & Paste
                    function copy2() {
                        if(hasSelection)
                            sceneDocumentBinder.copy(selectionStart, selectionEnd)
                    }

                    function paste2() {
                        if(canPaste) {
                            // Fix for https://github.com/teriflix/scrite/issues/195
                            // [0.5.2 All] Pasting doesnt replace the selected text #195
                            if(sceneTextEditor.hasSelection)
                                sceneTextEditor.remove(sceneTextEditor.selectionStart, sceneTextEditor.selectionEnd)
                            if(!sceneDocumentBinder.paste(sceneTextEditor.cursorPosition))
                                sceneTextEditor.paste()
                        }
                    }
                }
            }


            Rectangle {
                width: parent.width * 0.01
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: Qt.tint(contentItem.theScene.color, "#A7FFFFFF")
                visible: screenplayAdapter.currentIndex === contentItem.theIndex
            }

            function mergeWithPreviousScene() {
                if(!contentItem.canJoinToPreviousScene)
                    return
                screenplayAdapter.mergeElementWithPrevious(contentItem.theElement)
            }

            function splitScene() {
                if(!contentItem.canSplitScene)
                    return
                screenplayAdapter.splitElement(contentItem.theElement, sceneDocumentBinder.currentElement, sceneDocumentBinder.currentElementCursorPosition)
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
                var item = contentView.loadedItemAtIndex(idx)
                item.assumeFocusAt(-1)
            }

            function scrollToNextScene() {
                var idx = screenplayAdapter.nextSceneElementIndex()
                if(idx === screenplayAdapter.elementCount-1 && idx === theIndex) {
                    contentView.positionViewAtEnd()
                    assumeFocusAt(-1)
                    return
                }

                contentView.scrollIntoView(idx)
                var item = contentView.loadedItemAtIndex(idx)
                item.assumeFocusAt(0)
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
                if(theScene.heading.enabled)
                    sceneHeadingLoader.viewOnly = false
            }

            height: sceneHeadingLayout.height + 24
            color: Qt.tint(theScene.color, "#E7FFFFFF")

            Item {
                width: ruler.leftMarginPx
                height: parent.height

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width * 0.075
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 20

                    SceneTypeImage {
                        width: sceneHeadingLoader.height
                        height: width
                        anchors.verticalCenter: parent.verticalCenter
                        sceneType: headingItem.theScene.type
                    }

                    TextField2 {
                        label: "Scene No."
                        labelAlwaysVisible: true
                        width: headingFontMetrics.averageCharacterWidth*maximumLength
                        text: headingItem.theElement.userSceneNumber
                        anchors.bottom: parent.bottom
                        font: headingFontMetrics.font
                        onTextChanged: headingItem.theElement.userSceneNumber = text
                        maximumLength: 5
                        placeholderText: headingItem.theElement.sceneNumber
                        visible: headingItem.theElement.elementType === ScreenplayElement.SceneElementType &&
                                 headingItem.theScene.heading.enabled &&
                                 screenplayAdapter.isSourceScreenplay
                        onActiveFocusChanged: screenplayAdapter.currentIndex = headingItem.theElementIndex
                    }
                }
            }

            Column {
                id: sceneHeadingLayout
                spacing: sceneCharactersListLoader.active ? 5 : 0
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: ruler.leftMarginPx
                anchors.rightMargin: ruler.rightMarginPx
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 8

                Row {
                    spacing: 5
                    width: parent.width

                    Loader {
                        id: sceneHeadingLoader
                        width: parent.width - sceneMenuButton.width - parent.spacing
                        height: item ? item.contentHeight : headingFontMetrics.lineSpacing
                        property bool viewOnly: true
                        property int sceneIndex: headingItem.theElementIndex
                        property SceneHeading sceneHeading: headingItem.theScene.heading
                        property TextArea sceneTextEditor: headingItem.sceneTextEditor
                        sourceComponent: {
                            if(scriteDocument.readOnly)
                                return sceneHeading.enabled ? sceneHeadingViewer : sceneHeadingDisabled
                            if(sceneHeading.enabled)
                                return viewOnly ? sceneHeadingViewer : sceneHeadingEditor
                            return sceneHeadingDisabled
                        }

                        Connections {
                            target: sceneHeadingLoader.item
                            ignoreUnknownSignals: true
                            onEditRequest: sceneHeadingLoader.viewOnly = false
                            onEditingFinished: sceneHeadingLoader.viewOnly = true
                        }
                    }

                    ToolButton3 {
                        id: sceneMenuButton
                        iconSource: "../icons/navigation/menu.png"
                        ToolTip.text: "Click here to view scene options menu."
                        ToolTip.delay: 1000
                        onClicked: sceneMenu.visible = true
                        down: sceneMenu.visible
                        anchors.verticalCenter: parent.verticalCenter
                        width: headingFontMetrics.lineSpacing
                        height: headingFontMetrics.lineSpacing
                        visible: enabled
                        enabled: !scriteDocument.readOnly

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

                                Menu2 {
                                    title: "Mark Scene As"

                                    Repeater {
                                        model: app.enumerationModel(headingItem.theScene, "Type")

                                        MenuItem2 {
                                            text: modelData.key
                                            font.bold: headingItem.theScene.type === modelData.value
                                            onTriggered: headingItem.theScene.type = modelData.value
                                        }
                                    }
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
                                            scriteDocument.screenplay.currentElementIndex = headingItem.theElementIndex
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
                                    enabled: screenplayAdapter.screenplay === scriteDocument.screenplay
                                    onClicked: {
                                        sceneMenu.close()
                                        scriteDocument.screenplay.removeSceneElements(headingItem.theScene)
                                    }
                                }
                            }
                        }
                    }
                }

                Loader {
                    id: sceneCharactersListLoader
                    width: parent.width
                    readonly property bool editorHasActiveFocus: headingItem.sceneHasFocus
                    property Scene scene: headingItem.theScene
                    active: screenplayEditorSettings.displaySceneCharacters
                    sourceComponent: sceneCharactersList
                }
            }
        }
    }

    FontMetrics {
        id: defaultFontMetrics
        readonly property SceneElementFormat format: scriteDocument.formatting.elementFormat(SceneElement.Action)
        font: format ? format.font2 : scriteDocument.formatting.defaultFont2
    }

    FontMetrics {
        id: headingFontMetrics
        readonly property SceneElementFormat format: scriteDocument.formatting.elementFormat(SceneElement.Heading)
        font: format.font2
    }

    Component {
        id: sceneHeadingDisabled

        Item {
            property real contentHeight: headingFontMetrics.lineSpacing

            Text {
                text: "no scene heading"
                anchors.verticalCenter: parent.verticalCenter
                color: primaryColors.c10.text
                font: headingFontMetrics.font
                opacity: 0.25
            }
        }
    }

    Component {
        id: sceneHeadingEditor

        Item {
            property real contentHeight: height
            height: layout.height + 4
            Component.onCompleted: {
                locTypeEdit.forceActiveFocus()
                screenplayAdapter.currentIndex = sceneIndex
            }

            signal editingFinished()

            property bool hasFocus: locTypeEdit.activeFocus || locTypeEdit.showingSymbols ||
                                    locEdit.activeFocus || locEdit.showingSymbols ||
                                    momentEdit.activeFocus || momentEdit.showingSymbols
            onHasFocusChanged: {
                if(!hasFocus)
                    editingFinished()
            }

            Row {
                id: layout
                anchors.left: parent.left
                anchors.right: parent.right

                TextField2 {
                    id: locTypeEdit
                    font: headingFontMetrics.font
                    width: Math.max(headingFontMetrics.averageCharacterWidth*5, Math.min(contentWidth, 120*zoomLevel))
                    anchors.verticalCenter: parent.verticalCenter
                    text: sceneHeading.locationType
                    completionStrings: scriteDocument.structure.standardLocationTypes()
                    enableTransliteration: true
                    onEditingComplete: sceneHeading.locationType = text
                    tabItem: locEdit
                    includeEmojiSymbols: app.isWindowsPlatform || app.isLinuxPlatform
                    onActiveFocusChanged: screenplayAdapter.currentIndex = sceneIndex
                }

                Text {
                    id: sep1Text
                    font: headingFontMetrics.font
                    text: ". "
                    anchors.verticalCenter: parent.verticalCenter
                }

                TextField2 {
                    id: locEdit
                    font: headingFontMetrics.font
                    width: parent.width - locTypeEdit.width - sep1Text.width - momentEdit.width - sep2Text.width
                    anchors.verticalCenter: parent.verticalCenter
                    text: sceneHeading.location
                    enableTransliteration: true
                    completionStrings: scriteDocument.structure.allLocations()
                    onEditingComplete: sceneHeading.location = text
                    tabItem: momentEdit
                    includeEmojiSymbols: app.isWindowsPlatform || app.isLinuxPlatform
                    onActiveFocusChanged: screenplayAdapter.currentIndex = sceneIndex
                }

                Text {
                    id: sep2Text
                    font: headingFontMetrics.font
                    text: "- "
                    anchors.verticalCenter: parent.verticalCenter
                }

                TextField2 {
                    id: momentEdit
                    font: headingFontMetrics.font
                    width: Math.max(headingFontMetrics.averageCharacterWidth*5, Math.min(contentWidth, 200*zoomLevel))
                    anchors.verticalCenter: parent.verticalCenter
                    text: sceneHeading.moment
                    enableTransliteration: true
                    completionStrings: scriteDocument.structure.standardMoments()
                    onEditingComplete: sceneHeading.moment = text
                    tabItem: sceneTextEditor
                    includeEmojiSymbols: app.isWindowsPlatform || app.isLinuxPlatform
                    onActiveFocusChanged: screenplayAdapter.currentIndex = sceneIndex
                }
            }
        }
    }

    Component {
        id: sceneHeadingViewer

        Item {
            property real contentHeight: sceneHeadingText.contentHeight
            signal editRequest()

            Text {
                id: sceneHeadingText
                width: parent.width
                font: headingFontMetrics.font
                text: sceneHeading.text
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.WordWrap
                color: headingFontMetrics.format.textColor
            }

            MouseArea {
                anchors.fill: parent
                onClicked: parent.editRequest()
            }
        }
    }

    Component {
        id: sceneCharactersList

        Flow {
            spacing: 5
            flow: Flow.LeftToRight

            Text {
                id: sceneCharactersListHeading
                text: "Characters: "
                font.bold: true
                topPadding: 5
                bottomPadding: 5
                font.pointSize: 12
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
                    border.width: editorHasActiveFocus ? 0 : 1
                    border.color: colors.text
                    color: colors.background
                    textColor: colors.text
                    text: modelData
                    leftPadding: 10
                    rightPadding: 10
                    topPadding: 2
                    bottomPadding: 2
                    font.pointSize: 12
                    closable: scene.isCharacterMute(modelData)
                    onClicked: requestCharacterMenu(modelData)
                    onCloseRequest: {
                        if(!scriteDocument.readOnly)
                            scene.removeMuteCharacter(modelData)
                    }
                }
            }

            Loader {
                id: newCharacterInput
                width: active && item ? Math.max(item.contentWidth, 100) : 0
                active: false
                sourceComponent: Item {
                    property alias contentWidth: textViewEdit.contentWidth
                    height: textViewEdit.height

                    TextViewEdit {
                        id: textViewEdit
                        width: parent.width
                        y: fontDescent
                        readOnly: false
                        font.capitalization: Font.AllUppercase
                        font.pointSize: 12
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.NoWrap
                        completionStrings: scriteDocument.structure.characterNames
                        onEditingFinished: {
                            scene.addMuteCharacter(text)
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
                height: sceneCharactersListHeading.height
                opacity: 0.5
                visible: enabled
                enabled: !scriteDocument.readOnly

                MouseArea {
                    ToolTip.text: "Click here to capture characters who don't have any dialogues in this scene, but are still required for the scene."
                    ToolTip.delay: 1000
                    ToolTip.visible: containsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                    onClicked: newCharacterInput.active = true
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
        width: sceneListSidePanel.width // Math.max(sceneListSidePanel.width, notesSidePanel.width)
        property bool expanded: sceneListSidePanel.expanded
        onExpandedChanged: contentView.commentsExpandCounter = 0

        SidePanel {
            id: sceneListSidePanel
            height: parent.height
            buttonY: 20
            buttonText: ""
            z: expanded ? 1 : 0

            content: Item {
                Text {
                    id: dragHotspotItem
                    font.family: "Courier Prime"
                    font.pixelSize: app.idealFontPointSize + 4
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

                ListView {
                    id: sceneListView
                    anchors.fill: parent
                    clip: true
                    model: screenplayAdapter
                    currentIndex: screenplayAdapter.currentIndex
                    ScrollBar.vertical: ScrollBar {
                        policy: sceneListView.contentHeight > sceneListView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                        minimumSize: 0.1
                        palette {
                            mid: Qt.rgba(0,0,0,0.25)
                            dark: Qt.rgba(0,0,0,0.75)
                        }
                        opacity: active ? 1 : 0.2
                        Behavior on opacity {
                            enabled: screenplayEditorSettings.enableAnimations
                            NumberAnimation { duration: 250 }
                        }
                    }
                    highlightFollowsCurrentItem: true
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0

                    header: Rectangle {
                        width: sceneListView.width-1
                        height: 40
                        color: screenplayAdapter.currentIndex < 0 ? accentColors.windowColor : Qt.rgba(0,0,0,0)

                        Text {
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
                            font.family: "Courier Prime"
                            font.pixelSize: 14
                            font.bold: screenplayAdapter.currentIndex < 0
                            text: "[#] TITLE PAGE"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                screenplayAdapter.currentIndex = -1
                                contentView.positionViewAtBeginning()
                            }
                        }
                    }

                    delegate: Rectangle {
                        id: delegateItem
                        width: sceneListView.width-1
                        height: 40
                        color: scene ? Qt.tint(scene.color, (screenplayAdapter.currentIndex === index ? "#9CFFFFFF" : "#E7FFFFFF")) : Qt.rgba(0,0,0,0)

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
                            property real leftMargin: 6 + (sceneTypeImage.width+12)*sceneTypeImage.t
                            anchors.left: parent.left
                            anchors.leftMargin: leftMargin
                            anchors.right: parent.right
                            anchors.rightMargin: (sceneListView.contentHeight > sceneListView.height ? sceneListView.ScrollBar.vertical.width : 5) + 5
                            anchors.verticalCenter: parent.verticalCenter
                            font.family: "Courier Prime"
                            font.bold: screenplayAdapter.currentIndex === index || screenplayElementType === ScreenplayElement.BreakElementType
                            font.pixelSize: screenplayElementType === ScreenplayElement.BreakElementType ? 16 : 14
                            font.letterSpacing: screenplayElementType === ScreenplayElement.BreakElementType ? 3 : 0
                            horizontalAlignment: screenplayElementType === ScreenplayElement.BreakElementType ? Qt.AlignRight : (scene && scene.heading.enabled ? Qt.AlignLeft : Qt.AlignRight)
                            color: screenplayElementType === ScreenplayElement.BreakElementType ? "gray" : "black"
                            font.capitalization: Font.AllUppercase
                            text: {
                                if(scene && scene.heading.enabled)
                                    return screenplayElement.resolvedSceneNumber + ". " + scene.heading.text
                                if(screenplayElementType === ScreenplayElement.BreakElementType)
                                    return screenplayElement.breakTitle
                                return "NO SCENE HEADING"
                            }
                            elide: Text.ElideMiddle
                        }

                        MouseArea {
                            id: delegateMouseArea
                            anchors.fill: parent
                            enabled: screenplayElementType === ScreenplayElement.SceneElementType
                            onClicked: navigateToScene()
                            drag.target: screenplayAdapter.isSourceScreenplay && !scriteDocument.readOnly ? parent : null
                            drag.axis: Drag.YAxis
                            onPressed: {
                                dragHotspotItem.text = scene.heading.text
                                dragHotspotItem.grabToImage(function(result) {
                                    delegateItem.Drag.imageSource = result.url
                                })
                            }
                            onDoubleClicked: {
                                navigateToScene()
                                sceneListSidePanel.expanded = false
                            }

                            function navigateToScene() {
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
                            if(screenplayElementType === ScreenplayElement.BreakElementType)
                                scriteDocument.screenplay.currentElementIndex = index
                        }

                        DropArea {
                            anchors.fill: parent
                            keys: ["sceneListView/sceneID"]

                            onEntered: {
                                drag.accepted = true
                                dropIndicator.visible = true
                                sceneListView.forceActiveFocus()
                            }

                            onExited: {
                                drag.accepted = true
                                dropIndicator.visible = false
                            }

                            onDropped: {
                                dropIndicator.visible = false
                                var dropSource = drop.source
                                app.execLater(sceneListView, 100, function() { sceneListView.dropSceneAt(dropSource, index) })
                                drop.acceptProposedAction()
                            }
                        }

                        Rectangle {
                            id: dropIndicator
                            width: parent.width
                            height: 2
                            color: primaryColors.borderColor
                            visible: false
                        }
                    }

                    footer: Item {
                        width: sceneListView.width-1
                        height: 40

                        DropArea {
                            anchors.fill: parent
                            keys: ["sceneListView/sceneID"]

                            onEntered: {
                                drag.accepted = true
                                dropIndicator2.visible = true
                                sceneListView.forceActiveFocus()
                            }

                            onExited: {
                                drag.accepted = true
                                dropIndicator2.visible = false
                            }

                            onDropped: {
                                dropIndicator2.visible = false
                                var dropSource = drop.source
                                app.execLater(sceneListView, 100, function() { sceneListView.dropSceneAt(dropSource, -1) })
                                drop.acceptProposedAction()
                            }
                        }

                        Rectangle {
                            id: dropIndicator2
                            width: parent.width
                            height: 2
                            color: primaryColors.borderColor
                            visible: false
                        }
                    }

                    function dropSceneAt(scene, index) {
                        if(!screenplayAdapter.isSourceScreenplay)
                            return

                        var fromIndex = scriteDocument.screenplay.indexOfElement(scene)
                        var toIndex = index < 0 ? index : (fromIndex < index ? index-1 : index)
                        var curIndex = index < 0 ? scriteDocument.screenplay.elementCount-1 : toIndex
                        scriteDocument.screenplay.moveElement(scene, toIndex)
                        screenplayAdapter.refresh()
                        positionViewAtIndex(curIndex, ListView.Contain)
                        screenplayAdapter.currentIndex = curIndex
                    }
                }
            }
        }
    }

    Loader {
        id: screenplayPreview
        visible: globalScreenplayEditorToolbar.showScreenplayPreview
        active: globalScreenplayEditorToolbar.showScreenplayPreview
        anchors.fill: parent
        sourceComponent: ScreenplayPreview {
            purpose: ScreenplayTextDocument.ForPrinting
            screenplay: screenplayTextDocument.screenplay
            screenplayFormat: screenplayTextDocument.formatting
            titlePage: screenplayEditorSettings.includeTitlePageInPreview
            titlePageIsCentered: scriteDocument.screenplay.titlePageIsCentered
        }
    }

    function requestCharacterMenu(characterName) {
        if(characterMenu.characterReports.length === 0) {
            var reports = scriteDocument.supportedReports
            var chReports = []
            reports.forEach( function(item) {
                if(item.name.indexOf('Character') >= 0)
                    chReports.push(item)
            })
            characterMenu.characterReports = chReports
        }

        characterMenu.characterName = characterName
        characterMenu.popup()
    }

    Menu2 {
        id: characterMenu
        width: 300
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
            model: characterMenu.characterReports.length > 0 ? (additionalCharacterMenuItems.length ? 1 : 0) : 0

            MenuSeparator { }
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
                    switch(scriteDocument.screenplay.coverPagePhotoSize) {
                    case Screenplay.SmallCoverPhoto:
                        return parent.maxWidth / 4
                    case Screenplay.MediumCoverPhoto:
                        return parent.maxWidth / 2
                    }
                    return parent.maxWidth
                }

                source: visible ? "file:///" + scriteDocument.screenplay.coverPagePhoto : ""
                visible: scriteDocument.screenplay.coverPagePhoto !== ""
                smooth: true; mipmap: true
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item { width: parent.width; height: scriteDocument.screenplay.coverPagePhoto !== "" ? 20 * zoomLevel : 0 }

            Text {
                font.family: scriteDocument.formatting.defaultFont.family
                font.pointSize: defaultFontSize + 2
                font.bold: true
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: scriteDocument.screenplay.title === "" ? "<untitled>" : scriteDocument.screenplay.title
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
            }

            Text {
                font.family: scriteDocument.formatting.defaultFont.family
                font.pointSize: defaultFontSize
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: scriteDocument.screenplay.subtitle
                visible: scriteDocument.screenplay.subtitle !== ""
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
            }

            Column {
                width: parent.width
                spacing: 0

                Text {
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: "Written By"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                    text: (scriteDocument.screenplay.author === "" ? "<unknown author>" : scriteDocument.screenplay.author)
                    anchors.horizontalCenter: parent.horizontalCenter
                    leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                    rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
                }
            }

            Text {
                font.family: scriteDocument.formatting.defaultFont.family
                font.pointSize: defaultFontSize
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: scriteDocument.screenplay.version === "" ? "Initial Version" : scriteDocument.screenplay.version
                anchors.horizontalCenter: parent.horizontalCenter
                leftPadding: contentWidth > maxWidth ? ruler.leftMarginPx : 0
                rightPadding: contentWidth > maxWidth ? ruler.rightMarginPx : 0
            }

            Text {
                font.family: scriteDocument.formatting.defaultFont.family
                font.pointSize: defaultFontSize
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                text: scriteDocument.screenplay.basedOn
                visible: scriteDocument.screenplay.basedOn !== ""
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
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: scriteDocument.screenplay.contact
                    visible: text !== ""
                }

                Text {
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: scriteDocument.screenplay.address
                    visible: text !== ""
                }

                Text {
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: scriteDocument.screenplay.phoneNumber
                    visible: text !== ""
                }

                Text {
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    font.underline: true
                    color: "blue"
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: scriteDocument.screenplay.email
                    visible: text !== ""

                    MouseArea {
                        anchors.fill: parent
                        onClicked: Qt.openUrlExternally("mailto:" + parent.text)
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                Text {
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.pointSize: defaultFontSize - 2
                    font.underline: true
                    color: "blue"
                    width: parent.width
                    elide: Text.ElideRight
                    text: scriteDocument.screenplay.website
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

    /*
    DockWidget {
        id: textFormatDockWindow
        anchors.fill: parent
        contentX: Math.min( screenplayEditorSettings.textFormatDockWidgetX, parent.width-contentWidth )
        contentY: Math.min( screenplayEditorSettings.textFormatDockWidgetY, parent.height-contentHeight )
        contentWidth: 250
        contentHeight: 225
        onContentXChanged: Qt.callLater( updatePositionInSettings )
        onContentYChanged: Qt.callLater( updatePositionInSettings )
        function updatePositionInSettings() {
            screenplayEditorSettings.textFormatDockWidgetX = contentX
            screenplayEditorSettings.textFormatDockWidgetY = contentY
        }

        title: "Text Formatting"
        property SceneDocumentBinder binder: globalScreenplayEditorToolbar.binder
        active: binder !== null
        visible: active
        content: Item {
            implicitWidth: toolBarLayout.width + 10
            implicitHeight: toolBarLayout.height + 10

            readonly property real buttonSize: 55
            readonly property TextFormat textFormat: textFormatDockWindow.binder.textFormat

            Column {
                id: toolBarLayout
                width: buttonSize * 4
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 5
                spacing: 5

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter

                    ToolButton3 {
                        iconSource: "../icons/editor/format_bold.png"
                        ToolTip.visible: false
                        checkable: true
                        checked: textFormat.bold
                        onToggled: textFormat.bold = checked
                    }

                    ToolButton3 {
                        iconSource: "../icons/editor/format_ital24px.png"
                        ToolTip.visible: false
                        checkable: true
                        checked: textFormat.italic
                        onToggled: textFormat.italic = checked
                    }

                    ToolButton3 {
                        iconSource: "../icons/editor/format_underline.png"
                        ToolTip.visible: false
                        checkable: true
                        checked: textFormat.underline
                        onToggled: textFormat.underline = checked
                    }
                }

                Row {
                    width: parent.width

                    Text {
                        text: "Text Color: "
                        width: parent.width - 44
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: app.idealFontPointSize
                    }

                    Rectangle {
                        color: textFormat.hasTextColor ? textFormat.textColor : Qt.rgba(0,0,0,0)
                        width: 42; height: width
                        border.width: 1
                        border.color: "black"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: textColorMenu.popup()
                        }

                        ColorMenu {
                            id: textColorMenu
                            onMenuItemClicked: textFormat.textColor = color
                        }
                    }
                }

                Row {
                    width: parent.width

                    Text {
                        text: "Background Color: "
                        width: parent.width - 44
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: app.idealFontPointSize
                    }

                    Rectangle {
                        color: textFormat.hasBackgroundColor ? textFormat.backgroundColor : Qt.rgba(0,0,0,0)
                        width: 42; height: width
                        border.width: 1
                        border.color: "black"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: backgroundColorMenu.popup()
                        }

                        ColorMenu {
                            id: backgroundColorMenu
                            onMenuItemClicked: textFormat.backgroundColor = color
                        }
                    }
                }
            }
        }
    }
    */
}
