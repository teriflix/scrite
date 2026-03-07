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

import QtQuick
import QtQuick.Window
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../helpers"
import "../../dialogs"
import "../../controls"

VclMenu {
    id: root

    property Character character

    signal deleteCharacterRequest()

    width: 250
    enabled: character

    onAboutToHide: character = null

    ColorMenu {
        title: "Character Color"

        onMenuItemClicked: {
            root.character.color = color
            root.close()
        }
    }

    VclMenuItem {
        text: "Rename/Merge Character"

        onClicked: RenameCharacterDialog.launch(root.character)
    }

    VclMenu {
        title: "Reports"

        Repeater {
            model: Runtime.characterReports.reports ? Runtime.characterReports.reports : 0

            delegate: VclMenuItem {
                required property int index
                required property var modelData

                text: modelData.name
                icon.source: "qrc" + modelData.icon

                onTriggered: {
                    let props = {}
                    props[Runtime.characterReports.propertyName] = [root.character.name]
                    ReportConfigurationDialog.launch(modelData.name, props, {initialPage: modelData.group})
                }
            }
        }
    }

    MenuSeparator { }

    VclMenuItem {
        text: "Delete Character"

        onClicked: {
            root.deleteCharacterRequest()
            root.close()
        }
    }
}
