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

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object, event, result) => {
                              EventFilter.forwardEventTo(_summaryField)
                              result.filter = true
                              result.accepted = true
                          }

    ColumnLayout {
        anchors.top: parent.top
        anchors.bottom: _attachmentsView.top
        anchors.margins: 11
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-30

        VclLabel {
            Layout.fillWidth: true

            text: _private.breakElement.breakTitle

            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 3
        }

        VclTextField {
            id: _headingField

            Layout.fillWidth: true

            label: ""
            placeholderText: _private.breakKind + " Name"
            tabItem: _summaryField
            text: _private.breakElement.breakSubtitle
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 5

            onTextChanged: _private.breakElement.breakSubtitle = text
        }

        FlickableTextArea {
            id: _summaryField

            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: _vscrollBar

            adjustTextWidthBasedOnScrollBar: false
            backTabItem: _headingField
            placeholderText: _private.breakKind + " Summary ..."
            text: _private.breakElement.breakSummary

            background: Rectangle {
                color: Runtime.colors.primary.windowColor
                opacity: 0.15
            }

            onTextChanged: _private.breakElement.breakSummary = text
        }
    }

    VclScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: _attachmentsView.top

        flickable: _summaryField.lod
        orientation: Qt.Vertical
    }

    AttachmentsView {
        id: _attachmentsView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        attachments: _private.breakElement ? _private.breakElement.attachments : null
    }

    AttachmentsDropArea {
        id: _dropArea

        anchors.fill: parent

        allowMultiple: true
        target: _private.breakElement ? _private.breakElement.attachments : null
    }

    VclLabel {
        anchors.centerIn: parent

        width: parent.width * 0.6

        horizontalAlignment: Text.AlignHCenter
        text: "Create " + _private.breakKind.toLowerCase() + " break in the screenplay to capture a summary for it."
        visible: _private.breakElement === null
        wrapMode: Text.WordWrap

        font.pointSize: Runtime.idealFontMetrics.font.pointSize
    }

    QtObject {
        id: _private

        property ScreenplayElement breakElement: root.pageData.notebookItemObject
        property string breakKind: root.pageData.notebookItemType === NotebookModel.EpisodeBreakType ? "Episode" : "Act"
    }
}
