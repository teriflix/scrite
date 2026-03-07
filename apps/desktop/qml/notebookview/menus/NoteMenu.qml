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
import "../../controls"

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
