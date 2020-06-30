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
import QtQuick.Window 2.13

Item {
    signal done()

    BorderImage {
        source: "../icons/content/shadow.png"
        anchors.fill: splashImage
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
        border { left: 21; top: 21; right: 21; bottom: 21 }
        smooth: true
        visible: true
        opacity: 0.75
    }

    Image {
        id: splashImage
        width: Screen.width * 0.6
        height: width / 2.35
        smooth: true
        mipmap: true
        source: "../images/splash.jpg"
        anchors.centerIn: parent

        Text {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: parent.height * 0.025
            anchors.leftMargin: parent.width * 0.05
            font.family: "Raleway"
            font.pixelSize: parent.height * 0.04
            text: app.applicationVersion
        }
    }

    Timer {
        running: true
        repeat: false
        interval: 2500
        onTriggered: parent.done()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: parent.done()
    }
}
