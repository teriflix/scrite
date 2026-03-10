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

import QtQuick
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

Rectangle {
    id: root

    required property var language

    width: _layout.width + 20
    height: _layout.height + 20

    color: Runtime.colors.primary.c10.background
    border { width: 1; color: Runtime.colors.primary.borderColor }

    Row {
        id: _layout

        anchors.centerIn: parent

        spacing: 10

        Repeater {
            model: _private.mappingModels

            delegate: Column {
                id: _delegate

                required property int index
                required property var modelData

                spacing: 0

                Rectangle {
                    width: _symbolGrid.width
                    height: 30

                    color: Runtime.colors.primary.c600.background

                    VclLabel {
                        anchors.centerIn: parent

                        text: _delegate.modelData.heading
                        color: Runtime.colors.primary.c600.text
                        padding: 8

                        font.pointSize: _normalFontMetrics.font.pointSize
                    }
                }

                Grid {
                    id: _symbolGrid

                    flow: Grid.TopToBottom
                    rows: 10
                    columns: Math.ceil(_delegate.modelData.array.length/rows)
                    columnSpacing: 10

                    Repeater {
                        model: _delegate.modelData.array

                        delegate: Row {
                            id: _delegate2

                            required property int index
                            required property var modelData

                            VclLabel {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.verticalCenterOffset: (_normalFontMetrics.height-_languageFontMetrics.height)*0.3

                                width: _private.textCellWidth

                                text: _delegate2.modelData.latin
                                font: _normalFontMetrics.font
                                padding: 8
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignRight
                            }

                            VclLabel {
                                anchors.verticalCenter: parent.verticalCenter

                                width: _private.textCellWidth

                                text: _delegate2.modelData.unicode
                                font: _languageFontMetrics.font
                                padding: 8
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                            }
                        }
                    }
                }
            }
        }
    }


    FontMetrics {
        id: _languageFontMetrics

        font.family: root.language.font()
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
    }

    FontMetrics {
        id: _normalFontMetrics

        font.pointSize: Runtime.idealFontMetrics.font.pointSize
    }

    QtObject {
        id: _private

        readonly property real textCellWidth: 50

        readonly property var alphabetMappings: DefaultTransliteration.alphabetMappingsFor(root.language.code)

        readonly property var mappingModels: [
            { "heading": "Vowels", "array": alphabetMappings.vowels },
            { "heading": "Consonants", "array": alphabetMappings.consonants },
            { "heading": "Digits", "array": alphabetMappings.digits },
            { "heading": "Symbols", "array": alphabetMappings.symbols }
        ]
    }
}
