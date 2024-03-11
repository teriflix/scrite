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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "./globals"
import "./controls"

MenuLoader {
    id: root
    enabled: !Scrite.document.readOnly

    property var spellingSuggestions
    signal menuAboutToShow()
    signal menuAboutToHide()
    signal replaceRequest(string suggestion)
    signal addToDictionaryRequest()
    signal addToIgnoreListRequest()

    menu: VclMenu {
        id: spellingSuggestionsMenu
        property int cursorPosition: -1
        onAboutToShow: root.menuAboutToShow()
        onAboutToHide: root.menuAboutToHide()

        Repeater {
            id: suggestionsRepeater
            model: root.spellingSuggestions

            VclMenuItem {
                text: modelData
                focusPolicy: Qt.NoFocus
                onClicked: {
                    Qt.callLater(root.replaceRequest, modelData)
                    root.close()
                }
            }
        }

        MenuSeparator { }

        VclMenuItem {
            text: "Add to dictionary"
            focusPolicy: Qt.NoFocus
            onClicked: {
                Qt.callLater(root.addToDictionaryRequest)
                root.close()
            }
        }

        VclMenuItem {
            text: "Ignore"
            focusPolicy: Qt.NoFocus
            onClicked: {
                Qt.callLater(root.addToIgnoreListRequest)
                root.close()
            }
        }
    }
}

