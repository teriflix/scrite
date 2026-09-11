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
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../../.."
import "../../../../helpers"
import "../../../../globals"
import "../../../../controls"
import "../../../../structureview"

Item {
    id: root

    required property real zoomLevel

    required property TextEdit sceneTextEditor
    required property SceneDocumentBinder sceneDocumentBinder

    function beforeZoomLevelChange() {
        _private.zoomLevelSettled = false
    }

    function afterZoomLevelChange() {
        _private.evalDualDialogueRects()
        _private.zoomLevelSettled = true
    }

    // Current line highlight
    Rectangle {
        id: _currentLineHighlight

        x: 0
        y: root.sceneTextEditor.cursorRectangle.y-2*root.zoomLevel
        width: parent.width-1
        height: root.sceneTextEditor.cursorRectangle.height+4*root.zoomLevel

        color: Runtime.colors.tintTx(root.sceneDocumentBinder.scene.highlightColor, Runtime.colors.currentLineHightlightTint)
        visible: root.sceneTextEditor.cursorVisible && root.sceneTextEditor.activeFocus && Runtime.screenplayEditorSettings.highlightCurrentLine && Scrite.app.usingGpuAcceleratedTheme

        Rectangle {
            width: 20 * root.zoomLevel
            height: parent.height

            color: root.sceneDocumentBinder.scene.highlightColor
        }

        onYChanged: _private.evalDualDialogueRects()
    }

    // Show a dotted rectangle around all dual-dialogues if the binder has its
    // renderDualDialogues property set to false.
    Repeater {
        id: _otherDualDialogues

        model: root.sceneDocumentBinder.renderDualDialogues ? [] : root.sceneDocumentBinder.scene.dualDialogues

        delegate: Rectangle {
            required property var modelData

            property rect dualDialogueRect
            property SceneDualDialogue dualDialogue: modelData as SceneDualDialogue

            function evalDualDialogueRect() {
                dualDialogueRect = root.sceneDocumentBinder.evalDualDialogueRect(dualDialogue)
            }

            x: root.sceneTextEditor.leftPadding - radius
            y: dualDialogueRect.y + root.sceneTextEditor.topPadding - _currentLineHighlight.height * 0.33
            width: root.sceneTextEditor.contentWidth + 2*radius
            height: dualDialogueRect.height + _currentLineHighlight.height * 0.66
            topLeftRadius: _currentLineHighlight.height
            bottomRightRadius: topLeftRadius

            color: dualDialogue === root.sceneDocumentBinder.currentDualDialogue ?
                       Runtime.colors.tx( Runtime.colors.primary.c400.background ) : Qt.rgba(0,0,0,0)

            border.width: 1
            border.color: Runtime.colors.primary.editor.text

            opacity: 0.5
            visible: dualDialogueRect.height > 1

            // Dual-dialogue indicator
            Image {
                anchors.left: parent.right
                anchors.top: parent.top
                anchors.margins: _currentLineHighlight.height/2

                width: _currentLineHighlight.height
                height: width

                source: Runtime.themedIcon("qrc:/icons/content/dual_dialogue.png")

                MouseArea {
                    id: _ddiMouseArea

                    anchors.fill: parent
                    cursorShape: Qt.ArrowCursor
                    hoverEnabled: true

                    ToolTipPopup {
                        container: _ddiMouseArea
                        text: "This is a dual dialogue displayed sequentially while editing."
                        visible: _ddiMouseArea.containsMouse
                    }
                }
            }
        }

        onCountChanged: Qt.callLater(_private.evalDualDialogueRects)
    }

    VclText {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.topMargin: root.sceneTextEditor.topPadding
        anchors.leftMargin: root.sceneTextEditor.leftPadding
        anchors.rightMargin: root.sceneTextEditor.rightPadding

        font: root.sceneTextEditor.font

        text: "Click here to type scene content ..."
        visible: text === "" && !root.sceneTextEditor.activeFocus
        opacity: 0.5
    }

    QtObject {
        id: _private

        property bool zoomLevelSettled: true

        function evalDualDialogueRects() {
            for(let i=0; i<_otherDualDialogues.count; i++) {
                let item = _otherDualDialogues.itemAt(i)
                item.evalDualDialogueRect()
            }
        }
    }
}
