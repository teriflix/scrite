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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Menu2 {
    id: breakElementContextMenu
    property ScreenplayElement element
    onClosed: element = null

    MenuItem2 {
        text: "Paste After"
        enabled: Scrite.document.screenplay.canPaste
        onClicked: {
            const index = Scrite.document.screenplay.indexOfElement(breakElementContextMenu.element);
            Scrite.document.screenplay.pasteAfter(index)
            breakElementContextMenu.close()
        }
    }

    MenuItem2 {
        text: "Remove"
        enabled: !Scrite.document.readOnly
        onClicked: {
            Scrite.document.screenplay.removeElement(breakElementContextMenu.element)
            breakElementContextMenu.close()
        }
    }
}
