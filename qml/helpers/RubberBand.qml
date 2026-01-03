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
    id: root

    signal select(rect rectangle)
    signal tryStart(point pos)

    property bool selectionMode: false
    property bool active: false
    property bool selecting: false

    Rectangle {
        id: _selection

        property point from: Qt.point(0,0)
        property point to: Qt.point(0,0)

        property rect rectangle: {
            if(from === to)
                return Qt.rect(from.x, from.y, 1, 1)
            return Qt.rect( Math.min(from.x,to.x), Math.min(from.y,to.y), Math.abs(to.x-from.x), Math.abs(to.y-from.y) )
        }

        x: rectangle.x
        y: rectangle.y
        width: rectangle.width
        height: rectangle.height

        color: Color.translucent(Scrite.app.palette.highlight,0.2)
        border { width: 2; color: Scrite.app.palette.highlight }
        visible: root.active
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: {
            if(mouse.modifiers & Qt.ControlModifier || parent.selectionMode) {
                var pos = Qt.point(mouse.x, mouse.y)
                root.tryStart(pos)

                if(root.active) {
                    _selection.from = pos
                    _selection.to = _selection.from
                    selecting = true
                    mouse.accepted = true
                } else
                    mouse.accepted = false
            } else {
                _rubberband.active = false
                mouse.accepted = false
            }
        }
        onPositionChanged: {
            if(root.active) {
                _selection.to = Qt.point(mouse.x, mouse.y)
                mouse.accepted = true
            } else
                mouse.accepted = false
        }
        onReleased: {
            if(root.active) {
                _selection.to = Qt.point(mouse.x, mouse.y)
                root.select(_selection.rectangle)
                selecting = false
                root.active = false
                mouse.accepted = true
            } else
                mouse.accepted = false
        }
    }
}
