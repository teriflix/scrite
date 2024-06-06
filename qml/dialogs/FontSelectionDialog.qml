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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0
import io.scrite.models 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    readonly property var standardPreviewText: [
        /* English */ "The bird flies in the sky.",
        /* Bengali */ "পাখিটি আকাশে উড়ে যায়।", // The bird flies in the sky.
        /* Gujarati */ "પંખી આકાશમાં ઊડે છે.", // The bird flies in the sky.
        /* Hindi */ "पक्षी आसमान में उड़ता है।", // The bird flies in the sky.
        /* Kannada */ "ಹಕ್ಕಿ ಆಕಾಶದಲ್ಲಿ ಹಾರುತ್ತದೆ.", // The bird flies in the sky.
        /* Malayalam */ "പക്ഷി ആകാശത്ത് പറക്കുന്നു.", // The bird flies in the sky.
        /* Marathi */ "पक्षी आकाशात उडतो.", // The bird flies in the sky.
        /* Oriya */ "ପକ୍ଷୀ ଆକାଶରେ ଉଡେ।", // The bird flies in the sky.
        /* Punjabi */ "ਪੰਛੀ ਅਸਮਾਨ ਵਿੱਚ ਉੱਡਦਾ ਹੈ।", // The bird flies in the sky.
        /* Sanskrit */ "पक्षी आकाशे उड़ति।", // The bird flies in the sky.
        /* Tamil */ "பறவை ஆகாயத்தில் பறக்கின்றது.", // The bird flies in the sky.
        /* Telugu */ "పక్షి ఆకాశంలో ఎగురుతుంది." // The bird flies in the sky.
    ];

    function launch(fontSelectedCallback) {
        const dlg = doLaunch()
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    function launchWithTitle(title, fontSelectedCallback) {
        const dlg = doLaunch({"title": title})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    function launchForLanguage(language, fontSelectedCallback) {
        const dlg = doLaunch({"language": language})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    function launchWithTitleForLanguage(title, language, fontSelectedCallback) {
        const dlg = doLaunch({"title": title, "language": language})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    name: "FontSelectionDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        property int language: TransliterationEngine.English
        property string previewText: root.standardPreviewText[language]

        signal fontSelected(string fontFamily)

        title: "Select a font"
        width: Math.min(750, Scrite.window.width*0.8)
        height: Math.min(Scrite.window.height*0.9, 550)

        content: Item {
            EventFilter.target: Scrite.app
            EventFilter.events: [EventFilter.KeyPress]
            EventFilter.onFilter: (watched, event, result) => {
                result.acceptEvent = true
                result.filter = true

                if(event.key === Qt.Key_Up)
                    fontList.currentIndex = Math.max(0, fontList.currentIndex-1)
                else if(event.key === Qt.Key_Down)
                    fontList.currentIndex = Math.min(fontList.count-1, fontList.currentIndex+1)
                else if(event.key === Qt.Key_PageUp)
                    fontList.currentIndex = Math.max(0, fontList.currentIndex-10)
                else if(event.key === Qt.Key_PageDown)
                    fontList.currentIndex = Math.min(fontList.count-1, fontList.currentIndex+10)
                else {
                    result.acceptEvent = false
                    result.filter = false
                }
            }

            GenericArrayModel {
                id: fontFamiliesModel
                array: {
                    const allFonts = Scrite.app.fontFamiliesFor(TransliterationEngine.English)
                    const languageSpecificFonts = dialog.language === TransliterationEngine.English ? [] : Scrite.app.fontFamiliesFor(dialog.language)

                    let ret = []
                    languageSpecificFonts.forEach( (font) => {
                                                    ret.push( {"category": "Suggested Fonts", "family": font} )
                                                  })

                    const cat = dialog.language === TransliterationEngine.English ? "Available Fonts" : "Other Fonts"
                    allFonts.forEach( (font) => {
                                                    if(languageSpecificFonts.indexOf(font) >= 0)
                                                        return

                                                    ret.push( {"category": cat, "family": font} )
                                                  })

                    return ret
                }
                objectMembers: ["category", "family"]
            }

            GenericArraySortFilterProxyModel {
                id: fontFamiliesFilterModel
                arrayModel: fontFamiliesModel
                onFilterRow: (source_row, result) => {
                    if(fontFilter.length == 0) {
                        result.value = true
                    } else {
                        let filter = fontFilter.text.toLowerCase()

                        let familyName = "" + fontFamiliesModel.get(source_row).family
                        familyName = familyName.toLowerCase()

                        result.value = familyName.indexOf(filter) == 0
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 10

                TextField {
                    id: fontFilter

                    Layout.fillWidth: true

                    selectByMouse: true
                    placeholderText: "Search for a font"

                    onTextEdited: {
                        fontList.currentIndex = -1
                        fontFamiliesFilterModel.refilter()
                        Qt.callLater( () => { fontList.currentIndex = 0 } )
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    Component {
                        id: fontListSectionDelegate

                        Rectangle {
                            required property string section

                            width: fontList.width - (fontList.ScrollBar.vertical.needed ? 17 : 0)
                            height: Runtime.idealFontMetrics.lineSpacing+15
                            color: Runtime.colors.accent.highlight.background

                            VclText {
                                id: sectionLabel

                                anchors.centerIn: parent

                                text: section
                                color: Runtime.colors.accent.highlight.text
                                width: parent.width
                                elide: Text.ElideRight
                                padding: 3
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    ListView {
                        id: fontList

                        anchors.fill: parent
                        anchors.margins: 1

                        ScrollBar.vertical: VclScrollBar { }

                        clip: true
                        model: fontFamiliesFilterModel
                        spacing: 5
                        currentIndex: -1
                        keyNavigationEnabled: false

                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0
                        highlightFollowsCurrentItem: true

                        section.property: "category"
                        section.criteria: ViewSection.FullString
                        section.delegate: dialog.language === TransliterationEngine.English ? null : fontListSectionDelegate

                        delegate: Item {
                            required property int index
                            required property string family

                            width: fontList.width - (fontList.ScrollBar.vertical.needed ? 17 : 0)
                            height: delegateLayout.height

                            RowLayout {
                                id: delegateLayout

                                width: parent.width-10
                                anchors.horizontalCenter: parent.horizontalCenter

                                VclLabel {
                                    Layout.preferredWidth: parent.width * 0.5

                                    text: family
                                    elide: Text.ElideRight
                                    padding: 3
                                }

                                VclLabel {
                                    Layout.fillWidth: true

                                    text: dialog.previewText
                                    elide: Text.ElideRight
                                    padding: 3
                                    wrapMode: Text.NoWrap
                                    font.family: family
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: fontList.currentIndex = index
                            }
                        }
                    }
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Select"
                    enabled: fontList.currentIndex >= 0

                    onClicked: {
                        _private.fontWasSelected = true
                        dialog.fontSelected(fontList.currentItem.family)
                        Qt.callLater(dialog.close)
                    }
                }
            }
        }

        onClosed: {
            if(!_private.fontWasSelected)
                fontSelected("")
        }

        QtObject {
            id: _private

            property bool fontWasSelected: false
        }
    }
}
