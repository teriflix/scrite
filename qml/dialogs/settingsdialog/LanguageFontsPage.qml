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

    ColumnLayout {
        id: layout

        readonly property real margin: 10

        width: parent.width-2*margin
        y: 10
        anchors.horizontalCenter: parent.horizontalCenter

        spacing: 10

        GroupBox {
            Layout.fillHeight: true

            label: VclText {
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: "Language Font Options"
            }

            ColumnLayout {
                width: parent.width
                spacing: 10

                Repeater {
                    model: GenericArrayModel {
                        array: Scrite.app.enumerationModelForType("TransliterationEngine", "Language")
                        objectMembers: ["key", "value"]
                    }

                    RowLayout {
                        required property int index
                        required property string key
                        required property int value

                        spacing: 10
                        Layout.fillWidth: true

                        VclText {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: 175

                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            font.bold: fontCombo.down

                            horizontalAlignment: Text.AlignRight

                            text: key
                        }

                        VclComboBox {
                            id: fontCombo
                            Layout.preferredWidth: 400

                            property var fontFamilies: Scrite.app.transliterationEngine.availableLanguageFontFamilies(value)
                            model: fontFamilies.families

                            currentIndex: fontFamilies.preferredFamilyIndex

                            onActivated: {
                                var family = fontFamilies.families[index]
                                Scrite.app.transliterationEngine.setPreferredFontFamilyForLanguage(value, family)
                                previewText.font.family = family
                            }
                        }

                        VclText {
                            id: previewText

                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: 150

                            font.family: Scrite.app.transliterationEngine.preferredFontFamilyForLanguage(value)
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            font.bold: fontCombo.down

                            text: _private.languagePreviewString[value]
                        }
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: "Custom Font Usage Options"
            }

            ColumnLayout {
                width: parent.width
                spacing: 10

                VclText {
                    Layout.fillWidth: true

                    wrapMode: Text.WordWrap
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    text: "Please choose where you want custom fonts configured here to be used."
                }

                VclRadioButton {
                    text: "PDF & HTML Only"
                    checked: !Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                    onToggled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts = !checked
                }

                VclRadioButton {
                    text: "Display, PDF & HTML"
                    checked: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                    onToggled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts = checked
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property var languagePreviewString: [ "Greetings", "বাংলা", "ગુજરાતી", "हिन्दी", "ಕನ್ನಡ", "മലയാളം", "मराठी", "ଓଡିଆ", "ਪੰਜਾਬੀ", "संस्कृत", "தமிழ்", "తెలుగు" ]
    }
}
