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

import Scrite 1.0

Item {
    signal done()

    BorderImage {
        source: "../icons/content/shadow.png"
        anchors.fill: splashImageArea
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
        border { left: 21; top: 21; right: 21; bottom: 21 }
        smooth: true
        visible: true
        opacity: 0.75
    }

    Item {
        id: splashImageArea
        width: Math.min(Screen.width * 0.7, splashImage.sourceSize.width)
        height: width / 2.35
        anchors.centerIn: parent
        clip: true

        Image {
            id: splashImage
            source: "../images/splash.jpg"
            anchors.fill: parent
            smooth: true
            mipmap: true
        }

        Text {
            id: versionText
            x: (765 / splashImage.sourceSize.width) * parent.width
            y: (140 / splashImage.sourceSize.height) * parent.height
            font.family: "Raleway"
            font.pixelSize: 20
            text: app.applicationVersion
            color: "#4a4a4a"
        }

        NumberAnimation {
            target: versionText
            property: "font.letterSpacing"
            duration: 1000
            easing.type: Easing.OutBack
            from: 10
            to: 0
            running: true
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

    EventFilter.events: [6, 7, 31, 117, 51]
    EventFilter.target: app
    EventFilter.onFilter: {
        if(event.type === 6 && event.key === Qt.Key_Escape)
            done()
        result.acceptEvent = true
        result.filter = true
    }
}
