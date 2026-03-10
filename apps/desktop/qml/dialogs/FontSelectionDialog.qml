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

pragma Singleton

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

DialogLauncher {
    id: root

    function launch(fontSelectedCallback, initialFontFamily) {
        const dlg = doLaunch({"initialFontFamily": ""+initialFontFamily})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    function launchWithTitle(title, fontSelectedCallback, initialFontFamily) {
        const dlg = doLaunch({"title": title, "initialFontFamily": ""+initialFontFamily})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    function launchForLanguage(language, fontSelectedCallback, initialFontFamily) {
        let languageCode = QtLocale.English
        if(typeof language === "number") {
            languageCode = language
        } else {
            languageCode = language.code
        }

        const dlg = doLaunch({"languageCode": languageCode, "initialFontFamily": ""+initialFontFamily})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    function launchWithTitleForLanguage(title, language, fontSelectedCallback, initialFontFamily) {
        let languageCode = QtLocale.English
        if(typeof language === "number") {
            languageCode = language
        } else {
            languageCode = language.code
        }

        const dlg = doLaunch({"title": title, "languageCode": languageCode, "initialFontFamily": ""+initialFontFamily})
        if(dlg)
            dlg.fontSelected.connect(fontSelectedCallback)
        return dlg
    }

    name: "FontSelectionDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        property var language: Runtime.language.available.findLanguage(languageCode)
        property int languageCode: QtLocale.English
        property bool languageUsesLatinScript: language.charScript() === QtChar.Script_Latin
        property string previewText: language.nativeName
        property string initialFontFamily

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
                    _fontList.currentIndex = Math.max(0, _fontList.currentIndex-1)
                else if(event.key === Qt.Key_Down)
                    _fontList.currentIndex = Math.min(_fontList.count-1, _fontList.currentIndex+1)
                else if(event.key === Qt.Key_PageUp)
                    _fontList.currentIndex = Math.max(0, _fontList.currentIndex-10)
                else if(event.key === Qt.Key_PageDown)
                    _fontList.currentIndex = Math.min(_fontList.count-1, _fontList.currentIndex+10)
                else {
                    result.acceptEvent = false
                    result.filter = false
                }
            }

            GenericArrayModel {
                id: _fontFamiliesModel

                array: {
                    const allFonts = LanguageEngine.scriptFontFamilies(QtChar.Script_Latin)
                    const languageSpecificFonts = _dialog.languageUsesLatinScript ? [] : _dialog.language.fontFamilies()

                    let ret = []
                    languageSpecificFonts.forEach( (font) => {
                                                    ret.push( {"category": "Suggested Fonts", "family": font} )
                                                  })

                    const cat = _dialog.languageUsesLatinScript ? "Available Fonts" : "Other Fonts"
                    allFonts.forEach( (font) => {
                                                    if(languageSpecificFonts.indexOf(font) >= 0)
                                                        return

                                                    ret.push( {"category": cat, "family": font} )
                                                  })

                    initialIndex = -1
                    for(let i=0; i<ret.length; i++) {
                        if(ret[i].family === _dialog.initialFontFamily) {
                            initialIndex = i
                            break
                        }
                    }

                    return ret
                }
                objectMembers: ["category", "family"]

                property int initialIndex: -1
            }

            GenericArraySortFilterProxyModel {
                id: _fontFamiliesFilterModel
                arrayModel: _fontFamiliesModel
                onFilterRow: (source_row, result) => {
                    if(_fontFilter.length == 0) {
                        result.value = true
                    } else {
                        let filter = _fontFilter.text.toLowerCase()

                        let familyName = "" + _fontFamiliesModel.get(source_row).family
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
                    id: _fontFilter

                    Layout.fillWidth: true

                    DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

                    selectByMouse: true
                    placeholderText: "Search for a font"

                    onTextEdited: {
                        _fontList.currentIndex = -1
                        _fontFamiliesFilterModel.refilter()
                        Qt.callLater( () => { _fontList.currentIndex = 0 } )
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    Component {
                        id: _fontListSectionDelegate

                        Rectangle {
                            required property string section

                            width: _fontList.width - (_fontList.ScrollBar.vertical.needed ? 17 : 0)
                            height: Runtime.idealFontMetrics.lineSpacing+15
                            color: Runtime.colors.accent.highlight.background

                            VclText {
                                id: _sectionLabel

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
                        id: _fontList

                        anchors.fill: parent
                        anchors.margins: 1

                        ScrollBar.vertical: VclScrollBar { }

                        clip: true
                        model: _fontFamiliesFilterModel
                        spacing: 5
                        currentIndex: -1
                        keyNavigationEnabled: false

                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0
                        highlightFollowsCurrentItem: true

                        highlightRangeMode: ListView.ApplyRange
                        preferredHighlightEnd: height * 0.75
                        preferredHighlightBegin: height * 0.25

                        section.property: "category"
                        section.criteria: ViewSection.FullString
                        section.delegate: _dialog.languageUsesLatinScript ? null : _fontListSectionDelegate

                        delegate: Item {
                            required property int index
                            required property string family

                            width: _fontList.width - (_fontList.ScrollBar.vertical.needed ? 17 : 0)
                            height: _delegateLayout.height

                            RowLayout {
                                id: _delegateLayout

                                width: parent.width-10
                                anchors.horizontalCenter: parent.horizontalCenter

                                VclLabel {
                                    Layout.preferredWidth: parent.width * 0.5

                                    text: (_dialog.initialFontFamily === family ? "* " : "") + family
                                    elide: Text.ElideRight
                                    padding: 3
                                }

                                VclLabel {
                                    Layout.fillWidth: true

                                    text: _dialog.previewText
                                    elide: Text.ElideRight
                                    padding: 3
                                    wrapMode: Text.NoWrap
                                    font.family: family
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: _fontList.currentIndex = index
                            }
                        }

                        Component.onCompleted: {
                            positionViewAtIndex(_fontFamiliesModel.initialIndex, ListView.Contain)
                            currentIndex = _fontFamiliesModel.initialIndex
                        }
                    }
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Select"
                    enabled: _fontList.currentIndex >= 0

                    onClicked: {
                        _private.fontWasSelected = true
                        _dialog.fontSelected(_fontList.currentItem.family)
                        Qt.callLater(_dialog.close)
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
