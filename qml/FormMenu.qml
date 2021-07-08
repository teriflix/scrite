/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Menu2 {
    title: "Forms"

    property Notes notes
    property alias formTypes: formFilterModel.filterValues
    signal formClicked(string formId)
    signal noteAdded(Note note)

    SortFilterObjectListModel {
        id: formFilterModel
        sourceModel: scriteDocument.globalForms
        sortByProperty: "title"
        filterByProperty: "type"
        filterValues: notes ? [notes.compatibleFormType] : undefined
    }

    enabled: formFilterModel.objectCount > 0

    Repeater {
        model: formFilterModel

        MenuItem2 {
            text: objectItem.title
            onClicked: {
                if(notes) {
                    var note = notes.addFormNote(objectItem.id)
                    note.objectName = "_newNote"
                    noteAdded(note)
                } else {
                    formClicked(objectItem.id)
                }
            }
        }
    }
}
