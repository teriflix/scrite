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
import QtQuick.Controls

import Scrite.App

import "../../../globals"
import "../../../helpers"
import "../sceneparteditors"

AbstractScenePartEditor {
    id: root

    implicitHeight: _commentsEdit.contentHeight + root.fontMetrics.lineSpacing * 1.5

    Rectangle {
        anchors.fill: parent

        color: Runtime.colors.tintTx(root.scene.color, Runtime.colors.sceneHeadingTint)
        border.width: 1
        border.color: Runtime.colors.primary.borderColor

        TextAreaInput {
            id: _commentsEdit

            PlaceholderVisibility.visible: false

            anchors.fill: parent
            anchors.margins: 5

            initialText: root.scene.comments
            wrapMode: Text.WordWrap
            placeholderText: "Comments"

            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10
            background.visible: false

            font: root.font
            readOnly: Scrite.document.readOnly

            onTextEdited: () => {
                              if(root.scene && root.scene.comments !== text) {
                                  root.scene.comments = text
                              }
                          }
        }
    }
}
