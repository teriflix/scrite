/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Column {
    id: formField
    spacing: 10

    property string questionKey: questionNumber
    property alias questionNumber: questionNumberText.text
    property alias question: questionText.text
    property alias answer: answerText.text
    property alias placeholderText: answerText.placeholderText

    Row {
        width: parent.width
        spacing: 10

        Label {
            id: questionNumberText
            font.bold: true
            horizontalAlignment: Text.AlignRight
            width: idealAppFontMetrics.averageCharacterWidth * 6
            anchors.top: parent.top
        }

        Label {
            id: questionText
            font.bold: true
            width: parent.width - questionNumberText.width - parent.spacing
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.top: parent.top
        }
    }

    Rectangle {
        width: questionText.width
        anchors.right: parent.right
        color: app.translucent(primaryColors.c100.background, 0.5)
        border.width: 1
        border.color: primaryColors.borderColor
        height: Math.max(200, Math.min(answerText.contentHeight, 500))

        FlickableTextArea {
            id: answerText
            anchors.fill: parent
        }
    }
}
