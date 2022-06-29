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

Item {
    id: busyIcon
    width: 48
    height: 48
    visible: false
    property alias running: busyIcon.visible
    property bool forDarkBackground: false

    Image {
        id: busyIconImage
        width: 48
        height: 48
        smooth: true
        mipmap: true
        source: forDarkBackground ? "../icons/content/time_inverted.png" : "../icons/content/time.png"
        anchors.centerIn: parent

        RotationAnimator {
            target: busyIconImage
            from: 0
            to: 360
            duration: 500
            loops: Animation.Infinite
            running: busyIcon.running
        }
    }
}
