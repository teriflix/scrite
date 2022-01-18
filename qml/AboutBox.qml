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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    readonly property real splashWidth: 1464
    readonly property real splashHeight: 978
    readonly property real iscale: (ui.width * 0.5)/splashWidth
    readonly property real ascale: width/splashWidth
    width: Math.max(iscale * splashWidth, 973)
    height: Math.max(iscale * splashHeight, 650)

    Component.onCompleted: {
        modalDialog.closeUponClickOutsideContentArea = true
        modalDialog.closeable = false
    }

    SwipeView {
        id: aboutBoxPages
        anchors.fill: parent
        interactive: false

        Item {
            Image {
                source: modalDialog.t >= 1 ? "../images/aboutbox.jpg" : ""
                anchors.fill: parent
                smooth: true; mipmap: true
                asynchronous: true
            }

            Text {
                id: versionText
                x: 60 * ascale
                y: 135 * ascale
                font.pixelSize: 18
                color: "white"
                text: Scrite.app.applicationVersion
                font.letterSpacing: applicationSettings.enableAnimations ? 10 : 0

                NumberAnimation {
                    target: versionText
                    property: "font.letterSpacing"
                    from: 10; to: 0
                    duration: 1500
                    running: true && applicationSettings.enableAnimations
                    easing.type: Easing.OutBack
                }
            }

            Text {
                font.pixelSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Build Timestamp:\n" + Scrite.app.buildTimestamp
                anchors.left: versionText.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: versionText.x
            }

            Column {
                spacing: 10
                anchors.centerIn: parent

                Image {
                    source: "../images/appicon.png"
                    width: 92; height: 92
                    mipmap: true; smooth: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    font.pixelSize: 20
                    text: "<font color=\"gray\">WRITE YOUR NEXT BLOCKBUSTER!</font><br/><font color=\"blue\">https://www.scrite.io</font>"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.scrite.io")
                    }
                }

                Item {
                    width: parent.width
                    height: 30
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Column {
                        id: links1
                        spacing: 20
                        anchors.top: parent.top

                        Text {
                            text: "Using <strong>PhoneticTranslator</strong><br/><font color=\"blue\">https://sourceforge.net/projects/phtranslator/</font>"
                            font.pointSize: Scrite.app.idealFontPointSize - 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://sourceforge.net/projects/phtranslator/")
                            }
                        }

                        Text {
                            text: "Using <strong>Sonnet</strong> from KDE Frameworks 5<br/><font color=\"blue\">https://api.kde.org/frameworks/sonnet/html/index.html</font>"
                            font.pointSize: Scrite.app.idealFontPointSize - 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://api.kde.org/frameworks/sonnet/html/index.html")
                            }
                        }

                        Text {
                            text: "Using <strong>QuaZip</strong><br/><font color=\"blue\">https://github.com/stachenov/quazip</font>"
                            font.pointSize: Scrite.app.idealFontPointSize - 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://github.com/stachenov/quazip")
                            }
                        }

                    }

                    Column {
                        id: links2
                        spacing: 20
                        anchors.top: parent.top

                        Text {
                            text: "Developed using <strong>Qt " + Scrite.app.qtVersion + "</strong><br/><font color=\"blue\">https://www.qt.io</font>"
                            font.pointSize: Scrite.app.idealFontPointSize - 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://www.qt.io")
                            }
                        }

                        Text {
                            text: "Using <strong>SimpleCrypt</strong>.<br/><font color=\"blue\">Click here</font> to know more."
                            font.pointSize: Scrite.app.idealFontPointSize - 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://wiki.qt.io/Simple_encryption_with_SimpleCrypt")
                            }
                        }

                        Text {
                            text: "Using <strong>Curved Arrows</strong>.<br/><font color=\"blue\">Click here</font> to know more."
                            font.pointSize: Scrite.app.idealFontPointSize - 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://github.com/dragonman225/curved-arrows")
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: 14
                }

                Text {
                    text: "This app is released under <strong>GPLv3</strong>. <font color=\"blue\">Click here</font> to view the license terms."
                    font.pointSize: Scrite.app.idealFontPointSize - 2
                    anchors.horizontalCenter: parent.horizontalCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: aboutBoxPages.currentIndex = 1
                    }
                }

                Item {
                    width: parent.width
                    height: 14
                }

                SocialShareIcons {
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button2 {
                        text: "Website"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io")
                    }

                    Button2 {
                        text: "Help"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/help/")
                    }

                    Button2 {
                        text: "Feedback"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/forum/")
                    }
                }
            }
        }

        Rectangle {
            Button {
                id: backButton
                text: "< Back"
                onClicked: aboutBoxPages.currentIndex = 0
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 10
            }

            ScrollView {
                anchors.left: backButton.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.leftMargin: 10
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                TextEdit {
                    id: licenseTextView
                    readOnly: true
                    font.family: "Courier Prime"
                    font.pointSize: 12
                    topPadding: backButton.y
                    bottomPadding: backButton.y
                    text: Scrite.app.fileContents(":/LICENSE.txt")
                    selectByMouse: true
                }
            }
        }
    }
}
