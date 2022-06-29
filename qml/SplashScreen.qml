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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    property bool animationsEnabled: Scrite.app.isWindowsPlatform ? !Scrite.app.isNotWindows10 : true
    signal done()

    BoxShadow {
        anchors.fill: splashImageArea
    }

    Item {
        id: splashImageArea
        width: Math.min(ui.width * 0.7, splashImage.sourceSize.width)
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
            x: parent.width - width - ((35 / splashImage.sourceSize.height) * parent.height)
            y: (750 / splashImage.sourceSize.height) * parent.height
            font.pixelSize: Scrite.app.idealFontPointSize + 1
            text: Scrite.app.applicationVersion
            color: "white"
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
    EventFilter.target: Scrite.app
    EventFilter.onFilter: {
        if(event.type === 6 && event.key === Qt.Key_Escape)
            done()
        result.acceptEvent = true
        result.filter = true
    }

    Label {
        anchors.centerIn: parent
        text: "Welcome to Scrite"
        Component.onCompleted: {
            if(Scrite.app.customFontPointSize === 0)
                Scrite.app.customFontPointSize = font.pointSize
            visible = false
        }
    }
}
