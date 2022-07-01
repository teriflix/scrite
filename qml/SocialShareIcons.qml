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
    width: buttonsRow.width + 2*buttonsRow.spacing
    height: buttonsRow.height + 2*buttonsRow.spacing
    property bool displayShareText: true

    Row {
        id: buttonsRow
        anchors.centerIn: parent

        Image {
            source: "../icons/action/share.png"
            anchors.verticalCenter: parent.verticalCenter
            width: 24; height: 24
            visible: displayShareText
        }

        Text {
            font.pointSize: Scrite.app.idealFontPointSize
            leftPadding: 10; rightPadding: 3
            text: "Share Scrite: "
            visible: displayShareText
            anchors.verticalCenter: parent.verticalCenter
        }

        Row {
            spacing: -8

            ToolButton3 {
                iconSource: "../icons/action/share_on_facebook.png"
                suggestedWidth: 50; suggestedHeight: 50
                onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_facebook")
                ToolTip.text: "Post about Scrite on your Facebook page."
            }

            ToolButton3 {
                iconSource: "../icons/action/share_on_linkedin.png"
                suggestedWidth: 50; suggestedHeight: 50
                onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_linkedin")
                ToolTip.text: "Post about Scrite on your LinkedIn page."
            }

            ToolButton3 {
                iconSource: "../icons/action/share_on_twitter.png"
                suggestedWidth: 50; suggestedHeight: 50
                onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_twitter")
                ToolTip.text: "Tweet about Scrite from your handle."
            }

            ToolButton3 {
                iconSource: "../icons/action/share_on_email.png"
                suggestedWidth: 50; suggestedHeight: 50
                readonly property string url: "mailto:?Subject=Take a look at Scrite&Body=I am using Scrite and I thought you should check it out as well. Visit https://www.scrite.io"
                onClicked: Qt.openUrlExternally(url)
                ToolTip.text: "Send an email about Scrite."
            }
        }
    }
}
