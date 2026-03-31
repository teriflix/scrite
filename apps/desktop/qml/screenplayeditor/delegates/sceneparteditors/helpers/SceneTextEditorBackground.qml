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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components


import "../../../../helpers"
import "../../../../globals"
import "../../../../controls"
import "../../../../structureview"
import "../../.."

Item {
    id: root

    required property real zoomLevel

    required property TextEdit sceneTextEditor
    required property SceneDocumentBinder sceneDocumentBinder

    // Current line highlight
    Rectangle {
        x: 0
        y: root.sceneTextEditor.cursorRectangle.y-2*root.zoomLevel
        width: parent.width
        height: root.sceneTextEditor.cursorRectangle.height+4*root.zoomLevel

        color: Runtime.colors.tintTx(root.sceneDocumentBinder.scene.highlightColor, Runtime.colors.currentLineHightlightTint)
        visible: root.sceneTextEditor.cursorVisible && root.sceneTextEditor.activeFocus && Runtime.screenplayEditorSettings.highlightCurrentLine && Scrite.app.usingMaterialTheme

        Rectangle {
            width: 20 * root.zoomLevel
            height: parent.height

            color: root.sceneDocumentBinder.scene.highlightColor
        }
    }

    VclText {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.topMargin: root.sceneTextEditor.topPadding
        anchors.leftMargin: root.sceneTextEditor.leftPadding
        anchors.rightMargin: root.sceneTextEditor.rightPadding

        font: root.sceneTextEditor.font

        text: root.sceneTextEditor.activeFocus ? "Start typing scene content" : "Click here to type scene content ..."
        visible: root.sceneTextEditor.text === ""
        opacity: 0.5
    }
}
