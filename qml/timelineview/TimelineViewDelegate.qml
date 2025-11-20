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
import QtQuick.Shapes 1.5
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    required property ListView screenplayElementList

    // Since this will be used as a delegate with Screenplay model, the following properties will come from that model
    required property int index
    required property string sceneID
    required property ScreenplayElement screenplayElement
    required property int screenplayElementType
    required property int breakType
    required property Scene scene

    signal editorRequest()
    signal dropSceneAtRequest(QtObject source, int index)

    width: _private.isBreakElement ? screenplayElementList.breakDelegateWidth :
            (root.screenplayElement.omitted ? screenplayElementList.omittedDelegateWidth
                             : Math.max(screenplayElementList.minimumDelegateWidth, _private.sceneElementCount*screenplayElementList.perElementWidth*zoomLevel))
    height: screenplayElementList.height

    Rectangle {
        anchors.fill: _elementItemBoxArea
        anchors.margins: -5

        visible: root.screenplayElement.selected

        color: Runtime.colors.accent.a700.background
    }

    Item {
        id: _elementItemBoxArea

        anchors.fill: parent
        anchors.leftMargin: 7.5
        anchors.rightMargin: 2.5
        anchors.bottomMargin: screenplayElementList.scrollBarRequired ? 17 : 3

        enabled: !_delegateDropArea.containsDrag

        Rectangle {
            id: _elementItemBox

            property color sceneColor2 : Qt.tint(_private.sceneColor, (root.screenplayElement.selected || _private.active) ? "#9CFFFFFF" : Runtime.colors.sceneControlTint)

            anchors.fill: parent

            color: root.scene ? (root.screenplayElement.omitted ? Qt.tint(sceneColor2, Runtime.colors.sceneControlTint) : sceneColor2) : _private.sceneColor
            border.color: color === Qt.rgba(1,1,1,1) ? "black" : _private.sceneColor
            border.width: _private.active ? 2 : 1

            Behavior on border.width {
                enabled: Runtime.applicationSettings.enableAnimations
                NumberAnimation { duration: 400 }
            }

            Loader {
                id: _breakTypeIconLoader

                anchors.top: parent.top
                anchors.margins: 10
                anchors.horizontalCenter: parent.horizontalCenter

                width: 24; height: 24
                active: _private.isBreakElement

                sourceComponent: Image {
                    source: _private.isEpisodeBreak ? "qrc:/icons/content/episode_inverted.png" : "qrc:/icons/content/act_inverted.png"
                }
            }

            Loader {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: _breakTypeIconLoader.active ? parent.bottom : _dragTriggerButton.top
                anchors.topMargin: _breakTypeIconLoader.active ? 0 : (_notesIconLoader.active ? 30 : 5)
                anchors.leftMargin: 5
                anchors.rightMargin: 5

                active: _private.isBreakElement || parent.width > screenplayElementList.minimumDelegateWidthForTextVisibility
                visible: active

                sourceComponent: Item {
                    VclLabel {
                        anchors.centerIn: parent

                        width: parent.width
                        height: parent.height

                        text: _private.sceneTitle

                        color: root.scene ? Color.textColorFor(_elementItemBox.color) : _private.colorPalette.text
                        elide: Text.ElideRight
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        font.bold: _private.isBreakElement || _private.active
                        lineHeight: 1.25
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        transformOrigin: Item.Center
                        maximumLineCount: _private.isBreakElement ? 2 : 5
                        verticalAlignment: _private.isBreakElement ? Text.AlignVCenter : Text.AlignTop
                        horizontalAlignment: _private.isBreakElement ? Text.AlignHCenter : Text.AlignLeft
                        font.capitalization: _private.isBreakElement ? Font.AllUppercase : Font.MixedCase
                    }
                }
            }

            MouseArea {
                id: _mouseArea

                Drag.active: _dragMouseArea.drag.active
                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.MoveAction
                Drag.hotSpot.x: width/2
                Drag.hotSpot.y: height/2
                Drag.source: root.screenplayElement
                Drag.mimeData: {
                    let md = {}
                    md[Runtime.timelineViewSettings.dropAreaKey] = root.sceneID
                    return md
                }
                Drag.onActiveChanged: {
                    if(!_private.isBreakElement)
                        Scrite.document.screenplay.currentElementIndex = root.index
                    screenplayElementList.moveMode = Drag.active
                }

                anchors.fill: parent

                hoverEnabled: true // !isBreakElement
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                ToolTipPopup {
                    y: -height - 15
                    container: _mouseArea
                    text: _private.evalToolTipText()
                    visible: _mouseArea.hoverEnabled && _mouseArea.containsMouse
                }

                onPressed: screenplayElementList.forceActiveFocus()

                onClicked: {
                    if(mouse.button === Qt.RightButton) {
                        if(root.screenplayElementType === ScreenplayElement.BreakElementType) {
                            _breakElementContextMenu.element = screenplayElement
                            _breakElementContextMenu.popup(this)
                        } else {
                            _sceneElementsContextMenu.element = screenplayElement
                            _sceneElementsContextMenu.popup(this)
                        }

                        Scrite.document.screenplay.currentElementIndex = root.index
                        root.editorRequest()

                        return
                    }

                    const isControlPressed = mouse.modifiers & Qt.ControlModifier
                    const isShiftPressed = mouse.modifiers & Qt.ShiftModifier
                    screenplayElementList.forceActiveFocus()
                    screenplayElementList.mutiSelectionMode = isControlPressed || isShiftPressed
                    if(isControlPressed)
                        root.screenplayElement.toggleSelection()
                    else if(isShiftPressed) {
                        function selectRange() {
                            const fromIndex = Math.min(Scrite.document.screenplay.currentElementIndex,root.index)
                            const toIndex = Math.max(Scrite.document.screenplay.currentElementIndex,root.index)
                            if(fromIndex === toIndex)
                                return
                            for(var i=fromIndex; i<=toIndex; i++) {
                                var element = Scrite.document.screenplay.elementAt(i)
                                if(element.elementType === ScreenplayElement.SceneElementType)
                                    element.selected = true
                            }
                        }
                        selectRange()
                    } else
                        Scrite.document.screenplay.clearSelection()

                    Scrite.document.screenplay.currentElementIndex = root.index
                    root.editorRequest()
                }
            }

            Loader {
                id: _notesIconLoader

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 5

                width: 24; height: 24

                active: showNotesIcon && (root.scene && root.scene.noteCount >= 1)
                opacity: 0.4

                sourceComponent: Image {
                    source: "qrc:/icons/content/bookmark_outline.png"
                }
            }

            Row {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 5

                visible: !_private.isBreakElement && parent.width > screenplayElementList.minimumDelegateWidthForTextVisibility

                Loader {
                    width: 24; height: 24

                    active: !_private.isBreakElement && Runtime.screenplayEditorSettings.longSceneWarningEnabled &&
                            root.scene.wordCount > Runtime.screenplayEditorSettings.longSceneWordTreshold
                    visible: active

                    sourceComponent: Image {
                        smooth: true
                        mipmap: true
                        source: Color.isLight(_elementItemBox.color) ? "qrc:/icons/content/warning.png" : "qrc:/icons/content/warning_inverted.png"
                        fillMode: Image.PreserveAspectFit

                        MouseArea {
                            id: _warningIconMouseArea

                            anchors.fill: parent

                            enabled: parent.visible
                            hoverEnabled: enabled

                            ToolTipPopup {
                                container: _warningIconMouseArea
                                text: "" + root.scene.wordCount + " words (limit: " + Runtime.screenplayEditorSettings.longSceneWordTreshold + ").\nRefer Settings > Screenplay > Options tab."
                                visible: _warningIconMouseArea.containsMouse
                            }
                        }
                    }
                }

                SceneTypeImage {
                    width: 24; height: 24

                    visible: sceneType !== Scene.Standard
                    sceneType: root.scene ? root.scene.type : Scene.Standard
                    showTooltip: false
                    lightBackground: Color.isLight(_elementItemBox.color)
                }
            }


            Image {
                id: _dragTriggerButton

                width: 24; height: 24

                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: parent.width > width+10 ? 5 : (parent.width - width)/2

                scale: _dragMouseArea.containsMouse ? 2 : 1
                source: "qrc:/icons/action/view_array.png"
                opacity: _dragMouseArea.containsMouse ? 1 : 0.1
                visible: !Scrite.document.readOnly && enableDragDrop
                enabled: visible

                Behavior on scale {
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: Runtime.stdAnimationDuration }
                }

                MouseArea {
                    id: _dragMouseArea

                    anchors.fill: parent

                    enabled: enableDragDrop
                    drag.target: parent
                    cursorShape: Qt.SizeAllCursor
                    hoverEnabled: true

                    onPressed: {
                        if(!root.screenplayElement.selected)
                            Scrite.document.screenplay.clearSelection()
                        root.screenplayElement.selected = true
                        screenplayElementList.forceActiveFocus()
                        root.grabToImage(function(result) {
                            root.Drag.imageSource = result.url
                        })
                    }
                }
            }

            Loader {
                anchors.fill: parent
                anchors.margins: parent.width * 0.25

                active: root.screenplayElement.omitted

                sourceComponent: Image {
                    source: "qrc:/icons/content/omitted_scene.png"
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    DropArea {
        id: _delegateDropArea

        anchors.fill: parent

        keys: [Runtime.timelineViewSettings.dropAreaKey]
        enabled: !root.screenplayElement.selected

        onEntered: (drag) => {
                       screenplayElementList.forceActiveFocus()
                       drag.acceptProposedAction()
                   }

        onDropped: (drop) => {
                       root.dropSceneAtRequest(drop.source, root.index)
                       drop.acceptProposedAction()
                   }
    }

    Rectangle {
        id: _dropAreaIndicator

        property bool highlightAsDropArea: _delegateDropArea.containsDrag

        anchors.left: parent.left

        width: 5
        height: parent.height

        color: highlightAsDropArea ? dropAreaHighlightColor : Qt.rgba(0,0,0,0)
    }

    QtObject {
        id: _private

        property bool isBreakElement: root.screenplayElementType === ScreenplayElement.BreakElementType
        property bool isEpisodeBreak: isBreakElement && root.breakType === Screenplay.Episode
        property bool active: root.scene ? Scrite.document.screenplay.activeScene === root.scene : false
        property int sceneElementCount: root.scene ? root.scene.elementCount : 1

        property string sceneTitle: {
            var ret = ""
            var escene = root.scene
            if(escene) {
                var sheading = escene.heading
                if(sheading.enabled)
                    ret += "[" + root.screenplayElement.resolvedSceneNumber + "]: "

                if(Runtime.timelineViewSettings.textMode === "HeadingOrTitle") {
                    var selement = escene.structureElement
                    var ntitle = selement.nativeTitle
                    if(ntitle !== "")
                        ret += ntitle
                    else if(selement.stackId === "" || selement.stackLeader) {
                        if(sheading.enabled)
                            ret += sheading.text
                        else
                            ret += "NO SCENE HEADING"
                    }
                } else
                    ret += escene.synopsis
            } else if(isEpisodeBreak)
                ret = "EP " + (root.screenplayElement.episodeIndex+1)
            else
                ret = root.screenplayElement.breakTitle

            return ret;
        }

        property var colorPalette: {
            if(root.scene) {
                if(Color.isLight(root.scene.color))
                    return { "background": root.scene.color, "text": "black" }
                return { "background": root.scene.color, "text": "white" }
            }
            if(root.breakType === Screenplay.Episode)
                return Runtime.colors.accent.c700
            return Runtime.colors.accent.c500
        }

        property color sceneColor: colorPalette.background

        readonly property ScreenplayPaginatorWatcher sceneLengthWatcher: ScreenplayPaginatorWatcher {
            paginator: Runtime.paginator.paused ? null : Runtime.paginator
            element: root.screenplayElement
        }

        function evalToolTipText() {
            let ret = ""

            if(_private.isBreakElement) {
                const idxList = Scrite.document.screenplay.sceneElementsInBreak(root.screenplayElement)
                if(idxList.length === 0)
                    return "No Scenes"

                if(idxList.length === 1)
                    ret = "1 Scene"
                else
                    ret = idxList.length + " Scenes"

                if(_private.sceneLengthWatcher.hasValidRecord)
                    ret += ", Duration: " + TMath.timeLengthString(_private.sceneLengthWatcher.timeLength)

                return ret
            }

            if(root.screenplayElement.omitted)
                return "Omitted Scene"

            let pc = root.scene.elementCount
            ret += pc + " " + (pc > 1 ? "Paragraphs" : "Paragraph")

            if(_private.sceneLengthWatcher.hasValidRecord)
                ret += ", Duration: " + TMath.timeLengthString(_private.sceneLengthWatcher.timeLength)

            if(root.width < screenplayElementList.minimumDelegateWidthForTextVisibility) {
                let str = _private.sceneTitle

                if(str.length > 140)
                    str = str.substring(0, 130) + "..."

                if(str.length > 0)
                    ret = "(" + ret + ") " + str
            }

            return ret
        }
    }

}

