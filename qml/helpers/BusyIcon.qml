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
    id: root

    property bool forDarkBackground: false

    property alias running: root.visible

    height: 48
    width: 48

    visible: false

    Image {
        id: _image

        anchors.centerIn: parent

        height: 48
        width: 48

        mipmap: true
        smooth: true
        source: forDarkBackground ? "qrc:/icons/content/time_inverted.png" : "qrc:/icons/content/time.png"

        RotationAnimator {
            duration: 500
            from: 0
            loops: Animation.Infinite
            running: root.running
            target: _image
            to: 360
        }
    }
}
