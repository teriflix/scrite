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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root
    height: layout.height + 2*layout.margin

    GridLayout {
        id: layout

        readonly property real margin: 10

        width: parent.width-margin
        y: margin

        rowSpacing: 10
        columnSpacing: 20
        columns: width > 700 ? 2 : 1

        Repeater {
            model: GenericArrayModel {
                array: Scrite.app.enumerationModelForType("TransliterationEngine", "Language")
                objectMembers: ["key", "value"]
            }

            RowLayout {
                required property int index
                required property string key
                required property int value

                Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.spacing)/layout.columns

                spacing: 10

                VclText {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: parent.width * 0.25

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    font.bold: tisSourceCombo.down

                    text: key + ": "
                }

                VclComboBox {
                    id: tisSourceCombo

                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true

                    property var sources: []
                    model: sources
                    textRole: "title"

                    onActivated: {
                        var item = sources[currentIndex]
                        Scrite.app.transliterationEngine.setTextInputSourceIdForLanguage(modelData.value, item.id)
                    }

                    Component.onCompleted: {
                        var tisSources = Scrite.app.textInputManager.sourcesForLanguageJson(value)
                        var tisSourceId = Scrite.app.transliterationEngine.textInputSourceIdForLanguage(value)
                        tisSources.unshift({"id": "", "title": "Default (Inbuilt Scrite Transliterator)"})
                        enabled = tisSources.length > 1
                        sources = tisSources
                        for(var i=0; i<sources.length; i++) {
                            if(sources[i].id === tisSourceId) {
                                currentIndex = i
                                break
                            }
                        }
                    }
                }
            }
        }
    }
}
