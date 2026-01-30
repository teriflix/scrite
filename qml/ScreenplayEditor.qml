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

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notifications"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/floatingdockpanels"

Rectangle {
    id: root

    readonly property alias searchBar: _searchBarArea

    property alias hasFocus: _elementListView.hasFocus
    property alias minSidePanelWidth: _private.minSidePanelWidth
    property alias maxSidePanelWidth: _private.maxSidePanelWidth
    property alias searchBarVisible: _searchBarArea.visible
    property alias sidePanelEnabled: _sidePanelLoader.active

    function toggleSearchBar(showReplace) {
        _searchBarArea.toggle(showReplace)
    }

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object,event,result) => {
                              if(MouseCursor.isOverItem(_workspace)) {
                                  EventFilter.forwardEventTo(_elementListView)
                                  result.filter = true
                                  result.accepted = true
                              } else {
                                  result.filter = false
                                  result.accepted = false
                              }
                          }

    color: Runtime.colors.primary.windowColor

    ScreenplayEditorSearchBar {
        id: _searchBarArea

        ObjectRegister.name: "screenplayEditorSearchBar"

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        screenplayAdapter: Runtime.screenplayAdapter

        ActionHandler {
            action: ActionHub.editOptions.find("find")

            onTriggered: parent.toggle(false)
        }

        ActionHandler {
            action: ActionHub.editOptions.find("replace")

            onTriggered: parent.toggle(true)
        }

        function toggle(showReplace) {
            if(typeof showReplace === "boolean")
                searchBar.showReplace = showReplace

            if(visible) {
                if(searchBar.hasFocus)
                    visible = false
                else
                    searchBar.assumeFocus()
            } else {
                visible = true
                searchBar.assumeFocus()
            }
        }
    }

    Loader {
        id: _sidePanelLoader

        anchors.top: _searchBarArea.visible ? _searchBarArea.bottom : parent.top
        anchors.left: parent.left
        anchors.bottom: _statusBar.top
        anchors.topMargin: 5
        anchors.bottomMargin: 5

        active: Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab

        sourceComponent: ScreenplayEditorSidePanel {
            id: _sceneListPanel

            readOnly: Scrite.document.readOnly
            screenplayAdapter: Runtime.screenplayAdapter

            onPositionScreenplayEditorAtTitlePage: _elementListView.positionViewAtBeginning()

            ActionHandler {
                action: ActionHub.sceneListPanelOptions.find("sidePanelVisibility")
                checked: _sceneListPanel.expanded

                onToggled: (source) => {
                               _sceneListPanel.expanded = !_sceneListPanel.expanded
                           }
            }

            ActionHandler {
                action: ActionHub.sceneListPanelOptions.find("displayScreenplayTracks")
                checked: Runtime.screenplayTracksSettings.displayTracks
                enabled: Runtime.appFeatures.structure.enabled

                onToggled: (source) => {
                               Runtime.screenplayTracksSettings.displayTracks = !Runtime.screenplayTracksSettings.displayTracks
                           }
            }
        }
    }

    Item {
        id: _workspace

        anchors.top: _searchBarArea.visible ? _searchBarArea.bottom : parent.top
        anchors.left: _sidePanelLoader.right
        anchors.right: parent.right
        anchors.bottom: _statusBar.top

        clip: true

        RulerItem {
            id: _ruler

            anchors.top: parent.top

            x: {
                if(_private.showSceneComments && Runtime.screenplayEditorSettings.sceneSidePanelOpen) {
                    const commentsPanelSpace = Runtime.bounded(Runtime.minSceneSidePanelWidth,
                                                               (parent.width - width),
                                                               Runtime.maxSceneSidePanelWidth)
                    return Math.max(_elementListView.minLeftMargin,
                                    (parent.width - (width + commentsPanelSpace))/2)
                }

                return (parent.width - width)/2
            }
            z: 1

            width: _private.pageLayout.paperWidth * _private.zoomLevel * _private.dpi
            height: Runtime.minimumFontMetrics.lineSpacing

            visible: Runtime.screenplayEditorSettings.displayRuler
            zoomLevel: _private.zoomLevel
            resolution: _private.pageLayout.resolution
            leftMargin: _private.pageMargins.left
            rightMargin: _private.pageMargins.right
            paragraphLeftMargin: _private.currentParagraphMargins.left
            paragraphRightMargin: _private.currentParagraphMargins.left

            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        }

        Rectangle {
            anchors.fill: _elementListView

            color: Runtime.colors.primary.editor.background
            border.width: 1
            border.color: root.color
        }

        ScreenplayElementListView {
            id: _elementListView

            readonly property real minLeftMargin: 70

            ScrollBar.vertical: _scrollBar

            anchors.top: _ruler.visible ? _ruler.bottom : parent.top
            anchors.left: _ruler.left
            anchors.right: _ruler.right
            anchors.bottom: parent.bottom

            readOnly: Scrite.document.readOnly
            zoomLevel: _private.zoomLevel
            pageMargins: _private.pageMargins
            screenplayAdapter: Runtime.screenplayAdapter
            showSceneComments: _private.showSceneComments
            spaceAvailableOnTheLeft: x-minLeftMargin-1
            spaceAvailableOnTheRight: parent.width - x - width - (_scrollBar.visible ? _scrollBar.width : 0)

            onHasFocusChanged: {
                if(hasFocus && !Runtime.screenplayEditorSettings.languageInputPreferenceChecked) {
                    if(LanguageEngine.hasPlatformLanguages()) {
                        Runtime.screenplayEditorSettings.languageInputPreferenceChecked = true

                        let message = ""
                        if(LanguageEngine.handleLanguageSwitch)
                            message = "Currently Scrite handles switching between language input methods. Would you like to change that?"
                        else
                            message = "Currently the operating system handles switching between language input methods. Would you like to change that?"

                        MessageBox.question("Language Input", message, ["Yes", "No"], (answer) => {
                                                   if(answer === "Yes") {
                                                       LanguageOptionsDialog.launch()
                                                   }
                                               })
                    }
                }
            }
        }

        VclScrollBar {
            id: _scrollBar

            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            z: 1
            flickable: _elementListView
        }
    }

    ScreenplayEditorStatusBar {
        id: _statusBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        pageMargins: _private.pageMargins
        sceneHeadingFontMetrics: _private.sceneHeadingFontMetrics
        screenplayEditorListView: _elementListView
        screenplayEditorLastItemIndex: _elementListView.firstVisibleDelegateIndex
        screenplayEditorFirstItemIndex: _elementListView.lastVisibleDelegateIndex

        onZoomLevelJustChanged: _elementListView.afterZoomLevelChange()
        onZoomLevelIsAboutToChange: _elementListView.beforeZoomLevelChange()
    }

    Item {
        id: _splitterHandle

        anchors.top: _workspace.top
        anchors.bottom: _workspace.bottom

        x: _sidePanelLoader.width
        width: 20
        visible: _sidePanelLoader.active && _private.sidePanel.expanded

        Rectangle {
            anchors.fill: parent

            color: Runtime.colors.primary.button.background
            opacity: 0.5
            visible: _splitterHandleMouseArea.containsMouse || _splitterHandleMouseArea.drag.active
            border.width: 1
            border.color: Runtime.colors.primary.borderColor
        }

        Rectangle {
            anchors.centerIn: parent

            width: 1
            height: 10

            color: (_splitterHandleMouseArea.containsMouse || _splitterHandleMouseArea.drag.active) ?
                       Runtime.colors.primary.highlight.background : Runtime.colors.primary.button.background
        }

        MouseArea {
            id: _splitterHandleMouseArea

            anchors.fill: parent

            drag.axis: Drag.XAxis
            drag.target: parent
            drag.minimumX: _private.minSidePanelWidth
            drag.maximumX: _private.maxSidePanelWidth

            cursorShape: Qt.SplitHCursor
            hoverEnabled: true
        }

        onXChanged: {
            if(_splitterHandleMouseArea.drag.active) {
                if(__updateSidePanelWidthTimer === null)
                    __updateSidePanelWidthTimer = Runtime.execLater(_splitterHandle, Runtime.stdAnimationDuration/2, __updateSidePanelWidth)
                else
                    __updateSidePanelWidthTimer.restart()
            }
        }

        property Timer __updateSidePanelWidthTimer

        function __updateSidePanelWidth() {
            _private.sidePanel.maxPanelWidth = x
            Runtime.screenplayEditorSettings.sidePanelWidth = x
        }
    }

    HelpTipNotification {
        tipName: "screenplay"
    }

    ScreenplayEditorDropArea {
        id: _dropArea

        anchors.fill: parent
    }

    QtObject {
        id: _private

        property real minSidePanelWidth: root.width * 0.15
        property real maxSidePanelWidth: root.width * 0.5
        property bool sidePanelExpanded: sidePanel && sidePanel.expanded
        property bool showSceneComments: Runtime.screenplayEditorSettings.displaySceneComments && Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab
        property ScreenplayEditorSidePanel sidePanel: _sidePanelLoader.item

        property ScreenplayFormat screenplayFormat: Scrite.document.displayFormat
        property ScreenplayPageLayout pageLayout: screenplayFormat.pageLayout

        property SceneElementFormat sceneHeadingFormat: screenplayFormat.elementFormat(SceneElement.Heading)
        property FontMetrics sceneHeadingFontMetrics: FontMetrics {
            font: _private.sceneHeadingFormat.font2
        }

        property var pageMargins: Runtime.margins( _ruler.zoomLevel * dpi * pageLayout.leftMargin,
                                                 _ruler.zoomLevel * dpi * pageLayout.topMargin,
                                                 _ruler.zoomLevel * dpi * pageLayout.rightMargin,
                                                 _ruler.zoomLevel * dpi * pageLayout.bottomMargin )

        property int currentParagraphType: currentSceneDocumentBinder && currentSceneDocumentBinder.currentElement ? currentSceneDocumentBinder.currentElement.type : -1
        property var currentParagraphMargins: {
            if(currentParagraphType >= 0 && currentParagraphType) {
                const elementFormat = _private.screenplayFormat.elementFormat(currentParagraphType)
                const lm = _ruler.leftMargin + _private.pageLayout.contentWidth * elementFormat.leftMargin * Screen.devicePixelRatio
                const rm = _ruler.rightMargin + _private.pageLayout.contentWidth * elementFormat.rightMargin * Screen.devicePixelRatio
                return Runtime.margins(lm, 0, rm, 0)
            }
            return Runtime.margins(0, 0, 0, 0)
        }

        property real dpi: Screen.devicePixelRatio

        property alias zoomLevel: _statusBar.zoomLevel

        // It is so yuck that we have to ask an external entity to tell us what SceneDocumentBinder is "current", when
        // each and every single instance of binders are created by this component itself!
        property SceneDocumentBinder currentSceneDocumentBinder: ActionHub.binder

        readonly property Connections scriteDocumentConnections : Connections {
            target: Scrite.document

            function onJustLoaded() { _private.restoreLayoutDetails() }
            function onAboutToSave() { _private.saveLayoutDetails() }            
        }

        function initZoomLevelModifier() {
            const evalFn = () => {
                const pageLayout = screenplayFormat.pageLayout
                const zoomLevels = screenplayFormat.fontZoomLevels
                const indexOfZoomLevel = (val) => {
                    for(var i=0; i<zoomLevels.length; i++) {
                        if(zoomLevels[i] === val)
                            return i
                    }
                    return -1
                }

                const zoomOneValue = indexOfZoomLevel(1)

                const availableWidth = Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab ? root.width-500 : root.width

                let zoomValue = zoomOneValue
                let zoomLevel = zoomLevels[zoomValue]
                let pageWidth = pageLayout.paperWidth * zoomLevel * Screen.devicePixelRatio
                let totalMargin = availableWidth - pageWidth
                if(totalMargin < 0) {
                    while(totalMargin < 20) { // 20 is width of vertical scrollbar.
                        if(zoomValue-1 < 0)
                            break
                        zoomValue = zoomValue - 1
                        zoomLevel = zoomLevels[zoomValue]
                        pageWidth = pageLayout.paperWidth * zoomLevel * Screen.devicePixelRatio
                        totalMargin = availableWidth - pageWidth
                    }
                } else if(totalMargin > pageWidth/2) {
                    while(totalMargin > pageWidth/2) {
                        if(zoomValue >= zoomLevels.length-1)
                            break
                        zoomValue = zoomValue + 1
                        zoomLevel = zoomLevels[zoomValue]
                        pageWidth = pageLayout.paperWidth * zoomLevel * Screen.devicePixelRatio
                        totalMargin = availableWidth - pageWidth
                    }
                }

                return zoomValue - zoomOneValue
            }

            screenplayFormat.pageLayout.evaluateRectsNow()

            if(Runtime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor)
                _statusBar.zoomLevelModifier = evalFn()
            else {
                const tabWiseZoomLevelModifiers = Runtime.screenplayEditorSettings.zoomLevelModifiers
                const currentTabZoomLevelModifier = tabWiseZoomLevelModifiers["tab"+Runtime.mainWindowTab]
                if(currentTabZoomLevelModifier !== undefined)
                    _statusBar.zoomLevelModifier = currentTabZoomLevelModifier
            }

            _statusBar.zoomLevelChanged.connect(_private.saveCurrentTabZoomLevelModifier)
        }

        function saveCurrentTabZoomLevelModifier() {
            let tabWiseZoomLevelModifiers = Runtime.screenplayEditorSettings.zoomLevelModifiers
            tabWiseZoomLevelModifiers["tab"+Runtime.mainWindowTab] = _statusBar.zoomLevelModifierToApply()
            Runtime.screenplayEditorSettings.zoomLevelModifiers = tabWiseZoomLevelModifiers
        }

        function saveLayoutDetails() {
            if(_sidePanelLoader.active) {
                var userData = Scrite.document.userData
                userData["screenplayEditor"] = {
                    "version": 0,
                    "sceneListSidePanelExpaned": _sidePanelLoader.item.expanded
                }
                Scrite.document.userData = userData
            }
        }

        function restoreLayoutDetails() {
            if(_sidePanelLoader.active) {
                var userData = Scrite.document.userData
                if(userData.screenplayEditor && userData.screenplayEditor.version === 0)
                    _sidePanelLoader.item.expanded = userData.screenplayEditor.sceneListSidePanelExpaned
            }
        }

        Component.onCompleted: {
            initZoomLevelModifier()
            restoreLayoutDetails()

            Runtime.screenplayEditor = root

            if(Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab)
                Scrite.user.logActivity1("screenplay")

            if(Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab) {
                if(_elementListView.currentDelegate)
                    _elementListView.currentDelegate.focusIn()
            }
        }

        Component.onDestruction: {
            Runtime.screenplayEditor = null
        }
    }
}
