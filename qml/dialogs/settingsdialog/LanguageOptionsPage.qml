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

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10

        spacing: 10

        ColumnLayout {
            Layout.fillHeight: true
            Layout.maximumWidth: root.width * 0.3
            Layout.preferredWidth: root.width * 0.3

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true

                color: Runtime.colors.primary.c100.background
                border.color: Runtime.colors.primary.borderColor
                border.width: 1

                ListView {
                    id: _listView

                    property real delegateWidth: contentHeight > height ? width-20 : width

                    ScrollBar.vertical: VclScrollBar { }

                    anchors.fill: parent
                    anchors.margins: 1

                    clip: true
                    model: Runtime.language.supported
                    currentIndex: 0
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        required property int index
                        required property var language

                        width: _listView.delegateWidth
                        height: _delegateLayout.height

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                _listView.currentIndex = index
                                parent.forceActiveFocus()
                            }
                        }

                        RowLayout {
                            id: _delegateLayout

                            width: parent.width
                            spacing: 5

                            Image {
                                Layout.alignment: Qt.AlignVCenter
                                Layout.leftMargin: parent.spacing
                                Layout.preferredWidth: Runtime.idealFontMetrics.averageCharacterWidth * 4
                                Layout.preferredHeight: _delegateLabel.height-6

                                fillMode: Image.PreserveAspectFit
                                mipmap: true
                                source: language.iconSource
                            }

                            VclLabel {
                                id: _delegateLabel

                                Layout.fillWidth: true

                                text: language.name
                                color: _listView.currentIndex === index ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.c100.text
                                elide: Text.ElideMiddle
                                padding: 10
                            }

                            VclToolButton {
                                suggestedWidth: _delegateLabel.height
                                suggestedHeight: _delegateLabel.height

                                visible: _listView.count > 1 && language.code !== QtLocale.English
                                opacity: hovered ? 1 : 0.5
                                hoverEnabled: true

                                icon.source: "qrc:/icons/action/delete.png"

                                onClicked: _private.deleteLanguage(language)
                            }
                        }
                    }

                    highlight: Rectangle {
                        color: Runtime.colors.primary.highlight.background
                    }

                    footer: ColumnLayout {
                        width: _listView.delegateWidth

                        spacing: 5

                        Rectangle {
                            Layout.topMargin: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1

                            color: Runtime.colors.primary.borderColor
                        }

                        VclButton {
                            Layout.margins: 10
                            Layout.fillWidth: true

                            text: "Add Language"
                            flat: true
                            icon.source: "qrc:/icons/content/add_box.png"

                            onClicked: _newLanguageDialog.open()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c100.background
            enabled: _private.language !== undefined
            opacity: enabled ? 1 : 0.5

            border.color: Runtime.colors.primary.borderColor
            border.width: 1

            Item {
                anchors.fill: parent
                anchors.margins: 20

                ColumnLayout {
                    width: parent.width

                    spacing: 30

                    VclLabel {
                        Layout.alignment: Qt.AlignHCenter

                        text: _private.language !== undefined ? _private.language.nativeName : "- NA -"
                        font.family: _private.language !== undefined ? _private.language.font().family : Runtime.idealFontMetrics.font.family
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 5
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: parent.width * 0.75

                        spacing: parent.spacing/2

                        VclGroupBox {
                            Layout.fillWidth: true

                            title: "Keyboard Shortcut"

                            ShortcutField {
                                Layout.fillWidth: true

                                opacity: enabled ? 1 : 0.5
                                description: _private.language !== undefined ? "Shortcut for switching to <b>" + _private.language.nativeName + "</b> language" : "Shortcut"
                                enabled: _private.language !== undefined ? (!DefaultTransliteration.supportsLanguageCode(_private.language.code) && _private.language.code !== QtLocale.English) : false
                                portableShortcut: _private.language !== undefined ? _private.language.shortcut() : ""

                                onShortcutEdited: (newShortcut) => {
                                                      if(ActionHub.hasShortcutConflict(newShortcut)) {
                                                          return
                                                      }

                                                      Runtime.language.supported.assignLanguageShortcut(_private.language.code, newShortcut)
                                                  }
                            }
                        }

                        VclGroupBox {
                            Layout.fillWidth: true

                            title: "Font"

                            ColumnLayout {
                                width: parent.width

                                VclComboBox {
                                    property var languageFonts: _private.language !== undefined ? _private.language.fontFamilies() : []

                                    Layout.fillWidth: true

                                    model: languageFonts
                                    padding: 0
                                    currentIndex: _private.language !== undefined ? languageFonts.indexOf(_private.language.font().family) : -1

                                    onActivated: (index) => {
                                                     if(_private.language)
                                                        Runtime.language.supported.assignLanguageFontFamily(_private.language.code, languageFonts[index])
                                                 }
                                }

                                VclText {
                                    Layout.fillWidth: true

                                    text: _private.language !== undefined ? "<b>NOTE:</b> Languages with " + _private.language.charScriptName() + " script, will share the same font." : ""
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        VclGroupBox {
                            Layout.fillWidth: true

                            title: "Input Method"

                            ColumnLayout {
                                width: parent.width

                                VclCheckBox {
                                    text: "Auto Select"
                                    checked: _private.language !== undefined ? _private.language.preferredTransliterationOptionId === "" : ""

                                    onToggled: {
                                        if(_private.language) {
                                            if(checked) {
                                                Runtime.language.supported.resetLanguageTranslator(_private.language.code);
                                            } else {
                                                Runtime.language.supported.useLanguageTransliteratorId(_private.language.code, _private.language.preferredTransliterationOption().id)
                                            }
                                        }
                                    }
                                }

                                VclComboBox {
                                    Layout.fillWidth: true

                                    model: _private.transliterationOptionsModel
                                    enabled: _private.language !== undefined && _private.language.preferredTransliterationOptionId !== ""
                                    textRole: "display"
                                    valueRole: "id"
                                    currentIndex: _private.language !== undefined ? indexOfValue(_private.language.preferredTransliterationOption().id) : -1
                                    onActivated: (index) => {
                                                     if(_private.language) {
                                                        Runtime.language.supported.useLanguageTransliteratorId( _private.language.code, _private.transliterationOptionsModel.get(index).id )
                                                     }
                                                 }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    VclDialog {
        id: _newLanguageDialog

        width: root.width * 0.8
        height: 300

        title: "Add New Language"

        content: Item {
            id: _newLanguageDialogContent

            readonly property var availableLanguages: _private.availableLanguages()

            function addLanguage(languageName) {
                if(languageName === "") {
                    _newLanguageDialog.close()
                    return
                }

                const index = availableLanguages.names.map(item => item.toLowerCase()).indexOf(languageName.toLowerCase());
                if(index < 0) {
                    MessageBox.information("Add Language Error",
                                           "No language by name \"" + languageName + "\" was found.")
                    return
                }

                const languageCode = availableLanguages.codes[index]
                const row = Runtime.language.supported.addLanguage(languageCode)
                if(row >= 0) {
                    _listView.currentIndex = row
                    _newLanguageDialog.close()
                } else {
                    MessageBox.information("Add Language Error",
                                           "No language by name \"" + languageName + "\" could be added.")
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                VclLabel {
                    Layout.fillWidth: true

                    text: "You can add Hindi, Marathi, Tamil, French, Vietnamese and more..."
                    wrapMode: Text.WordWrap
                }

                VclTextField {
                    id: _languageNameField

                    Layout.fillWidth: true

                    placeholderText: "Language name"
                    completionStrings: _newLanguageDialogContent.availableLanguages.names

                    onEditingComplete: _newLanguageDialogContent.addLanguage(text)

                    Component.onCompleted: Qt.callLater(forceActiveFocus)
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Add"
                    enabled: _newLanguageDialogContent.text !== ""

                    onClicked: _newLanguageDialogContent.addLanguage(_languageNameField.text)
                }

                VclLabel {
                    Layout.fillWidth: true

                    text: "NOTE: RTL languages are not yet supported."
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property ActionsModel actionsModel: ActionsModel { }

        property int previouslyActiveLanguage: -1

        property var language: _listView.currentItem ? _listView.currentItem.language : undefined

        property ListModel transliterationOptionsModel: ListModel { }

        Component.onCompleted: {
            previouslyActiveLanguage = Runtime.language.activeCode
            Runtime.language.setActiveCode(QtLocale.English)
        }

        Component.onDestruction: {
            Runtime.language.setActiveCode(previouslyActiveLanguage)
        }

        function populateTransliterationOptionsModel() {
            transliterationOptionsModel.clear()
            if(language === undefined)
                return

            const options = language.transliterationOptions()
            for(let i=0; i<options.length; i++) {
                const option = options[i]
                const record = {
                    "id": option.id,
                    "name": option.name,
                    "inApp": option.inApp,
                    "display": option.id + " (" + option.name + ")",
                    "transliterator": option.transliterator.name
                }
                transliterationOptionsModel.append(record)
            }
        }

        function availableLanguages() {
            let codes = []
            let names = []

            const nrLanguages = Runtime.language.available.count
            for(let i=0; i<nrLanguages; i++) {
                const language = Runtime.language.available.languageAt(i)
                codes.push(language.code)
                names.push(language.name)
            }

            return { "codes": codes, "names": names }
        }

        function deleteLanguage(language) {
            const row = Runtime.language.supported.removeLanguage(language.code)
            if(row < 0)
                return

            _listView.currentIndex = -1
            _listView.currentIndex = Math.max(0, Math.min(row, _listView.count-1))
        }

        onLanguageChanged: populateTransliterationOptionsModel()
    }
}
