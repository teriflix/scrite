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

    EventFilter.target: Scrite.app
    EventFilter.active: enabled && visible && opacity === 1
    EventFilter.events: [EventFilter.KeyPress]
    EventFilter.onFilter: (object, event, result) => { _private.eventFilter(object, event, result) }

    x: currentElementItem ? _private.currentElementRect.x : 0
    y: currentElementItem ? _private.currentElementRect.y : 0
    width: currentElementItem ? _private.currentElementRect.width : 0
    height: currentElementItem ? _private.currentElementRect.height : 0

    QtObject {
        id: _private

        property rect currentElementRect: root.currentElementItem ? root.currentElementItem.mapFromItem(root.parent, 0, 0, root.currentElementItem.width, root.currentElementItem.height) : Qt.rect(0,0,0,0)

        function eventFilter(object, event, result) {
            if(root.currentElementItem === null || root.currentElement === null)
                return

            const dist = (event.controlModifier ? 5 : 1) * root.tickDistance
            const element = root.currentElement

            const fbbl = Scrite.document.structure.forceBeatBoardLayout

            if(!fbbl)
                element.undoRedoEnabled = true

            switch(event.key) {
            case Qt.Key_Left:
                if(fbbl) return
                element.x -= dist
                result.accept = true
                result.filter = true
                break
            case Qt.Key_Right:
                if(fbbl) return
                element.x += dist
                result.accept = true
                result.filter = true
                break
            case Qt.Key_Up:
                if(fbbl) return
                element.y -= dist
                result.accept = true
                result.filter = true
                break
            case Qt.Key_Down:
                if(fbbl) return
                element.y += dist
                result.accept = true
                result.filter = true
                break
            case Qt.Key_Delete:
            case Qt.Key_Backspace:
                result.accept = true
                result.filter = true
                if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && element.follow)
                    element.follow.confirmAndDeleteSelf()
                else
                    root.deleteElementRequest(root.currentElement)
                break
            }

            element.undoRedoEnabled = false
        }
    }
}
