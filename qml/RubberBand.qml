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
import QtQuick.Controls 2.13

Item {
    id: rubberband
    signal tryStart(point pos)
    signal select(rect rectangle)
    property bool active: false

    Rectangle {
        id: selection
        property point from: Qt.point(0,0)
        property point to: Qt.point(0,0)

        color: app.translucent(app.palette.highlight,0.2)
        border { width: 2; color: app.palette.highlight }

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
            if(mouse.modifiers & Qt.ControlModifier) {
                var pos = Qt.point(mouse.x, mouse.y)
                rubberband.tryStart(pos)

                if(rubberband.active) {
                    selection.from = pos
                    selection.to = selection.from
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
                rubberband.active = false
                mouse.accepted = true
            } else
                mouse.accepted = false
        }
    }
}
