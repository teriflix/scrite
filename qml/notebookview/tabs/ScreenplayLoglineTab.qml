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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/structureview"

Item {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    readonly property Screenplay screenplay: Scrite.document.screenplay

    ColumnLayout {
        anchors.centerIn: parent

        width: Math.max(root.minTextAreaSize, Math.min(parent.width-20, root.maxTextAreaSize))

        VclLabel {
            Layout.fillWidth: true

            text: "A logline should swiftly convey what a screenplay is about, including the main character, central conflict, setup and antagonist."
            wrapMode: Text.WordWrap
        }

        Link {
            Layout.fillWidth: true

            elide: Text.ElideMiddle
            text: "https://online.pointpark.edu/screenwriting/loglines/"

            onClicked: Qt.openUrlExternally(text)
        }

        FlickableTextArea {
            id: _logline

            Component.onCompleted: syntaxHighlighter.addDelegate(_textLimitHighlighter)

            ScrollBar.vertical: VclScrollBar { }

            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(Runtime.idealFontMetrics.lineSpacing*10, contentHeight+10)

            adjustTextWidthBasedOnScrollBar: true
            placeholderText: "Type your logline here."
            readOnly: Scrite.document.readOnly
            text: root.screenplay.logline
            undoRedoEnabled: true

            font.family: Scrite.document.displayFormat.defaultFont2.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

            background: Rectangle {
                color: Runtime.colors.primary.windowColor
                opacity: 0.15
            }

            onTextChanged: root.screenplay.logline = text
        }

        VclLabel {
            Layout.fillWidth: true

            color: _textLimiter.limitReached ? "darkred" : Runtime.colors.primary.a700.background
            text: (_textLimiter.limitReached ? "WARNING: " : "") + "Words: " + _textLimiter.wordCount + "/" + _textLimiter.maxWordCount +
                  ", Letters: " + _textLimiter.letterCount + "/" + _textLimiter.maxLetterCount
            wrapMode: Text.WordWrap
        }
    }

   TextLimiterSyntaxHighlighterDelegate {
        id: _textLimitHighlighter

        textLimiter: TextLimiter {
            id: _textLimiter

            countMode: TextLimiter.CountInText
            maxLetterCount: 240
            maxWordCount: 50
        }
    }
}
