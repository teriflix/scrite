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

import "../../../helpers"
import "../../../globals"
import "../../../controls"

AbstractScenePartEditor {
    id: root

    implicitHeight: _synopsisInput.contentHeight + root.fontMetrics.lineSpacing * 1.5

    TextAreaInput {
        id: _synopsisInput

        Keys.onPressed: (event) => {
                            if(event.key === Qt.Key_Escape && root.isCurrent) {
                                const editSceneContent = ActionHub.editOptions.find("editSceneContent") as Action
                                editSceneContent.trigger()
                            }
                        }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: root.pageLeftMargin
        anchors.rightMargin: root.pageRightMargin
        anchors.bottomMargin: root.fontMetrics.lineSpacing * root.zoomLevel * 0.5

        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        readOnly: root.readOnly
        font: root.font
        initialText: root.scene.synopsis
        placeholderText: "Scene Synopsis"
        background: Item { }

        onTextChanged: if(activeFocus) root.scene.synopsis = text
        onEditingFinished: root.scene.synopsis = text

        onActiveFocusChanged: {
            if(activeFocus)
                root.ensureVisible(_synopsisInput, Qt.rect(0, -10, cursorRectangle.width, cursorRectangle.height+20))
        }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("editSceneSynopsis")
        enabled: root.isCurrent && !root.readOnly && !_synopsisInput.activeFocus

        onTriggered: () => {
                         _synopsisInput.selectAll()
                         _synopsisInput.forceActiveFocus()
                     }
    }
}
