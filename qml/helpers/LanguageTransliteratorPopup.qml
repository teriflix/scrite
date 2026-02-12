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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Popup {
    id: root

    property font editorFont: Runtime.idealFontMetrics.font

    property LanguageTransliterator transliterator

    property rect textRect: transliterator ? Scrite.window.contentItem.mapFromItem(transliterator.editor, transliterator.textRect) : Qt.rgba(0,0,0,0)

    x: textRect.x + 10
    y: textRect.y + textRect.height + 5

    parent: Scrite.window.contentItem
    visible: transliterator ? transliterator.commitString !== "" : false
    closePolicy: Popup.NoAutoClose

    contentItem: ColumnLayout {
        Label {
            Layout.fillWidth: true

            text: root.transliterator ? root.transliterator.commitString : ""
            font: root.editorFont
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 10

            Rectangle {
                anchors.bottom: parent.bottom

                width: parent.width
                color: Runtime.colors.primary.borderColor
            }
        }

        Link {
            text: "Need help?"
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            horizontalAlignment: Text.AlignHCenter

            onClicked: Qt.openUrlExternally("https://www.scrite.io/typing-in-multiple-languages/")
        }
    }
}
