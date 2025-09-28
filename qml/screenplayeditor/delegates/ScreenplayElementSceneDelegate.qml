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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor/delegates/sidepanel"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

AbstractScreenplayElementDelegate {
    id: root

    required property real spaceAvailableForScenePanel
    required property ListView listView // This must be the list-view in which the delegate is placed.

    property var additionalSceneMenuItems: []

    signal jumpToNextScene()
    signal jumpToLastScene()
    signal jumpToFirstScene()
    signal jumpToPreviousScene()
    signal scrollToNextSceneRequest()
    signal scrollToPreviousSceneRequest()

    signal splitSceneRequest(SceneElement paragraph, int cursorPosition)
    signal mergeWithPreviousSceneRequest()

    signal additionalSceneMenuItemClicked(string name)

    content: Loader {
        id: _content

        height: __placeHolder ? __placeHolder.implicitHeight : (item ? item.implicitHeight : 0)

        active: false
        sourceComponent: _highResolution

        Component.onCompleted: {
            active = root.usePlaceholder ? (root.screenplayElementType !== ScreenplayElement.SceneElementType && !root.screenplayElement.omitted)
                                         : true
            if(!active) {
                __placeHolder = _lowResolution.createObject(_content, {"width": width})
                Utils.execLater(_content, Runtime.screenplayEditorSettings.placeholderInterval, activateContent)
            }
        }

        function activateContent() {
            active = true
        }

        onLoaded: {
            if(__placeHolder) {
                __placeHolder.destroy()
                __placeHolder = null
            }
        }

        property Item __placeHolder
    }

    Component {
        id: _lowResolution

        Rectangle {
            z: 1

            implicitHeight: _sceneSizeHint.active ? (_headerLayout.height + _sceneSizeHint.height + _pageBreakAfter.height)
                                                  : root.screenplayElement.heightHint * root.zoomLevel

            color: root.scene ? Qt.tint(root.scene.color, Runtime.colors.sceneHeadingTint) : Runtime.colors.primary.c300.background

            Column {
                id: _headerLayout

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                Item {
                    width: parent.width
                    height: root.screenplayElement.pageBreakBefore ? root.fontMetrics.lineSpacing*0.7 : 1
                }

                Row {
                    id: _placeHolderHeaderLayout

                    width: parent.width

                    VclLabel {
                        width: root.pageLeftMargin

                        text: root.screenplayElement.resolvedSceneNumber
                        font: root.font
                        color: root.screenplayElement.hasUserSceneNumber ? "black" : "gray"
                        topPadding: root.fontMetrics.lineSpacing * 0.5
                        bottomPadding: root.fontMetrics.lineSpacing * 0.5
                        rightPadding: 20
                        horizontalAlignment: Text.AlignRight
                    }

                    VclLabel {
                        width: parent.width - root.pageLeftMargin - root.pageRightMargin

                        font: root.font
                        color: screenplayElementType === ScreenplayElement.BreakElementType ? "gray" : "black"
                        elide: Text.ElideMiddle
                        topPadding: root.fontMetrics.lineSpacing * 0.5
                        bottomPadding: root.fontMetrics.lineSpacing * 0.5

                        text: {
                            if(root.screenplayElementType === ScreenplayElement.BreakElementType)
                                return root.screenplayElement.breakTitle
                            if(root.screenplayElement.omitted)
                                return "[OMITTED]"
                            if(root.scene && root.scene.heading.enabled)
                                return root.scene.heading.text
                            return "NO SCENE HEADING"
                        }
                    }
                }
            }

            Item {
                anchors.top: _headerLayout.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: _pageBreakAfter.top

                Image {
                    anchors.fill: parent

                    source: "qrc:/images/sample_scene.png"
                    opacity: 0.5
                    fillMode: Image.TileVertically
                }

                SceneSizeHintItem {
                    id: _sceneSizeHint

                    width: contentWidth * zoomLevel
                    height: contentHeight * zoomLevel

                    scene: root.scene
                    active: root.screenplayElement.heightHint === 0
                    format: Scrite.document.printFormat
                    visible: false
                    asynchronous: false
                }
            }

            Item {
                id: _pageBreakAfter

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                height: root.screenplayElement.pageBreakAfter ? root.fontMetrics.lineSpacing*0.7 : 1
            }
        }
    }

    Component {
        id: _highResolution

        Item {
            implicitHeight: _layout.height + Runtime.sceneEditorFontMetrics.lineSpacing

            /**
              Initially, I did use ColumnLayout here so that we are able to consistently use QtQuick Layouts
              everywhere. But then, the issue with ColumnLayout is that we invariably use Layout attached property
              in it to specify layout hints. So, we will end up using two objects instead of one to simply
              layout. As it is the delegates used for screenplay editor list view are rather heavy. We are better
              off using lightweight Column for trival layouting.
              */
            Column {
                id: _layout

                width: parent.width

                // Scene Heading Area
                Rectangle {
                    width: parent.width
                    height: _headingLayout.height

                    color: Qt.tint(root.scene.color, Runtime.colors.sceneHeadingTint)

                    Column {
                        id: _headingLayout

                        anchors.verticalCenter: parent.verticalCenter

                        width: parent.width

                        spacing: 5

                        SceneHeadingPartEditor {
                            id: _sceneHeadingEditor

                            width: parent.width

                            index: root.index
                            sceneID: root.sceneID
                            screenplayElement: root.screenplayElement
                            screenplayElementDelegateHasFocus: root.hasFocus

                            partName: "SceneHeading"
                            zoomLevel: root.zoomLevel
                            fontMetrics: root.fontMetrics
                            pageMargins: root.pageMargins
                            screenplayAdapter: root.screenplayAdapter
                            additionalSceneMenuItems: root.additionalSceneMenuItems

                            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                            onHasFocusChanged: {
                                if(hasFocus)
                                    root.currentParagraphType = SceneElement.Heading
                            }

                            onAdditionalSceneMenuItemClicked: (name) => {
                                root.additionalSceneMenuItemClicked(name)
                            }
                        }

                        Loader {
                            width: parent.width

                            active: Runtime.screenplayEditorSettings.displaySceneCharacters
                            visible: active

                            sourceComponent: SceneStoryBeatTagsPartEditor {
                                index: root.index
                                sceneID: root.sceneID
                                screenplayElement: root.screenplayElement
                                screenplayElementDelegateHasFocus: root.hasFocus

                                partName: "StoryBeats"
                                zoomLevel: root.zoomLevel
                                fontMetrics: Runtime.idealFontMetrics
                                pageMargins: root.pageMargins
                                screenplayAdapter: root.screenplayAdapter

                                onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                                // TODO
                                onSceneTagAdded: (tagName) => { }
                                onSceneTagClicked: (tagName) => { }
                            }
                        }

                        Loader {
                            width: parent.width

                            active: Runtime.screenplayEditorSettings.displaySceneCharacters
                            visible: active

                            sourceComponent: SceneCharacterListPartEditor {
                                index: root.index
                                sceneID: root.sceneID
                                screenplayElement: root.screenplayElement
                                screenplayElementDelegateHasFocus: root.hasFocus

                                partName: "CharacterList"
                                zoomLevel: root.zoomLevel
                                fontMetrics: Runtime.idealFontMetrics
                                pageMargins: root.pageMargins
                                screenplayAdapter: root.screenplayAdapter

                                // TODO
                                additionalCharacterMenuItems: []

                                onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                                // TODO
                                onNewCharacterAdded: (characterName) => { }
                                onAdditionalCharacterMenuItemClicked: (characterName, menuItemName) => { }
                            }
                        }

                        Loader {
                            width: parent.width

                            active: Runtime.screenplayEditorSettings.displaySceneSynopsis
                            visible: active

                            sourceComponent: SceneSynopsisPartEditor {
                                index: root.index
                                sceneID: root.sceneID
                                screenplayElement: root.screenplayElement
                                screenplayElementDelegateHasFocus: root.hasFocus

                                partName: "Synopsis"
                                zoomLevel: root.zoomLevel
                                fontMetrics: Runtime.idealFontMetrics
                                pageMargins: root.pageMargins
                                screenplayAdapter: root.screenplayAdapter

                                onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
                            }
                        }

                        Item {
                            width: parent.width
                            height: root.fontMetrics.lineSpacing/2
                            visible: Runtime.screenplayEditorSettings.displaySceneCharacters && !Runtime.screenplayEditorSettings.displaySceneSynopsis
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: _sceneContentEditor.height

                    SceneContentEditor {
                        id: _sceneContentEditor

                        width: parent.width

                        index: root.index
                        sceneID: root.sceneID
                        screenplayElement: root.screenplayElement
                        screenplayElementDelegateHasFocus: root.hasFocus

                        focus: true
                        partName: "SceneContent"
                        zoomLevel: root.zoomLevel
                        fontMetrics: root.fontMetrics
                        pageMargins: root.pageMargins
                        screenplayAdapter: root.screenplayAdapter

                        onHasFocusChanged: {
                            if(hasFocus)
                                root.currentParagraphType = Qt.binding( () => { return currentParagraphType } )
                        }

                        onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                        onJumpToLastScene: () => { root.jumpToLastScene() }
                        onJumpToNextScene: () => { root.jumpToNextScene() }
                        onJumpToFirstScene: () => { root.jumpToFirstScene() }
                        onJumpToPreviousScene: () => { root.jumpToPreviousScene() }
                        onEditSceneHeadingRequest: () => { _sceneHeadingEditor.focus = true }
                        onScrollToNextSceneRequest: () => { root.scrollToNextSceneRequest() }
                        onScrollToPreviousSceneRequest: () => { root.scrollToPreviousSceneRequest() }
                        onSplitSceneRequest: (paragraph, cursorPosition) => { root.splitSceneRequest(paragraph, cursorPosition) }
                        onMergeWithPreviousSceneRequest: () => { root.mergeWithPreviousSceneRequest() }
                    }

                    SceneTextEditorPageNumbersLoader {
                        id: _pageNumbersLoader

                        anchors.fill: parent

                        zoomLevel: root.zoomLevel
                        fontMetrics: root.fontMetrics
                        sceneTextEditor: _sceneContentEditor.editor
                        screenplayElement: root.screenplayElement
                    }

                    Connections {
                        target: root

                        function on__ZoomLevelJustChanged() {
                            _sceneContentEditor.afterZoomLevelChange()
                        }

                        function on__ZoomLevelAboutToChange() {
                            _sceneContentEditor.beforeZoomLevelChange()
                        }
                    }
                }
            }

            Loader {
                id: _sidePanelLoader

                property real __screenY: __evaluateScreenY()
                property real __maxTopMargin: root.height - height

                anchors.top: parent.top
                anchors.left: parent.right
                anchors.topMargin: __screenY < 0 ? Math.min(-__screenY, __maxTopMargin) : 0

                active: Runtime.screenplayEditorSettings.displaySceneComments &&
                        Runtime.mainWindowTab === Runtime.e_ScreenplayTab &&
                        root.spaceAvailableForScenePanel >= Runtime.minSceneSidePanelWidth

                sourceComponent: SceneSidePanel {
                    height: 300

                    index: root.index
                    sceneID: root.sceneID
                    screenplayElement: root.screenplayElement
                    screenplayElementDelegateHasFocus: root.hasFocus

                    partName: "SidePanel"
                    zoomLevel: 1
                    fontMetrics: Runtime.idealFontMetrics
                    pageMargins: Utils.margins(0, 0, 0, 0)
                    screenplayAdapter: root.screenplayAdapter

                    label: expanded ? evaluateLabel() : ""
                    readOnly: root.readOnly
                    listView: root.listView
                    maxPanelWidth: Math.min(root.spaceAvailableForScenePanel, Runtime.maxSceneSidePanelWidth)

                    onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                    function evaluateLabel() {
                        let rsn = root.screenplayElement.resolvedSceneNumber
                        if(rsn === "")
                            rsn = "#" + (root.index + 1)

                        if(_sidePanelLoader.__screenY >= 0)
                            return "Scene " + rsn
                        return rsn + ". " + (root.scene.heading.enabled ? root.scene.heading.displayText : "NO SCENE HEADING")
                    }
                }

                Connections {
                    target: root.listView

                    function onContentYChanged() {
                        _sidePanelLoader.__screenY = _sidePanelLoader.__evaluateScreenY()
                    }
                }

                function __evaluateScreenY() {
                    return root.listView.mapFromItem(root, 0, 0).y
                }
            }

            Connections {
                target: root

                function on__FocusIn(cursorPosition) {
                    _sceneContentEditor.assumeFocusAt(cursorPosition)
                }

                function on__FocusOut() { }

                function on__SearchBarSaysReplaceCurrent(replacementText, searchAgent) {
                    _sceneContentEditor.__searchBarSaysReplaceCurrent(replacementText, searchAgent)
                }
            }

            onHeightChanged: Qt.callLater(updateHeightHint)

            function updateHeightHint() {
                if(height > 0 && root && root.screenplayElement && root.zoomLevel > 0)
                    root.screenplayElement.heightHint = height / zoomLevel
            }
        }
    }
}
