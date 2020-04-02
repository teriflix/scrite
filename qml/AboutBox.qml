/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
        spacing: 40

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
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Version " + app.applicationVersion
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            text: "Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd.\n" +
                  "Developed using Qt " + app.qtVersion;
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            text: "Using <strong>PhoneticTranslator</strong> for transliteration support.<br/><font color=\"blue\">https://sourceforge.net/projects/phtranslator/</font>"

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally("https://sourceforge.net/projects/phtranslator/")
            }
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Visit Website"
            onClicked: Qt.openUrlExternally("https://www.teriflix.com")
        }
    }
}
