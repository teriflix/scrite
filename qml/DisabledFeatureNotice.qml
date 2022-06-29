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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Rectangle {
    id: dfNotice
    property string reason: Scrite.user.loggedIn ? privateData.loggedInReason : privateData.loggedOutReason
    property string suggestion: Scrite.user.loggedIn ? privateData.loggedInSuggestion : privateData.loggedOutSuggestion
    property string featureName
    color: primaryColors.c100.background
    clip: true

    signal clicked()

    QtObject {
        id: privateData

        readonly property string loggedInReason: "This feature is not available in your subscription plan."
        readonly property string loggedInSuggestion: "Please review and upgrade your subscription plan."

        readonly property string loggedOutReason: "Some features of Scrite are user profile linked and require you to be logged in to access it."
        readonly property string loggedOutSuggestion: "Please sign-up/login to continue."
    }

    Flickable {
        id: contentsFlick
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 350)
        height: Math.min(contents.height, parent.height)
        contentWidth: width
        contentHeight: contents.height
        ScrollBar.vertical: vscrollBar

        Column {
            id: contents
            spacing: 20
            width: contentsFlick.width

            Item { width: parent.width; height: 30 }

            Image {
                id: icon
                source: "../images/feature_locked.png"
                width: 64; height: 64
                asynchronous: false
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: featureName
                font.pointSize: Scrite.app.idealFontPointSize + 8
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                visible: text !== ""
            }

            Text {
                id: reasonSuggestion
                text: [reason, suggestion].join(" ").trim()
                width: parent.width
                font.pointSize: Scrite.app.idealFontPointSize
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                visible: text !== ""
            }

            Link {
                id: link
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.underline: false
                text: "<u>Click here</u> to know more."
                onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/login-and-activation/")
            }

            Button2 {
                text: Scrite.user.loggedIn ? "Subscribe" : "Sign Up / Login"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    dfNotice.clicked()
                    Announcement.shout("97369507-721E-4A7F-886C-4CE09A5BCCFB", null)
                }
            }

            Item { width: parent.width; height: 30 }
        }
    }

    ScrollBar2 {
        id: vscrollBar
        flickable: contentsFlick
        orientation: Qt.Vertical
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }

}
