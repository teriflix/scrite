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

import "../../../helpers"
import "../../../globals"
import "../../../controls"

AbstractScenePartEditor {
    id: root

    implicitHeight: _synopsisLoader.height + root.fontMetrics.lineSpacing * 1.5

    LodLoader {
        id: _synopsisLoader

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
            height: _synopsisView.implicitHeight

            TextArea {
                id: _synopsisView

                width: parent.width

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font: root.font
                text: root.scene && root.scene.hasSynopsis ? root.scene.synopsis : "Scene Synopsis"
                opacity: root.scene && root.scene.hasSynopsis ? 1 : 0.5
                readOnly: true
                leftPadding: 0
                rightPadding: 0
                verticalAlignment: Text.AlignTop
                background: Item { }

            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.IBeamCursor

                onClicked: Qt.callLater(_synopsisLoader.highLod)
            }

            ActionHandler {
                action: ActionHub.editOptions.find("editSceneSynopsis")
                enabled: root.isCurrent && !root.readOnly

                onTriggered: () => { Qt.callLater(_synopsisLoader.highLod) }
            }
        }

        highDetailComponent: TextAreaInput {
            id: _synopsisInput

            Keys.onPressed: (event) => {
                                if(event.key === Qt.Key_Escape && root.isCurrent) {
                                    const editSceneContent = ActionHub.editOptions.find("editSceneContent") as Action
                                    editSceneContent.trigger()
                                }
                            }

            Component.onCompleted: {
                Qt.callLater(() => {
                                 text = root.scene.synopsis
                                 forceActiveFocus()
                                 selectAll()
                             })
            }

            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            readOnly: root.readOnly
            font: root.font
            initialText: ""
            undoRedoEnabled: true
            placeholderText: "Scene Synopsis"
            leftPadding: 0
            rightPadding: 0
            background: Item { }

            onTextEdited: root.scene.setSynopsisDirectly(text)
            onEditingFinished: root.scene.setSynopsisDirectly(text)

            onActiveFocusChanged: {
                if(activeFocus)
                    root.ensureVisible(_synopsisInput, Qt.rect(0, -10, cursorRectangle.width, cursorRectangle.height+20))
                else if(!_synopsisInput.persistentSelection)
                    Qt.callLater(_synopsisLoader.lowLod)
            }
        }
    }
}
