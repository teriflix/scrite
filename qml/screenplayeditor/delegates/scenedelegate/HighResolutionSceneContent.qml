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

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor/delegates"
import "qrc:/qml/screenplayeditor/delegates/sidepanel"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

Item {
    id: root

    required property bool showSceneComments
    required property AbstractScreenplayElementSceneDelegate sceneDelegate

    implicitHeight: Math.max(_layout.height + Runtime.sceneEditorFontMetrics.lineSpacing,
                             _sidePanelLoader.active && _sidePanelLoader.item.expanded ? _sidePanelLoader.height : 0)

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

            color: Runtime.colors.tint(sceneDelegate.scene.color, sceneDelegate.isCurrent ? Runtime.colors.selectedSceneHeadingTint : Runtime.colors.sceneHeadingTint)

            Column {
                id: _headingLayout

                anchors.verticalCenter: parent.verticalCenter

                width: parent.width

                spacing: 5

                SceneHeadingPartEditor {
                    id: _sceneHeadingEditor

                    width: parent.width

                    index: sceneDelegate.index
                    sceneID: sceneDelegate.sceneID
                    screenplayElement: sceneDelegate.screenplayElement
                    screenplayElementDelegateHasFocus: sceneDelegate.hasFocus

                    partName: "SceneHeading"
                    isCurrent: sceneDelegate.isCurrent
                    zoomLevel: sceneDelegate.zoomLevel
                    fontMetrics: sceneDelegate.fontMetrics
                    pageMargins: sceneDelegate.pageMargins
                    screenplayAdapter: sceneDelegate.screenplayAdapter

                    onEnsureVisible: (item, area) => { sceneDelegate.ensureVisible(item, area) }

                    onHasFocusChanged: {
                        if(hasFocus)
                            sceneDelegate.currentParagraphType = SceneElement.Heading
                    }
                }

                Loader {
                    width: parent.width

                    active: Runtime.screenplayEditorSettings.displaySceneCharacters
                    visible: active

                    sourceComponent: SceneStoryBeatTagsPartEditor {
                        index: sceneDelegate.index
                        sceneID: sceneDelegate.sceneID
                        screenplayElement: sceneDelegate.screenplayElement
                        screenplayElementDelegateHasFocus: sceneDelegate.hasFocus

                        partName: "StoryBeats"
                        isCurrent: sceneDelegate.isCurrent
                        zoomLevel: sceneDelegate.zoomLevel
                        fontMetrics: Runtime.idealFontMetrics
                        pageMargins: sceneDelegate.pageMargins
                        screenplayAdapter: sceneDelegate.screenplayAdapter

                        onEnsureVisible: (item, area) => { sceneDelegate.ensureVisible(item, area) }

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
                        index: sceneDelegate.index
                        sceneID: sceneDelegate.sceneID
                        screenplayElement: sceneDelegate.screenplayElement
                        screenplayElementDelegateHasFocus: sceneDelegate.hasFocus

                        partName: "CharacterList"
                        isCurrent: sceneDelegate.isCurrent
                        zoomLevel: sceneDelegate.zoomLevel
                        fontMetrics: Runtime.idealFontMetrics
                        pageMargins: sceneDelegate.pageMargins
                        screenplayAdapter: sceneDelegate.screenplayAdapter

                        onEnsureVisible: (item, area) => { sceneDelegate.ensureVisible(item, area) }

                        onNewCharacterAdded: (characterName) => { }
                    }
                }

                Loader {
                    width: parent.width

                    active: Runtime.screenplayEditorSettings.displaySceneSynopsis
                    visible: active

                    sourceComponent: SceneSynopsisPartEditor {
                        index: sceneDelegate.index
                        sceneID: sceneDelegate.sceneID
                        screenplayElement: sceneDelegate.screenplayElement
                        screenplayElementDelegateHasFocus: sceneDelegate.hasFocus

                        partName: "Synopsis"
                        isCurrent: sceneDelegate.isCurrent
                        zoomLevel: sceneDelegate.zoomLevel
                        fontMetrics: Runtime.idealFontMetrics
                        pageMargins: sceneDelegate.pageMargins
                        screenplayAdapter: sceneDelegate.screenplayAdapter

                        onEnsureVisible: (item, area) => { sceneDelegate.ensureVisible(item, area) }
                    }
                }

                Item {
                    width: parent.width
                    height: sceneDelegate.fontMetrics.lineSpacing/2
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

                index: sceneDelegate.index
                sceneID: sceneDelegate.sceneID
                screenplayElement: sceneDelegate.screenplayElement
                screenplayElementDelegateHasFocus: sceneDelegate.hasFocus

                focus: true
                partName: "SceneContent"
                isCurrent: sceneDelegate.isCurrent
                zoomLevel: sceneDelegate.zoomLevel
                fontMetrics: sceneDelegate.fontMetrics
                pageMargins: sceneDelegate.pageMargins
                screenplayAdapter: sceneDelegate.screenplayAdapter

                onHasFocusChanged: {
                    if(hasFocus)
                        sceneDelegate.currentParagraphType = Qt.binding( () => { return currentParagraphType } )
                }

                onEnsureVisible: (item, area) => { sceneDelegate.ensureVisible(item, area) }

                onSplitSceneRequest: (paragraph, cursorPosition) => { sceneDelegate.splitSceneRequest(paragraph, cursorPosition) }
                onMergeWithPreviousSceneRequest: () => { sceneDelegate.mergeWithPreviousSceneRequest() }
            }

            SceneTextEditorPageNumbers {
                id: _pageNumbersLoader

                anchors.fill: parent

                isCurrent: sceneDelegate.isCurrent
                zoomLevel: sceneDelegate.zoomLevel
                fontMetrics: Runtime.minimumFontMetrics
                sceneTextEditor: _sceneContentEditor.editor
                screenplayElement: sceneDelegate.screenplayElement
            }

            Connections {
                target: sceneDelegate

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
        property real __maxTopMargin: sceneDelegate.height - height

        anchors.top: parent.top
        anchors.left: parent.right
        anchors.topMargin: __screenY < 0 ? Math.min(-__screenY, __maxTopMargin) : 0

        active: root.showSceneComments &&
                sceneDelegate.spaceAvailableForScenePanel >= Runtime.minSceneSidePanelWidth

        sourceComponent: SceneSidePanel {
            height: 300

            index: sceneDelegate.index
            sceneID: sceneDelegate.sceneID
            screenplayElement: sceneDelegate.screenplayElement
            screenplayElementDelegateHasFocus: sceneDelegate.hasFocus

            partName: "SidePanel"
            isCurrent: sceneDelegate.isCurrent
            zoomLevel: 1
            fontMetrics: Runtime.idealFontMetrics
            pageMargins: Runtime.margins(0, 0, 0, 0)
            screenplayAdapter: sceneDelegate.screenplayAdapter

            label: expanded ? evaluateLabel() : ""
            readOnly: sceneDelegate.readOnly
            listView: sceneDelegate.listView
            maxPanelWidth: Math.min(sceneDelegate.spaceAvailableForScenePanel, Runtime.maxSceneSidePanelWidth)

            onEnsureVisible: (item, area) => { sceneDelegate.ensureVisible(item, area) }

            function evaluateLabel() {
                let rsn = sceneDelegate.screenplayElement.resolvedSceneNumber
                if(rsn === "")
                    rsn = "#" + (sceneDelegate.index + 1)

                if(_sidePanelLoader.__screenY >= 0)
                    return "Scene " + rsn
                return rsn + ". " + (sceneDelegate.scene.heading.enabled ? sceneDelegate.scene.heading.displayText : "NO SCENE HEADING")
            }
        }

        Connections {
            target: sceneDelegate.listView

            function onContentYChanged() {
                _sidePanelLoader.__screenY = _sidePanelLoader.__evaluateScreenY()
            }
        }

        function __evaluateScreenY() {
            return sceneDelegate.listView.mapFromItem(sceneDelegate, 0, 0).y
        }
    }

    Connections {
        target: sceneDelegate

        function on__FocusIn(cursorPosition) {
            _sceneContentEditor.assumeFocusAt(cursorPosition)
        }

        function on__FocusOut() { }

        function on__SearchBarSaysReplaceCurrent(replacementText, searchAgent) {
            _sceneContentEditor.__searchBarSaysReplaceCurrent(replacementText, searchAgent)
        }

        function onZoomLevelChanged() {
            _sidePanelLoader.__evaluateScreenY()
        }
    }

    onHeightChanged: Qt.callLater(__updateHeightHint)

    function __updateHeightHint() {
        if(height > 0 && sceneDelegate && sceneDelegate.screenplayElement && sceneDelegate.zoomLevel > 0)
            sceneDelegate.screenplayElement.heightHint = height / zoomLevel
    }}
