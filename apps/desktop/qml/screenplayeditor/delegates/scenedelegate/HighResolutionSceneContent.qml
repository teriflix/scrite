/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import io.scrite.components

import "../"
import "../../../globals"
import "../../../dialogs"
import "../../../helpers"
import "../../../controls"
import "../sidepanel"
import "../sceneparteditors"
import "../sceneparteditors/helpers"

Item {
    id: root

    required property bool showSceneComments
    required property AbstractScreenplayElementSceneDelegate sceneDelegate

    height: implicitHeight
    implicitHeight: Math.max(_layout.height + sceneDelegate.fontMetrics.lineSpacing,
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

            color: Runtime.colors.tintTx(root.sceneDelegate.scene.color, root.sceneDelegate.isCurrent ? Runtime.colors.selectedSceneHeadingTint : Runtime.colors.sceneHeadingTint)

            Column {
                id: _headingLayout

                anchors.verticalCenter: parent.verticalCenter

                width: parent.width

                spacing: 5

                SceneHeadingPartEditor {
                    id: _sceneHeadingEditor

                    width: parent.width

                    index: root.sceneDelegate.index
                    sceneID: root.sceneDelegate.sceneID
                    screenplayElement: root.sceneDelegate.screenplayElement
                    screenplayElementDelegateHasFocus: root.sceneDelegate.hasFocus

                    partName: "SceneHeading"
                    isCurrent: root.sceneDelegate.isCurrent
                    zoomLevel: root.sceneDelegate.zoomLevel
                    fontMetrics: root.sceneDelegate.fontMetrics
                    pageMargins: root.sceneDelegate.pageMargins
                    screenplayAdapter: root.sceneDelegate.screenplayAdapter

                    onEnsureVisible: (item, area) => { _ensureTimer.ensureVisible(item, area) }

                    onHasFocusChanged: {
                        if(hasFocus)
                            root.sceneDelegate.currentParagraphType = SceneElement.Heading
                    }
                }

                Loader {
                    width: parent.width
                    height: item ? item.implicitHeight : 0

                    active: Runtime.screenplayEditorSettings.displaySceneCharacters
                    visible: active

                    sourceComponent: SceneStoryBeatTagsPartEditor {
                        index: root.sceneDelegate.index
                        sceneID: root.sceneDelegate.sceneID
                        screenplayElement: root.sceneDelegate.screenplayElement
                        screenplayElementDelegateHasFocus: root.sceneDelegate.hasFocus

                        partName: "StoryBeats"
                        isCurrent: root.sceneDelegate.isCurrent
                        zoomLevel: root.sceneDelegate.zoomLevel
                        fontMetrics: Runtime.partFontMetrics
                        pageMargins: root.sceneDelegate.pageMargins
                        screenplayAdapter: root.sceneDelegate.screenplayAdapter

                        onEnsureVisible: (item, area) => { _ensureTimer.ensureVisible(item, area) }

                        // TODO
                        onSceneTagAdded: (tagName) => { }
                        onSceneTagClicked: (tagName) => { }
                    }
                }

                Loader {
                    width: parent.width
                    height: item ? item.implicitHeight : 0

                    active: Runtime.screenplayEditorSettings.displaySceneCharacters
                    visible: active

                    sourceComponent: SceneCharacterListPartEditor {
                        index: root.sceneDelegate.index
                        sceneID: root.sceneDelegate.sceneID
                        screenplayElement: root.sceneDelegate.screenplayElement
                        screenplayElementDelegateHasFocus: root.sceneDelegate.hasFocus

                        partName: "CharacterList"
                        isCurrent: root.sceneDelegate.isCurrent
                        zoomLevel: root.sceneDelegate.zoomLevel
                        fontMetrics: Runtime.partFontMetrics
                        pageMargins: root.sceneDelegate.pageMargins
                        screenplayAdapter: root.sceneDelegate.screenplayAdapter

                        onEnsureVisible: (item, area) => { _ensureTimer.ensureVisible(item, area) }

                        onNewCharacterAdded: (characterName) => { }
                    }
                }

                Loader {
                    width: parent.width
                    height: item ? item.implicitHeight : 0

                    active: Runtime.screenplayEditorSettings.displaySceneSynopsis
                    visible: active

                    sourceComponent: SceneSynopsisPartEditor {
                        index: root.sceneDelegate.index
                        sceneID: root.sceneDelegate.sceneID
                        screenplayElement: root.sceneDelegate.screenplayElement
                        screenplayElementDelegateHasFocus: root.sceneDelegate.hasFocus

                        partName: "Synopsis"
                        isCurrent: root.sceneDelegate.isCurrent
                        zoomLevel: root.sceneDelegate.zoomLevel
                        fontMetrics: Runtime.partFontMetrics
                        pageMargins: root.sceneDelegate.pageMargins
                        screenplayAdapter: root.sceneDelegate.screenplayAdapter

                        onEnsureVisible: (item, area) => { _ensureTimer.ensureVisible(item, area) }
                    }
                }

                Item {
                    width: parent.width
                    height: root.sceneDelegate.fontMetrics.lineSpacing/2
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

                index: root.sceneDelegate.index
                sceneID: root.sceneDelegate.sceneID
                screenplayElement: root.sceneDelegate.screenplayElement
                screenplayElementDelegateHasFocus: root.sceneDelegate.hasFocus

                focus: true
                partName: "SceneContent"
                listView: root.sceneDelegate.listView
                isCurrent: root.sceneDelegate.isCurrent
                zoomLevel: root.sceneDelegate.zoomLevel
                fontMetrics: root.sceneDelegate.fontMetrics
                pageMargins: root.sceneDelegate.pageMargins
                screenplayAdapter: root.sceneDelegate.screenplayAdapter
                ensureCursorCenteredAction: root.sceneDelegate.ensureCursorCenteredAction

                onHasFocusChanged: {
                    if(hasFocus)
                        root.sceneDelegate.currentParagraphType = Qt.binding( () => { return currentParagraphType } )
                }

                onEnsureVisible: (item, area) => {
                                     if(cursorPosition === 0) {
                                        _ensureTimer.ensureVisible(_sceneHeadingEditor, Qt.rect(0, 0, area.width, area.height))
                                     } else
                                        _ensureTimer.ensureVisible(item, area)
                                 }
                onEnsureCentered: (item, area) => { _ensureTimer.ensureCentered(item, area) }

                onSplitSceneRequest: (paragraph, cursorPosition) => { root.sceneDelegate.splitSceneRequest(paragraph, cursorPosition) }
                onMergeWithPreviousSceneRequest: () => { root.sceneDelegate.mergeWithPreviousSceneRequest() }
            }

            SceneTextEditorPageNumbers {
                id: _pageNumbersLoader

                anchors.fill: parent

                isCurrent: root.sceneDelegate.isCurrent
                zoomLevel: root.sceneDelegate.zoomLevel
                fontMetrics: Runtime.idealFontMetrics
                sceneTextEditor: _sceneContentEditor.editor
                screenplayElement: root.sceneDelegate.screenplayElement
                zeroPositionOffset: _headingLayout.height
            }

            Connections {
                target: root.sceneDelegate

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
        property real __maxTopMargin: root.sceneDelegate.height - height

        anchors.top: parent.top
        anchors.left: parent.right
        anchors.topMargin: __screenY < 0 ? Math.min(-__screenY, __maxTopMargin) : 0

        active: root.showSceneComments &&
                root.sceneDelegate.spaceAvailableForScenePanel >= Runtime.minSceneSidePanelWidth

        sourceComponent: SceneSidePanel {
            height: 300

            index: root.sceneDelegate.index
            sceneID: root.sceneDelegate.sceneID
            screenplayElement: root.sceneDelegate.screenplayElement
            screenplayElementDelegateHasFocus: root.sceneDelegate.hasFocus

            partName: "SidePanel"
            isCurrent: root.sceneDelegate.isCurrent
            zoomLevel: 1
            fontMetrics: Runtime.idealFontMetrics
            pageMargins: Runtime.margins(0, 0, 0, 0)
            screenplayAdapter: root.sceneDelegate.screenplayAdapter

            label: expanded ? evaluateLabel() : ""
            readOnly: root.sceneDelegate.readOnly
            listView: root.sceneDelegate.listView
            maxPanelWidth: Math.min(root.sceneDelegate.spaceAvailableForScenePanel, Runtime.maxSceneSidePanelWidth)

            onEnsureVisible: (item, area) => { _ensureTimer.ensureVisible(item, area) }

            function evaluateLabel() {
                let rsn = root.sceneDelegate.screenplayElement.resolvedSceneNumber
                if(rsn === "")
                    rsn = "#" + (root.sceneDelegate.index + 1)

                if(_sidePanelLoader.__screenY >= 0)
                    return "Scene " + rsn
                return rsn + ". " + (root.sceneDelegate.scene.heading.enabled ? root.sceneDelegate.scene.heading.displayText : "NO SCENE HEADING")
            }
        }

        Connections {
            target: root.sceneDelegate.listView

            function onContentYChanged() {
                _sidePanelLoader.__screenY = _sidePanelLoader.__evaluateScreenY()
            }
        }

        function __evaluateScreenY() {
            return root.sceneDelegate.listView.mapFromItem(root.sceneDelegate, 0, 0).y
        }
    }

    Connections {
        target: root.sceneDelegate.scene

        function onSceneAboutToReset() {
            _ensureTimer.stop()
            _ensureTimer.interval = 20000 // Set to a large value just to make sure that we keep batching all ensureXYZ() calls
        }

        function onSceneReset() {
            _ensureTimer.stop()
            _ensureTimer.interval = Runtime.stdAnimationDuration/2 // Set to a reasonable value, so that they get processed soon.
            _ensureTimer.start()
        }
    }

    Timer {
        id: _ensureTimer

        property int kind: 0 // 0=UnknownKind, 1=EnsureVisibleKind, 2=EnsureCenteredKind
        property Item item
        property rect area: Qt.rect(0,0,0,0)

        function ensureVisible(item_, area_) {
            if(interval === 0 && !running) {
                root.sceneDelegate.ensureVisible(item_, area_)
                return
            }

            kind = 1
            item = item_
            area = area_
            start()
        }

        function ensureCentered(item_, area_) {
            if(running && kind == 1)
                return // There is already a ensureVisible queued.

            kind = 2
            item = item_
            area = area_
            start()
        }

        repeat: false
        interval: 0

        onTriggered: {
            if(item === null || kind < 1 || kind > 2 || area === Qt.rect(0,0,0,0)) {
                interval = 0
                return
            }

            if(kind == 1) {
                root.sceneDelegate.ensureVisible(item, area)
            } else {
                root.sceneDelegate.ensureCentered(item, area)
            }

            item = null
            area = Qt.rect(0,0,0,0)
            kind = 0
            interval = 0
        }
    }

    Connections {
        target: root.sceneDelegate

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
        if(height > 0 && sceneDelegate && root.sceneDelegate.screenplayElement && root.sceneDelegate.zoomLevel > 0)
            root.sceneDelegate.screenplayElement.heightHint = height / root.sceneDelegate.zoomLevel
    }
}
