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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"

Item {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    required property Scene scene

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: {
        EventFilter.forwardEventTo(_textArea)
        result.filter = true
        result.accepted = true
    }

    FlickableTextArea {
        id: _textArea

        ScrollBar.vertical: _scrollBar

        anchors.centerIn: parent

        width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-20
        height: parent.height - 20

        adjustTextWidthBasedOnScrollBar: false
        placeholderText: "Scene Comments"
        readOnly: Scrite.document.readOnly
        text: root.scene.comments
        undoRedoEnabled: true

        background: Rectangle {
            color: Runtime.colors.primary.windowColor
            opacity: 0.25
            border.width: 1
            border.color: Runtime.colors.primary.borderColor
        }

        onTextEdited: root.scene.comments = text
    }

    VclScrollBar {
        id: _scrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        orientation: Qt.Vertical
        flickable: _textArea
    }
}
