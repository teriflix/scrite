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
import QtQuick.Controls
import QtQuick.Layouts

import Scrite.App

import "../../globals"
import "../../helpers"
import "../../controls"
import ".."
import "../tabs"

AbstractNotebookTabPage {
    id: root

    hostUri: "Scrite.App.Notebook.BreakSummaryPage"

    notes: _private.notes

    readonly property ScreenplayElement breakElement: owner as ScreenplayElement
    tabBarName: _private.breakElement ? _private.breakElement.breakTitle : ""
    tabsVisible: _private.breakElement !== null
    currentTabUri: Runtime.notebookSettings.breakPageTabUri

    NotebookPageTab {
        uri: "Scrite.NotebookTab.BreakSummaryPage.InformationTab"
        title: "Information"
        order: 100

        BreakInformationTab {
            breakElement: _private.breakElement
            breakKind: _private.breakKind
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.BreakSummaryPage.NotesTab"
        title: "Notes"
        order: 200

        NotesTab {
            notes: _private.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
            onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.BreakSummaryPage.AttributesTab"
        title: "Attributes"
        order: 300

        AttributesTab {
            notes: _private.notes
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    SctLabel {
        anchors.centerIn: parent

        width: parent.width * 0.6

        horizontalAlignment: Text.AlignHCenter
        text: "Create " + _private.breakKind.toLowerCase() + " break in the screenplay to capture a summary for it."
        visible: _private.breakElement === null
        wrapMode: Text.WordWrap

        font.bold: true
        font.pointSize: Runtime.notebookFontPointSize + 3
    }

    onCurrentTabUriChanged: Runtime.notebookSettings.breakPageTabUri = currentTabUri

    QtObject {
        id: _private

        property ScreenplayElement breakElement: root.pageData

        property Notes notes: breakElement ? breakElement.notes : null

        property string breakKind: breakElement ? (breakElement.notebookItemType === NotebookModel.EpisodeBreakType ? "Episode" : "Act") : "a"
    }
}
