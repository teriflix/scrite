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
import QtQuick.Layouts 1.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/dialogs"
import "qrc:/qml/notebookview"

AbstractNotebookPage {
    id: root

    Rectangle {
        anchors.fill: parent

        color: Qt.tint(note.color, Runtime.colors.sceneHeadingTint)
    }

    ColumnLayout {
        anchors.fill: parent

        CheckListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            note: _private.note
        }

        AttachmentsView {
            Layout.fillWidth: true

            attachments: _private.note ? _private.note.attachments : null
        }
    }

    AttachmentsDropArea {
        id: _dropArea

        anchors.fill: parent

        allowMultiple: true
        target: _private.note ? _private.note.attachments : null
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export this text note as a PDF or ODT."

        onTriggered: (source) => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.note
                         ReportConfigurationDialog.launch(rgen)
                     }
    }

    QtObject {
        id: _private

        property Note note: root.pageData ? root.pageData.notebookItemObject : null
    }
}
