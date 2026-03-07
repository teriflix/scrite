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
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

Popup {
    id: root

    property font editorFont: Runtime.idealFontMetrics.font

    property LanguageTransliterator transliterator

    property rect textRect: transliterator ? Scrite.window.contentItem.mapFromItem(transliterator.editor, transliterator.textRect) : Qt.rgba(0,0,0,0)

    x: textRect.x + 10
    y: textRect.y + textRect.height + 5
    width: _content.implicitWidth + leftPadding + rightPadding
    height: _content.implicitHeight + topPadding + bottomPadding

    parent: Scrite.window.contentItem
    visible: transliterator ? transliterator.suggestions.length > 0 : false
    closePolicy: Popup.NoAutoClose

    contentItem: ColumnLayout {
        id: _content

        ListView {
            id: _suggestionsList

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            Layout.fillWidth: true
            Layout.preferredWidth: GMath.horizontalAdvance(transliterator.suggestions, root.editorFont) + 30
            Layout.preferredHeight: count ? itemAtIndex(0).height * Math.min(count, 5) : 30

            clip: contentHeight > height
            model: transliterator.suggestions
            currentIndex: transliterator.currentSuggestionIndex

            delegate: Label {
                required property string modelData

                width: _suggestionsList.width

                text: modelData
                font: root.editorFont
                padding: 8
            }

            highlight: Rectangle {
                color: "lightsteelblue"
                visible: _suggestionsList.count > 1
            }
            highlightMoveDuration: 0
            highlightResizeDuration: 0
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
            id: _needHelp

            Layout.fillWidth: true
            Layout.minimumWidth: contentWidth

            text: "Need help?"
            elide: Text.ElideRight
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            horizontalAlignment: Text.AlignHCenter

            onClicked: Qt.openUrlExternally("https://www.scrite.io/docs/userguide/languages/")
        }
    }
}
