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

import Scrite.App

import "../../../helpers"
import "../../../globals"
import "../../../controls"

AbstractElementPartEditor {
    id: root

    implicitHeight: _summaryLoader.height + root.fontMetrics.lineSpacing * 1.5

    LodLoader {
        id: _summaryLoader

        function lowLod() { lod = LodLoader.LOD.Low }
        function highLod() { lod = LodLoader.LOD.High }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: root.pageLeftMargin
        anchors.rightMargin: root.pageRightMargin
        anchors.bottomMargin: root.fontMetrics.lineSpacing * root.zoomLevel * 0.5

        lod: LodLoader.LOD.Low

        lowDetailComponent: Item {
            height: _summaryView.implicitHeight

            TextArea {
                id: _summaryView

                width: parent.width

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font: root.font
                text: root.screenplayElement.breakSummary !== "" ? root.screenplayElement.breakSummary : "Break Summary"
                opacity: root.screenplayElement.breakSummary !== "" ? 1 : 0.5
                readOnly: true
                leftPadding: 0
                rightPadding: 0
                verticalAlignment: Text.AlignTop
                background: Item { }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.IBeamCursor

                onClicked: Qt.callLater(_summaryLoader.highLod)
            }
        }

        highDetailComponent: TextAreaInput {
            id: _summaryInput

            Component.onCompleted: {
                Qt.callLater(() => {
                                 text = root.screenplayElement.breakSummary
                                 forceActiveFocus()
                                 selectAll()
                             })
            }

            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            readOnly: root.readOnly
            font: root.font
            initialText: ""
            undoRedoEnabled: true
            markdownSyntaxEnabled: true
            placeholderText: "Break Summary"
            leftPadding: 0
            rightPadding: 0
            background: Item { }

            onTextEdited: root.screenplayElement.breakSummary = text
            onEditingFinished: root.screenplayElement.breakSummary = text

            onActiveFocusChanged: {
                if(activeFocus)
                    root.ensureVisible(_summaryInput, Qt.rect(0, -10, cursorRectangle.width, cursorRectangle.height+20))
                else if(!_summaryInput.persistentSelection)
                    Qt.callLater(_summaryLoader.lowLod)
            }
        }
    }
}
