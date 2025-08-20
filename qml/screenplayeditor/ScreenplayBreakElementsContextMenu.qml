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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

VclMenu {
    id: root

    property ScreenplayElement element

    onClosed: element = null

    VclMenuItem {
        text: "Paste After"
        enabled: Scrite.document.screenplay.canPaste
        onClicked: {
            const index = Scrite.document.screenplay.indexOfElement(root.element);
            Scrite.document.screenplay.pasteAfter(index)
            root.close()
        }
    }

    VclMenuItem {
        text: "Remove"
        enabled: !Scrite.document.readOnly
        onClicked: {
            Scrite.document.screenplay.removeElement(root.element)
            root.close()
        }
    }
}
