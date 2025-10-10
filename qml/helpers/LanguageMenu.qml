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
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

VclMenu {
    id: root

    title: "Language"

    Repeater {
        model: LanguageEngine.supportedLanguages

        VclMenuItem {
            id: _languageMenuItem

            required property int index
            required property var language // This is of type Language, but we have to use var here.
                                           // You cannot use Q_GADGET struct names as type names in QML
                                           // that privilege is only reserved for QObject types.

            contentItem: GridLayout {
                columns: 3
                rowSpacing: 10
                columnSpacing: 5

                Rectangle {
                    Layout.rightMargin: parent.rowSpacing

                    implicitWidth: Runtime.idealFontMetrics.averageCharacterWidth * 4
                    implicitHeight: implicitWidth

                    color: Runtime.colors.primary.c800.background
                    radius: Math.min(width,height) * 0.2

                    VclLabel {
                        anchors.centerIn: parent

                        text: _languageMenuItem.language.glyph
                        color: Runtime.colors.primary.c800.text

                        font.bold: true
                        font.family: _languageMenuItem.language.font().family
                    }
                }

                VclText {
                    Layout.fillWidth: true

                    text: _languageMenuItem.language.name
                    font: _languageMenuItem.font
                }

                VclText {
                    property string shortcut: _languageMenuItem.language.shortcut()

                    text: Scrite.app.polishShortcutTextForDisplay(shortcut)
                    font: _languageMenuItem.font
                }
            }

            font.bold: Runtime.language.activeCode === language.code

            onTriggered: Runtime.language.setActiveCode(language.code)
        }
    }

    // Private implementation
    property Item __separator
    property Item __moreLanguages

    readonly property Component __separatorComponent : MenuSeparator {
        id: _separator
    }

    readonly property Component __moreLanguagesComponent : VclMenuItem {
        text: "More Languages .."

        onTriggered: LanguageOptionsDialog.launch()
    }

    onAboutToShow: {
        __separator = __separatorComponent.createObject(root)
        addItem(__separator)

        __moreLanguages = __moreLanguagesComponent.createObject(root)
        addItem(__moreLanguages)

        const iconWidth = Runtime.idealFontMetrics.averageCharacterWidth * 4 + 10

        let widthRequired = 0
        for(let i=0; i<count; i++) {
            const item = itemAt(i)
            if(item.language) {
                const shortcut = Scrite.app.polishShortcutTextForDisplay(item.language.shortcut())
                widthRequired = Math.max( Runtime.idealFontMetrics.boundingRect(item.language.name).width +
                                          Runtime.idealFontMetrics.boundingRect(shortcut).width + iconWidth, widthRequired )
            }
        }

        contentWidth = widthRequired + 100
    }

    onAboutToHide: {
        removeItem(__separator)
        __separator.destroy()

        removeItem(__moreLanguages)
        __moreLanguages.destroy()
    }
}
