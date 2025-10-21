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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    readonly property bool modal: true
    readonly property string title: "Welcome to Scrite!"

    Image {
        anchors.fill: parent
        source: "qrc:/images/useraccountdialogbg.png"
        fillMode: Image.PreserveAspectCrop
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 50
        anchors.leftMargin: 50
        anchors.rightMargin: 175
        anchors.bottomMargin: 50

        spacing: 40

        Flickable {
            id: flick

            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: VclScrollBar { }

            clip: ScrollBar.vertical.needed
            contentWidth: label.width
            contentHeight: label.height

            VclLabel {
                id: label

                width: flick.width - (flick.clip ? 20 : 0)

                text: "Thank you for choosing Scrite as the place to bring your screenplays to life. We’re thrilled to have you on board and can’t wait to celebrate the stories you create with our powerful and intuitive writing tools.\n\n" +
                      "In the next few screens, we’ll guide you through the simple process of logging in to your account and setting up your personalized copy of Scrite. This will only take a few moments, and by the end, you’ll be ready to dive right into your screenplay creation.\n\n" +
                      "Please click Next to continue and get started on your writing journey. We’re here to make your experience as smooth and enjoyable as possible."
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
                font.pointSize: Runtime.idealFontMetrics.font.pointSize+2
            }
        }

        VclButton {
            Layout.alignment: Qt.AlignRight

            text: "Next »"

            onClicked: {
                Runtime.userAccountDialogSettings.welcomeScreenShown = true
                Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "AccountEmailScreen")
            }
        }
    }
}
