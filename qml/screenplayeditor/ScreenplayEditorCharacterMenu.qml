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
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

VclMenu {
    id: root

    property Item popupSource

    property string characterName

    width: 350

    onAboutToHide: {
        popupSource = null
        characterName = ""
    }

    Repeater {
        model: Runtime.characterListReports

        VclMenuItem {
            required property var modelData

            text: modelData.name
            icon.source: "qrc" + modelData.icon

            onTriggered: ReportConfigurationDialog.launch(modelData.name,
                                                          {"characterNames": [root.characterName]},
                                                          {"initialPage": modelData.group})
        }
    }

    MenuSeparator { }

    VclMenuItem {
        text: "Character Notes"
        icon.source: "qrc:/icons/content/note.png"

        onTriggered: {
            let characterNotes = ActionHub.notebookOperations.find("characterNotes")
            characterNotes.characterName = root.characterName
            characterNotes.trigger()
        }
    }

    Repeater {
        model: Runtime.characterListReports.length > 0 ? 1 : 0

        VclMenuItem {
            text: "Rename/Merge Character"
            icon.source: "qrc:/icons/screenplay/character.png"

            onTriggered: {
                const character = Scrite.document.structure.addCharacter(root.characterName)
                if(character)
                    RenameCharacterDialog.launch(character)
            }
        }
    }
}
