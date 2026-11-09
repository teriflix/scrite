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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../helpers"
import "../../controls"
import ".."
import "../tabs"

AbstractNotebookTabPage {
    id: root

    signal deleteCharacterRequest(Character character)

    hostUri: "Scrite.App.Notebook.CharactersPage"
    tabBarName: "Characters"
    contentMargin: 11
    currentTabUri: Runtime.notebookSettings.charactersPageTabUri

    NotebookPageTab {
        uri: "Scrite.NotebookTab.CharactersPage.ListTab"
        title: "List"
        order: 100

        AllCharactersTab {
            onSwitchRequest: (item) => { root.switchRequest(item) }
            onDeleteCharacterRequest: (character) => { root.deleteCharacterRequest(character) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.CharactersPage.RelationshipsTab"
        title: "Relationships"
        order: 200

        AllCharactersRelationshipsTab {
            onSwitchRequest: (item) => { root.switchRequest(item) }
        }
    }

    onCurrentTabUriChanged: Runtime.notebookSettings.charactersPageTabUri = currentTabUri
}
