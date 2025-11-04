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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"

Rectangle {
    id: root

    // These come from the model (either Screenplay or ScreenplayAdapter)
    required property int index
    required property int breakType
    required property int screenplayElementType
    required property var modelData
    required property Scene scene
    required property ScreenplayElement screenplayElement

    // These are additional
    required property bool readOnly
    required property bool viewHasFocus
    required property real leftPadding
    required property real rightPadding
    required property real leftPaddingRatio
    required property real sceneIconSize
    required property real sceneIconPadding

    required property string dragDropMimeType

    required property ScreenplayAdapter screenplayAdapter

    signal clicked()
    signal dragStarted()
    signal doubleClicked()
    signal dragFinished(var dropAction) // When we move to Qt 6.9+, change type to DropAction
    signal dropEntered(var drag) // When we move to Qt 6.9+, change type to DragEvent
    signal dropExited()
    signal dropRequest(var drop) // When we move to Qt 6.9+, change type to DragEvent
    signal contextMenuRequest()
    signal collapseSideListPanelRequest()

    Drag.active: _mouseArea.drag.active
    Drag.source: root.screenplayElement
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.MoveAction
    Drag.mimeData: _private.dragMimeData
    Drag.onDragStarted: {
        if(!root.screenplayElement.selected && root.screenplayAdapter.isSourceScreenplay)
           root.screenplayAdapter.screenplay.clearSelection()
        root.screenplayElement.selected = true
        if(root.screenplayElementType === ScreenplayElement.BreakElementType)
            root.screenplayAdapter.currentIndex = index
        root.dragStarted()
    }
    Drag.onDragFinished: (dropAction) => { root.dragFinished(dropAction) }

    height: _mainLayout.height

    color: _private.color
    border.width: _private.isCurrent && root.viewHasFocus
    border.color: Qt.darker(_private.color, 120)

    ColumnLayout {
        id: _mainLayout

        width: parent.width
        spacing: 0

        Loader {
            Layout.fillWidth: true

            active: root.screenplayElement.pageBreakBefore
            visible: active

            sourceComponent: PageBreakItem {
                fullSize: false
                placement: Qt.TopEdge
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: _contentLayout.height + 16
            Layout.preferredHeight: _contentLayout.height + 16

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom

                width: root.sceneIconPadding * (_private.multiSelection ? (_private.isCurrent ? 0.85 : 0.5) : 0.5)

                color: Runtime.colors.accent.windowColor
                visible: _private.isSelection
            }

            SceneTypeImage {
                id: _sceneTypeImage

                anchors.top: _private.isSceneTextModeHeading ? undefined : _contentLayout.top
                anchors.left: parent.left
                anchors.leftMargin: root.sceneIconPadding
                anchors.verticalCenter: _private.isSceneTextModeHeading ? _contentLayout.verticalCenter : undefined

                width: root.sceneIconSize
                height: root.sceneIconSize

                visible: root.leftPaddingRatio > 0
                opacity: (_private.isCurrent ? 1 : 0.5) * root.leftPaddingRatio
                sceneType: root.scene ? root.scene.type : Scene.Standard
                showTooltip: false
                lightBackground: Color.isLight(root.color)
            }

            RowLayout {
                id: _contentLayout

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: root.leftPadding
                anchors.rightMargin: root.rightPadding
                anchors.verticalCenter: parent.verticalCenter

                spacing: 5

                Loader {
                    Layout.alignment: _private.isSceneTextModeHeading ? Qt.AlignVCenter : Qt.AlignTop
                    Layout.preferredWidth: root.sceneIconSize
                    Layout.preferredHeight: root.sceneIconSize

                    active: !_private.isBreak && !root.screenplayElement.omitted &&
                            Runtime.screenplayEditorSettings.longSceneWarningEnabled &&
                            root.scene.wordCount > Runtime.screenplayEditorSettings.longSceneWordTreshold
                    visible: active

                    sourceComponent: Image {
                        smooth: true
                        mipmap: true
                        source: "qrc:/icons/content/warning.png"
                        fillMode: Image.PreserveAspectFit

                        MouseArea {
                            ToolTip.text: "" + root.scene.wordCount + " words (limit: " + Runtime.screenplayEditorSettings.longSceneWordTreshold + "). Refer Settings > Screenplay > Options tab."
                            ToolTip.visible: containsMouse

                            anchors.fill: parent

                            hoverEnabled: enabled
                        }
                    }
                }

                VclLabel {
                    id: _label

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    text: _private.text
                    elide: _private.isSceneTextModeHeading ? Text.ElideMiddle : Text.ElideRight
                    color: Runtime.colors.primary.c10.text
                    wrapMode: _private.isSceneTextModeHeading ? Text.NoWrap : Text.WrapAtWordBoundaryOrAnywhere
                    maximumLineCount: _private.isSceneTextModeHeading ? 1 : Runtime.bounded(1,Runtime.screenplayEditorSettings.slpSynopsisLineCount,5)
                    verticalAlignment: Qt.AlignVCenter
                    horizontalAlignment: Qt.AlignLeft

                    font.bold: _private.isCurrent || _private.isBreak
                    font.family: Runtime.sceneEditorFontMetrics.font.family
                    // font.pointSize: Math.ceil(Runtime.idealFontMetrics.font.pointSize*(delegateItem.elementIsBreak ? 1.2 : 1))
                    font.capitalization: _private.isBreak || _private.isSceneTextModeHeading ? Font.AllUppercase : Font.MixedCase

                    Loader {
                        anchors.fill: parent

                        active: _label.truncated && Runtime.sceneListPanelSettings.showTooltip

                        sourceComponent: MouseArea {
                            ToolTip.text: _label.text
                            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                            ToolTip.visible: containsMouse

                            hoverEnabled: true
                        }
                    }
                }

                Loader {
                    Layout.preferredWidth: root.sceneIconSize
                    Layout.preferredHeight: root.sceneIconSize
                    Layout.alignment: _private.isSceneTextModeHeading ? Qt.AlignVCenter : Qt.AlignTop

                    active: !_private.isBreak && !root.scene.hasContent
                    visible: active

                    sourceComponent: Image {
                        smooth: true
                        mipmap: true
                        source: "qrc:/icons/content/empty_scene.png"
                        fillMode: Image.PreserveAspectFit

                        MouseArea {
                            ToolTip.text: "This scene is empty."
                            ToolTip.visible: containsMouse

                            anchors.fill: parent

                            enabled: parent.visible
                            hoverEnabled: enabled
                        }
                    }
                }

                VclLabel {
                    Layout.alignment: Qt.AlignVCenter

                    text: _private.sceneLength
                    color: Runtime.colors.primary.c10.text
                    visible: !Runtime.screenplayTextDocument.paused && (Runtime.sceneListPanelSettings.displaySceneLength === "PAGE" || Runtime.sceneListPanelSettings.displaySceneLength === "TIME")
                    opacity: 0.5

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize-3
                }
            }
        }

        Loader {
            Layout.fillWidth: true

            visible: active
            active: root.screenplayElement.pageBreakAfter

            sourceComponent: PageBreakItem {
                fullSize: false
                placement: Qt.BottomEdge
            }
        }
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        preventStealing: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        drag.axis: Drag.YAxis
        drag.target: root.screenplayAdapter.isSourceScreenplay && !root.readOnly ? parent : null

        onDoubleClicked: (mouse) => {
                             root.doubleClicked()

                             root.screenplayAdapter.screenplay.clearSelection()
                             root.screenplayElement.toggleSelection()
                             root.screenplayAdapter.currentIndex = index
                             root.collapseSideListPanelRequest()
                         }

        onClicked: (mouse) => {
                       root.clicked()

                       if(mouse.button === Qt.RightButton) {
                           root.screenplayAdapter.currentIndex = index
                           root.contextMenuRequest()
                           return
                       }

                       if(root.screenplayAdapter.isSourceScreenplay) {
                           const isShiftPressed = mouse.modifiers & Qt.ShiftModifier
                           const isControlPressed = mouse.modifiers & Qt.ControlModifier
                           if(isControlPressed) {
                               root.screenplayElement.toggleSelection()
                           } else if(isShiftPressed) {
                               const fromIndex = Math.min(root.screenplayAdapter.currentIndex, index)
                               const toIndex = Math.max(root.screenplayAdapter.currentIndex, index)
                               if(fromIndex === toIndex) {
                                   root.screenplayElement.toggleSelection()
                               } else {
                                   for(let i=fromIndex; i<=toIndex; i++) {
                                       let element = root.screenplayAdapter.screenplay.elementAt(i)
                                       if(element.elementType === ScreenplayElement.SceneElementType) {
                                           element.selected = true
                                       }
                                   }
                               }
                           } else {
                               root.screenplayAdapter.screenplay.clearSelection()
                               root.screenplayElement.toggleSelection()
                           }
                       }

                       root.screenplayAdapter.currentIndex = index
                   }
    }

    DropArea {
        id: _dropArea

        anchors.fill: parent

        keys: [root.dragDropMimeType]
        enabled: !screenplayElement.selected && !root.readOnly

        onEntered: (drag) => {
                       drag.acceptProposedAction()
                       root.dropEntered(drag)
                   }

        onExited: root.dropExited()

        onDropped: (drop) => {
                       root.dropRequest(drop)
                       drop.acceptProposedAction()
                   }
    }

    Rectangle {
        id: _dropIndicator

        width: parent.width
        height: 2

        color: Runtime.colors.primary.borderColor
        visible: _dropArea.containsDrag
    }

    /**
    Rectangle {
        anchors.top: _dropIndicator.visible ? _dropIndicator.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: 1

        color: _private.isEpisodeBreak ? Runtime.colors.accent.c200.background : Runtime.colors.accent.c100.background
        visible: _private.isBreak
    }
    */

    QtObject {
        id: _private

        property bool multiSelection: root.screenplayAdapter.screenplay.selectedElementsCount > 1

        property bool isBreak: root.screenplayElementType === ScreenplayElement.BreakElementType
        property bool isCurrent: root.screenplayAdapter.currentIndex === root.index
        property bool isSelection: (isCurrent || screenplayElement.selected)
        property bool isEpisodeBreak: root.screenplayElementType === ScreenplayElement.BreakElementType && root.breakType === Screenplay.Episode
        property bool isSceneTextModeHeading: Runtime.sceneListPanelSettings.sceneTextMode === "HEADING"

        property color color: {
            if(root.scene)
                return isSelection ? selectedColor :
                                           (root.screenplayAdapter.isSourceScreenplay && multiSelection ?
                                                Qt.tint(normalColor, "#40FFFFFF") : normalColor)
            return isCurrent ? Color.translucent(Runtime.colors.accent.windowColor, 0.25) : Qt.rgba(0,0,0,0.01)
        }
        property color normalColor: root.scene ? Qt.tint(root.scene.color, Runtime.colors.sceneHeadingTint) : Runtime.colors.primary.c200.background
        property color selectedColor: root.scene ?
                                          (Color.isVeryLight(root.scene.color) ?
                                               Qt.tint(Runtime.colors.primary.highlight.background, Runtime.colors.selectedSceneHeadingTint) :
                                               Qt.tint(root.scene.color, Runtime.colors.selectedSceneHeadingTint)) :
                                          Runtime.colors.primary.c300.background

        property var dragMimeData: {
            let ret = {}
            ret[root.dragDropMimeType] = root.screenplayElement.sceneID
            return ret
        }

        property string text: {
            let ret = "UNKNOWN"

            if(root.scene) {
                const sceneHeading = root.scene.heading
                if(isSceneTextModeHeading) {
                    if(sceneHeading.enabled) {
                        ret = root.screenplayElement.resolvedSceneNumber + ". "
                        if(root.screenplayElement.omitted)
                            ret += "[OMITTED] <font color=\"gray\">" + root.scene.heading.text + "</font>"
                        else
                            ret += sceneHeading.text
                    } else if(screenplayElement.omitted)
                        ret = "[OMITTED]"
                    else
                        ret = "NO SCENE HEADING"
                } else {
                    let summary = root.scene.summary
                    if(sceneHeading.enabled) {
                        ret = root.screenplayElement.resolvedSceneNumber + ". "
                        if(root.screenplayElement.omitted)
                            ret += "[OMITTED] <font color=\"gray\">" + summary + "</font>"
                        else
                            ret += summary
                    } else if(root.screenplayElement.omitted)
                        ret = "[OMITTED]" + summary
                    else
                        ret = summary
                }

                return ret
            }

            if(isBreak) {
                if(isEpisodeBreak)
                    ret = root.screenplayElement.breakTitle
                else if(root.screenplayAdapter.isSourceScreenplay && root.screenplayAdapter.screenplay.episodeCount > 1)
                    ret = "Ep " + (root.screenplayElement.episodeIndex+1) + ": " + root.screenplayElement.breakTitle
                else
                    ret = root.screenplayElement.breakTitle
                if(root.screenplayElement.breakSubtitle !== "")
                    ret +=  ": " + root.screenplayElement.breakSubtitle
                return ret
            }

            return ret
        }

        property string sceneLength: evaluateSceneLength()

        function evaluateSceneLength() {
            if(root.scene) {
                if(Runtime.sceneListPanelSettings.displaySceneLength === "PAGE") {
                    const pl = Runtime.screenplayTextDocument.lengthInPages(root.screenplayElement, null)
                    return Math.round(pl*100,2)/100
                }
                if(Runtime.sceneListPanelSettings.displaySceneLength === "TIME")
                    return Runtime.screenplayTextDocument.lengthInTimeAsString(root.screenplayElement, null)
            }
            return ""
        }

        function updateSceneLength() {
            _private.sceneLength = _private.evaluateSceneLength()
        }

        Component.onCompleted: {
            Runtime.screenplayTextDocument.pausedChanged.connect(updateSceneLength)
            Runtime.screenplayTextDocument.pageBoundariesChanged.connect(updateSceneLength)
            Runtime.sceneListPanelSettings.displaySceneLengthChanged.connect(updateSceneLength)
        }

        Component.onDestruction: {
            Runtime.screenplayTextDocument.pausedChanged.disconnect(updateSceneLength)
            Runtime.screenplayTextDocument.pageBoundariesChanged.disconnect(updateSceneLength)
            Runtime.sceneListPanelSettings.displaySceneLengthChanged.disconnect(updateSceneLength)
        }
    }
}
