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

Rectangle {
    id: focusIndicator
    property bool active: false
    border.width: active ? 2 : 0
    border.color: color1
    color: accentColors.c10.background
    property color color1: Scrite.app.translucent(accentColors.c800.background, 0.55)
    property color color2: Scrite.app.translucent(accentColors.c500.background, 0.45)

    SequentialAnimation {
        running: focusIndicator.active
        loops: Animation.Infinite

        ColorAnimation {
            from: focusIndicator.color1
            to: focusIndicator.color2
            duration: 1000
            target: focusIndicator.border
            properties: "color"
        }

        ColorAnimation {
            from: focusIndicator.color2
            to: focusIndicator.color1
            duration: 1000
            target: focusIndicator.border
            properties: "color"
        }
    }
}
