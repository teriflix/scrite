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
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    width: 700
    height: 700

    SwipeView {
        id: aboutBoxPages
        anchors.fill: parent
        interactive: false

        Item {
            Column {
                width: parent.width * 0.75
                anchors.centerIn: parent
                spacing: 30

                Column {
                    spacing: 10
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "../images/teriflix_logo.png"
                        fillMode: Image.PreserveAspectFit
                        height: 128
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: app.applicationName
                        font.family: "Courier Prime"
                        font.pixelSize: 80
                        color: accentColors.c50.text
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: 20
                        text: "Version " + app.applicationVersion
                        color: accentColors.c50.text
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: 14
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: "This app is released under GPLv3.<br/><font color=\"blue\">Click here</font> to view the license terms."
                        wrapMode: Text.WordWrap
                        color: accentColors.c50.text

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: aboutBoxPages.currentIndex = 1
                        }
                    }
                }

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    text: "© TERIFLIX Entertainment Spaces Pvt. Ltd.<br/><font color=\"blue\">https://www.teriflix.com</font>"
                    color: accentColors.c50.text
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.teriflix.com")
                    }
                }

                Column {
                    width: parent.width
                    spacing: parent.spacing/3

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        text: "Developed using Qt " + app.qtVersion + " <br/><font color=\"blue\">https://www.qt.io</font>"
                        color: accentColors.c50.text

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://www.qt.io")
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        text: "Using <strong>PhoneticTranslator</strong> for transliteration support.<br/><font color=\"blue\">https://sourceforge.net/projects/phtranslator/</font>"
                        color: accentColors.c50.text

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://sourceforge.net/projects/phtranslator/")
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        text: "Using <strong>Sonnet</strong> from KDE Frameworks 5 for Spellcheck.<br/><font color=\"blue\">https://api.kde.org/frameworks/sonnet/html/index.html</font>"
                        color: accentColors.c50.text

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://api.kde.org/frameworks/sonnet/html/index.html")
                        }
                    }
                }

                Row {
                    spacing: 10
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

        Item {
            Button {
                id: backButton
                text: "< Back"
                onClicked: aboutBoxPages.currentIndex = 0
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 10
            }

            ScrollView {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: backButton.bottom
                anchors.bottom: parent.bottom
                anchors.margins: 10
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                TextEdit {
                    readOnly: true
                    font.family: "Courier Prime"
                    font.pointSize: 10
                    text: app.fileContents(":/LICENSE.txt")
                    selectByMouse: true
                }
            }
        }
    }
}
