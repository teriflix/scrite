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

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

Rectangle {
    id: dfNotice
    property string reason: User.loggedIn ? "This feature is not enabled in your subscription." : "Some features are accessible only after you sign-up or login."
    property string suggestion: User.loggedIn ? "Please opt-in for a subscription to use this feature." : "Sign up / login with your email to unlock this feature."
    property string featureName
    color: primaryColors.c100.background
    clip: true

    signal clicked()


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
                text: "Unlock '" + featureName + "'"
                font.pointSize: app.idealFontPointSize + 8
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                visible: text !== ""
            }

            Text {
                id: reasonSuggestion
                text: [reason, suggestion].join(" ").trim()
                width: parent.width
                font.pointSize: app.idealFontPointSize
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
                text: User.loggedIn ? "Subscribe" : "Sign Up / Login"
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
