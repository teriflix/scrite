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
import QtQuick.Layouts

import Scrite.App

import "../../globals"
import "../../helpers"
import "../../controls"
import "../../dialogs"

Item {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    required property ScreenplayElement breakElement
    required property string breakKind // "Episode" or "Act"

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

        SctLabel {
            Layout.fillWidth: true

            text: root.breakElement.breakTitle

            font.pointSize: Runtime.notebookFontPointSize + 3
        }

        SctTextField {
            id: _headingField

            Layout.fillWidth: true

            label: ""
            placeholderText: root.breakKind + " Name"
            tabItem: _summaryField
            text: root.breakElement.breakSubtitle
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font.pointSize: Runtime.notebookFontPointSize + 5

            onTextEdited: root.breakElement.breakSubtitle = text
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c200.background

            FlickableTextArea {
                id: _summaryField

                anchors.fill: parent
                anchors.margins: 4

                ScrollBar.vertical: _vscrollBar

                adjustTextWidthBasedOnScrollBar: false
                backTabItem: _headingField
                background.visible: false
                markdownSyntaxEnabled: true
                placeholderText: root.breakKind + " Summary ..."
                text: root.breakElement.breakSummary
                font.pointSize: Runtime.notebookFontPointSize

                onTextEdited: root.breakElement.breakSummary = text
            }
        }
    }

    SctScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: _attachmentsView.top

        flickable: _summaryField
        orientation: Qt.Vertical
    }

    AttachmentsView {
        id: _attachmentsView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        attachments: root.breakElement.attachments
    }

    AttachmentsDropArea {
        anchors.fill: parent

        allowMultiple: true
        target: root.breakElement.attachments
    }
}
