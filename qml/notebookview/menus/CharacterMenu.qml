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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

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
