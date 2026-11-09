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
import "../../dialogs"
import "../../helpers"
import "../../controls"
import ".."
import "../tabs"

AbstractNotebookTabPage {
    id: root

    hostUri: "Scrite.App.Notebook.ScreenplayPage"

    notes: _private.structure.notes

    readonly property Structure structure: owner as Structure
    tabBarName: "Screenplay"
    currentTabUri: Runtime.notebookSettings.screenplayPageTabUri

    NotebookPageTab {
        uri: "Scrite.NotebookTab.ScreenplayPage.TitlePageTab"
        title: "Title Page"
        order: 100

        ScreenplayTitlePageTab {
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.ScreenplayPage.LoglineTab"
        title: "Logline"
        order: 200

        ScreenplayLoglineTab {
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.ScreenplayPage.NotesTab"
        title: "Notes"
        order: 300

        NotesTab {
            notes: _private.structure.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
            onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.ScreenplayPage.StatsTab"
        title: "Stats"
        order: 400

        ScreenplayStatsTab { }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.ScreenplayPage.AttributesTab"
        title: "Attributes"
        order: 500

        AttributesTab {
            notes: _private.structure.notes
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export entire screenplay notes as PDF or ODT."

        onTriggered: () => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.screenplay
                         ReportConfigurationDialog.launch(generator)
                     }
    }

    onCurrentTabUriChanged: Runtime.notebookSettings.screenplayPageTabUri = currentTabUri

    QtObject {
        id: _private

        property Structure structure: Scrite.document.structure

        property Screenplay screenplay: Scrite.document.screenplay
    }
}
