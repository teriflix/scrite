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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    id: rubberband
    signal tryStart(point pos)
    signal select(rect rectangle)

    property bool selectionMode: false
    property bool active: false
    property bool selecting: false

    Rectangle {
        id: selection
        property point from: Qt.point(0,0)
        property point to: Qt.point(0,0)

        color: Scrite.app.translucent(Scrite.app.palette.highlight,0.2)
        border { width: 2; color: Scrite.app.palette.highlight }

        property rect rectangle: {
            if(from === to)
                return Qt.rect(from.x, from.y, 1, 1)
            return Qt.rect( Math.min(from.x,to.x), Math.min(from.y,to.y), Math.abs(to.x-from.x), Math.abs(to.y-from.y) )
        }

        x: rectangle.x
        y: rectangle.y
        width: rectangle.width
        height: rectangle.height
        visible: rubberband.active
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: {
            if(mouse.modifiers & Qt.ControlModifier || parent.selectionMode) {
                var pos = Qt.point(mouse.x, mouse.y)
                rubberband.tryStart(pos)

                if(rubberband.active) {
                    selection.from = pos
                    selection.to = selection.from
                    selecting = true
                    mouse.accepted = true
                } else
                    mouse.accepted = false
            } else {
                rubberBand.active = false
                mouse.accepted = false
            }
        }
        onPositionChanged: {
            if(rubberband.active) {
                selection.to = Qt.point(mouse.x, mouse.y)
                mouse.accepted = true
            } else
                mouse.accepted = false
        }
        onReleased: {
            if(rubberband.active) {
                selection.to = Qt.point(mouse.x, mouse.y)
                rubberband.select(selection.rectangle)
                selecting = false
                rubberband.active = false
                mouse.accepted = true
            } else
                mouse.accepted = false
        }
    }
}
