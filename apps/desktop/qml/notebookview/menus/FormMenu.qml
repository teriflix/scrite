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

import QtQuick
import QtQuick.Window
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../controls"

VclMenu {
    id: root

    property Notes notes
    property alias formTypes: _model.filterValues

    signal formClicked(string formId)
    signal noteAdded(Note note)

    width: 325

    enabled: _model.objectCount > 0
    title: "Forms"

    Repeater {
        model: _model

        delegate: VclMenuItem {
            required property int index
            required property var objectItem

            text: objectItem.title

            onClicked: {
                if(notes) {
                    var note = notes.addFormNote(objectItem.id)
                    note.objectName = "_newNote"
                    root.noteAdded(note)
                } else {
                    root.formClicked(objectItem.id)
                }
            }
        }
    }

    SortFilterObjectListModel {
        id: _model

        filterByProperty: "type"
        filterValues: notes ? [notes.compatibleFormType] : []
        sortByProperty: "title"
        sourceModel: Scrite.document.globalForms
    }
}
