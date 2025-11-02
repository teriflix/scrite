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

    required property Note note

    signal deleteNoteRequest()

    enabled: note

    onAboutToHide: note = null

    ColorMenu {
        title: "Note Color"
        selectedColor: note.color

        onMenuItemClicked: (color) => {
                               root.note.color = color
                               root.close()
                           }
    }

    MenuSeparator { }

    VclMenuItem {
        text: "Delete Note"
        onClicked: () => {
                       root.deleteNoteRequest()
                       root.close()
                   }
    }
}
