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
import QtQuick.Controls 2.13
import Scrite 1.0

Image {
    source: "../images/aboutbox.jpg"
    width: Screen.width * 0.5
    height: iscale*sourceSize.height
    smooth: true; mipmap: true

    Component.onCompleted: {
        modalDialog.closeUponClickOutsideContentArea = true
        modalDialog.closeable = false
    }

    property real iscale: width / sourceSize.width

    Text {
        id: versionText
        x: 60 * iscale
        y: 125 * iscale
        font.family: "Raleway"
        font.pixelSize: 18
        color: "white"
        text: app.applicationVersion
        font.letterSpacing: 10

        NumberAnimation {
            target: versionText
            property: "font.letterSpacing"
            from: 10; to: 0
            duration: 1500
            running: true
            easing.type: Easing.OutBack
        }
    }

    SwipeView {
        id: aboutBoxPages
        anchors.fill: parent
        interactive: false

        Item {
            Column {
                spacing: 50
                anchors.centerIn: parent

                Image {
                    source: "../images/appicon.png"
                    width: 92; height: 92
                    mipmap: true; smooth: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    font.pixelSize: 20
                    text: "© TERIFLIX Entertainment Spaces Pvt. Ltd.<br/><font color=\"blue\">https://www.teriflix.com</font>"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.teriflix.com")
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Column {
                        id: links1
                        spacing: 20

                        Text {
                            text: "Using <strong>PhoneticTranslator</strong><br/><font color=\"blue\">https://sourceforge.net/projects/phtranslator/</font>"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://sourceforge.net/projects/phtranslator/")
                            }
                        }

                        Text {
                            text: "Using <strong>Sonnet</strong> from KDE Frameworks 5<br/><font color=\"blue\">https://api.kde.org/frameworks/sonnet/html/index.html</font>"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://api.kde.org/frameworks/sonnet/html/index.html")
                            }
                        }
                    }

                    Column {
                        id: links2
                        spacing: 20

                        Text {
                            text: "Developed using <strong>Qt " + app.qtVersion + "</strong><br/><font color=\"blue\">https://www.qt.io</font>"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://www.qt.io")
                            }
                        }

                        Text {
                            text: "This app is released under <strong>GPLv3</strong>.<br/><font color=\"blue\">Click here</font> to view the license terms."

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: aboutBoxPages.currentIndex = 1
                            }
                        }
                    }
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
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/help/#feedback")
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
                anchors.fill: parent
                anchors.leftMargin: Math.max(10, (parent.width-licenseTextView.contentWidth-20)/2)
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                TextEdit {
                    id: licenseTextView
                    readOnly: true
                    font.family: "Courier Prime"
                    font.pointSize: 12
                    topPadding: backButton.y
                    bottomPadding: backButton.y
                    text: app.fileContents(":/LICENSE.txt")
                    selectByMouse: true
                }
            }
        }
    }
}














