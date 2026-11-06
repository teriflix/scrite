/****************************************************************************
**
** Copyright (C) 2026 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com)
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

import Scrite.App

import "../../globals"
import "../../helpers"
import "../../controls"
import "../helpers"

Item {
    id: root

    required property Note note

    required property real maxTextAreaSize
    required property real minTextAreaSize

    signal deleteRequest(Note note)

    // The tab says which note this is, so the views have no title.
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: _private.viewFor(root.note)
        }

        AttachmentsView {
            Layout.fillWidth: true

            attachments: root.note ? root.note.attachments : null
        }
    }

    AttachmentsDropArea {
        anchors.fill: parent

        allowMultiple: true
        target: root.note ? root.note.attachments : null
    }

    Component {
        id: _formView

        FormView {
            note: root.note
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
            showHeading: false
            viewAccessory: _deleteButton

            ActionHandler {
                action: ActionHub.notebookOperations.find("report")

                enabled: true
                tooltip: "Export current form note as a PDF or ODT."

                onTriggered: () => {
                                 let generator = Scrite.document.createReportGenerator("Notebook Report")
                                 generator.section = root.note
                                 ReportConfigurationDialog.launch(generator)
                             }
            }
        }
    }

    Component {
        id: _unknownView

        Column {
            anchors.centerIn: parent

            spacing: 12

            SctLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Unknown note type."

                font.pointSize: Runtime.notebookFontPointSize
            }

            Loader {
                anchors.horizontalCenter: parent.horizontalCenter

                sourceComponent: _deleteButton
            }
        }
    }

    Component {
        id: _deleteButton

        FlatToolButton {
            suggestedWidth: 28

            iconSource: Runtime.themedIcon("qrc:/icons/action/delete.png")
            toolTipText: "Delete this note"
            visible: !Scrite.document.readOnly

            onClicked: root.deleteRequest(root.note)
        }
    }

    QtObject {
        id: _private

        // The view for a type of note. A new type of singleton note needs a Component above, and a
        // case here.
        function viewFor(note) {
            switch(note.type) {
            case Note.FormNoteType:
                return _formView
            }

            return _unknownView
        }
    }
}
