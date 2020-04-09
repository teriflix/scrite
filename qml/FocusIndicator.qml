/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13

Rectangle {
    id: focusIndicator
    property bool active: false
    border.width: active ? 3 : 0
    color: Qt.rgba(0,0,0,0)

    SequentialAnimation {
        running: focusIndicator.active
        loops: Animation.Infinite

        ColorAnimation {
            from: app.palette.highlight
            to: app.palette.alternateBase // Qt.lighter(app.palette.highlight,1.25)
            duration: 1000
            target: focusIndicator.border
            properties: "color"
        }

        ColorAnimation {
            from: app.palette.alternateBase // Qt.lighter(app.palette.highlight,1.25)
            to: app.palette.highlight
            duration: 1000
            target: focusIndicator.border
            properties: "color"
        }
    }
}
