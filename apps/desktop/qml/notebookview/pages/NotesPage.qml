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

    hostUri: "Scrite.App.Notebook.NotesPage"
    showTabs: false

    notes: root.pageData ? root.pageData.notebookItemObject : null
    tabBarName: notes ? notes.title : "Notes"

    NotebookPageTab {
        uri: "Scrite.NotebookTab.NotesPage.NotesTab"
        title: "Notes"
        order: 100

        NotesTab {
            notes: root.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
            onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.NotesPage.AttributesTab"
        title: "Attributes"
        order: 200

        AttributesTab {
            notes: root.notes
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }
}
