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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

BoxShadow {
    id: root

    property StructureElement currentElement: currentElementItem ? currentElementItem.element : null
    property AbstractStructureElementUI currentElementItem: null

    signal deleteElementRequest(StructureElement element)

    x: currentElementItem ? _private.currentElementRect.x : 0
    y: currentElementItem ? _private.currentElementRect.y : 0
    width: currentElementItem ? _private.currentElementRect.width : 0
    height: currentElementItem ? _private.currentElementRect.height : 0

    QtObject {
        id: _private

        property rect currentElementRect: root.currentElementItem ? root.currentElementItem.mapFromItem(root.parent, 0, 0, root.currentElementItem.width, root.currentElementItem.height) : Qt.rect(0,0,0,0)
    }
}
