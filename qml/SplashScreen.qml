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
    property bool animationsEnabled: app.isWindowsPlatform ? !app.isNotWindows10 : true
    signal done()

    BoxShadow {
        anchors.fill: splashImageArea
    }

    Item {
        id: splashImageArea
        width: Math.min(ui.width * 0.6, splashImage.sourceSize.width)
        height: width / 2.35
        anchors.centerIn: parent
        clip: true

        Image {
            id: splashImage
            source: "../images/splash.jpg"
            anchors.fill: parent
            smooth: true
            mipmap: true
            asynchronous: true
        }

        Text {
            id: versionText
            x: (1018 / splashImage.sourceSize.width) * parent.width
            y: (187 / splashImage.sourceSize.height) * parent.height
            font.family: "Raleway"
            font.pixelSize: 24
            text: app.applicationVersion
            color: "#4a4a4a"
        }

        SequentialAnimation {
            running: true & animationsEnabled

            ParallelAnimation {
                NumberAnimation {
                    target: versionText
                    property: "opacity"
                    duration: 500
                    easing.type: Easing.OutBack
                    from: 0; to: 0.8
                }

                NumberAnimation {
                    target: versionText
                    property: "font.letterSpacing"
                    duration: 500
                    easing.type: Easing.OutBack
                    from: 10; to: 0
                }
            }

            PauseAnimation {
                duration: 500
            }

            ParallelAnimation {
                NumberAnimation {
                    target: footnoteText
                    property: "opacity"
                    duration: 500
                    easing.type: Easing.OutBack
                    from: 0; to: 0.8
                }

                NumberAnimation {
                    target: footnoteText
                    property: "font.letterSpacing"
                    duration: 500
                    easing.type: Easing.OutBack
                    from: 10; to: 0
                }
            }
        }
    }

    Text {
        id: footnoteText
        anchors.top: splashImageArea.bottom
        anchors.topMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 20
        text: "Click anywhere to get started."
        font.letterSpacing: animationsEnabled ? 10 : 0
        opacity: animationsEnabled ? 0.01 : 1
        color: "#4a4a4a"
    }

    Timer {
        running: true
        repeat: false
        interval: 7500
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
