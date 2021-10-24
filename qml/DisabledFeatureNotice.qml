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
import Scrite 1.0

Rectangle {
    id: dfNotice
    property string reason: User.loggedIn ? "This feature is not enabled in your subscription." : ""
    property string suggestion: User.loggedIn ? "Please sign up for a subscription to use this feature." : "Login & activate your device to access this feature."
    property string featureName
    color: primaryColors.c100.background

    signal clicked()

    Column {
        anchors.centerIn: parent
        width: Math.max(parent.width * 0.8, 350)
        spacing: 20

        Text {
            text: featureName
            font.pointSize: app.idealFontPointSize + 8
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: reason
            width: parent.width
            font.pointSize: app.idealFontPointSize + 2
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            visible: text !== ""
        }

        Text {
            text: suggestion
            width: parent.width
            font.pointSize: app.idealFontPointSize
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            visible: text !== ""
        }

        Button2 {
            text: User.loggedIn ? "Subscribe" : "Login / Activate"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                dfNotice.clicked()
                Announcement.shout("97369507-721E-4A7F-886C-4CE09A5BCCFB", null)
            }
        }
    }
}
