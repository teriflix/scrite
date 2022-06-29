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

import QtQuick 2.15
import QtQuick.Controls 2.15
import io.scrite.components 1.0

Rectangle {
    property var mappings: Scrite.app.transliterationEngine.alphabetMappings
    property font languageFont: Scrite.app.transliterationEngine.font

    property var mappingModels: [
        { "heading": "Vowels", "array": mappings.vowels },
        { "heading": "Consonants", "array": mappings.consonants },
        { "heading": "Digits", "array": mappings.digits },
        { "heading": "Symbols", "array": mappings.symbols }
    ]
    width: layout.width + 20
    height: layout.height + 20
    color: primaryColors.c10.background
    border { width: 1; color: primaryColors.borderColor }

    readonly property real textCellWidth: 50

    FontMetrics {
        id: languageFontMetrics
        font.family: languageFont.family
        font.pointSize: Scrite.app.idealFontPointSize
    }

    FontMetrics {
        id: normalFontMetrics
        font.pointSize: Scrite.app.idealFontPointSize
    }

    Row {
        id: layout
        spacing: 10
        anchors.centerIn: parent

        Repeater {
            model: mappingModels

            Column {
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 30
                    color: primaryColors.c600.background

                    Text {
                        text: modelData.heading
                        padding: 8
                        font.pointSize: normalFontMetrics.font.pointSize
                        anchors.centerIn: parent
                        color: primaryColors.c600.text
                    }
                }

                Grid {
                    rows: 10
                    columns: Math.ceil(modelData.array.length/rows)
                    flow: Grid.TopToBottom
                    columnSpacing: 10

                    Repeater {
                        model: modelData.array

                        Row {
                            Text {
                                width: textCellWidth
                                padding: 8
                                font: normalFontMetrics.font
                                text: modelData.latin
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.verticalCenterOffset: (normalFontMetrics.height-languageFontMetrics.height)*0.3
                            }
                            Text {
                                width: textCellWidth
                                padding: 8
                                font: languageFontMetrics.font
                                text: modelData.unicode
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }
}
