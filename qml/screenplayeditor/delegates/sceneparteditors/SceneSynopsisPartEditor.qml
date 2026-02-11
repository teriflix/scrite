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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"

AbstractScenePartEditor {
    id: root

    implicitHeight: _synopsisInput.contentHeight + root.fontMetrics.lineSpacing

    TextAreaInput {
        id: _synopsisInput


        Keys.onPressed: (event) => {
                            if(event.key === Qt.Key_Escape && root.isCurrent) {
                                const editSceneContent = ActionHub.editOptions.find("editSceneContent")
                                editSceneContent.trigger()
                            }
                        }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: root.pageLeftMargin
        anchors.rightMargin: root.pageRightMargin

        text: root.scene.synopsis
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        readOnly: root.readOnly
        placeholderText: "Scene Synopsis"

        font.family: root.font.family
        font.pointSize: Math.max( Math.ceil(root.font.pointSize * root.zoomLevel), Runtime.minimumFontMetrics.font.pointSize)

        background: Item { }

        TextAreaSpellingSuggestionsMenu {
            textArea: _synopsisInput
        }

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

        onTriggered: (source) => {
                         _synopsisInput.selectAll()
                         _synopsisInput.forceActiveFocus()
                     }
    }
}
