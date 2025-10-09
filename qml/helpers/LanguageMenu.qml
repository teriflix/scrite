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
            required property int index
            required property var language // This is of type Language, but we have to use var here.
                                           // You cannot use Q_GADGET struct names as type names in QML
                                           // that privilege is only reserved for QObject types.

            text: {
                const shortcut = language.shortcut()
                let ret = language.name
                if(shortcut !== "")
                    ret += "\t\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)
                return ret
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

        let widthRequired = 0
        for(let i=0; i<count; i++) {
            const item = itemAt(i)
            if(item.text) {
                widthRequired = Math.max( Runtime.idealFontMetrics.boundingRect(item.text).width, widthRequired )
            }
        }

        width = widthRequired + 100
    }

    onAboutToHide: {
        removeItem(__separator)
        __separator.destroy()

        removeItem(__moreLanguages)
        __moreLanguages.destroy()
    }
}
