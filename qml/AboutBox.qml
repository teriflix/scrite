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
    width: 500
    height: 600

    Column {
        width: parent.width * 0.75
        anchors.centerIn: parent
        spacing: 30

        Column {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../images/teriflix_logo.png"
                fillMode: Image.PreserveAspectFit
                height: 128
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "scrite"
                font.pixelSize: 80
                color: accentColors.c50.text
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 24
                text: "Version " + app.applicationVersion
                color: accentColors.c50.text
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
                text: "Developed using Qt " + app.qtVersion + " LGPL<br/><font color=\"blue\">https://www.qt.io</font>"
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
