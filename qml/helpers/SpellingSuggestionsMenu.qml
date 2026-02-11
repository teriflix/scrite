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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

MenuLoader {
    id: root

    property var spellingSuggestions

    signal menuAboutToShow()
    signal menuAboutToHide()
    signal replaceRequest(string suggestion)
    signal addToDictionaryRequest()
    signal addToIgnoreListRequest()

    enabled: !Scrite.document.readOnly

    menu: VclMenu {
        property int cursorPosition: -1

        onAboutToShow: root.menuAboutToShow()
        onAboutToHide: root.menuAboutToHide()

        Repeater {
            id: suggestionsRepeater

            model: root.spellingSuggestions

            delegate: VclMenuItem {
                required property int index
                required property string modelData

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

