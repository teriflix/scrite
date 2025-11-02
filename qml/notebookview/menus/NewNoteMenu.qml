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
import "qrc:/qml/controls"

VclMenu {
    id: root

    required property Notes notes

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    enabled: notes

    ColorMenu {
        title: "Text Note"

        onMenuItemClicked: (color) => {
                               let note = root.notes.addTextNote()
                               if(note) {
                                   note.color = color
                                   note.objectName = "_newNote"
                                   root.switchRequest(note)
                                   root.close()
                               }
                           }
    }

    FormMenu {
        title: "Form Note"
        notes: root.notes

        onNoteAdded: (note) => {
                         root.switchRequest(note)
                         root.close()
                     }
    }

    ColorMenu {
        title: "Checklist Note"

        onMenuItemClicked: (color) => {
                               var note = root.notes.addCheckListNote()
                               if(note) {
                                   note.color = color
                                   note.objectName = "_newNote"
                                   root.switchRequest(note)
                               }
                               root.close()
                           }
    }
}
