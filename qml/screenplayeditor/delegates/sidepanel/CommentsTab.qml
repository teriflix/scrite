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

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors"

AbstractScenePartEditor {
    id: root

    TextAreaInput {
        id: _commentsEdit

        ToolTip.text: "Please consider capturing long comments as scene notes in the notebook tab."
        ToolTip.delay: 1000
        ToolTip.visible: height < contentHeight

        anchors.fill: parent

        text: root.scene.comments
        wrapMode: Text.WordWrap
        placeholderText: "Comments"

        topPadding: 10
        leftPadding: 10
        rightPadding: 10
        bottomPadding: 10

        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 1

        background: Rectangle {
            color: Qt.tint(scene.color, Runtime.colors.sceneHeadingTint)
        }
        readOnly: Scrite.document.readOnly

        SpecialSymbolsSupport {
            anchors.top: parent.bottom
            anchors.left: parent.left

            enabled: !_commentsEdit.readOnly
            textEditor: _commentsEdit
            textEditorHasCursorInterface: true
        }

        TextAreaSpellingSuggestionsMenu {

        }

        onTextChanged: () => {
                           if(root.scene && root.scene.comments !== text) {
                               root.scene.comments = text
                           }
                       }
    }
}
