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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    readonly property bool modal: true
    readonly property string title: "Welcome to Scrite!"

    Image {
        anchors.fill: parent
        source: "qrc:/images/useraccountdialogbg.png"
        fillMode: Image.PreserveAspectCrop
        opacity: 0.25
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: 50
        anchors.leftMargin: 50
        anchors.rightMargin: 175
        anchors.bottomMargin: 50

        ColumnLayout {
            anchors.centerIn: parent

            width: parent.width * 0.8

            spacing: 20

            VclLabel {
                Layout.fillWidth: true

                text: sendActivationCodeCall.busy ? "Requesting activation code ..." : "Click the button below to request activation code."
            }

            VclButton {
                visible: !sendActivationCodeCall.hasError && !sendActivationCodeCall.busy

                text: "Request Activation Code"

                onClicked: sendActivationCodeCall.call()
            }
        }

        Component.onCompleted: sendActivationCodeCall.call()
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: sendActivationCodeCall.busy
    }

    AppRequestActivationCodeRestApiCall {
        id: sendActivationCodeCall
        onFinished: {
            if(hasError || !hasResponse) {
                const errMsg = hasError ? errorMessage : "Couldn't request activation code. Please try again."
                MessageBox.information("Error", errMsg)
                return
            }

            Announcement.shout(Runtime.announcementIds.userAccountDialogScreen, "ActivationCodeScreen")
        }
    }

    QtObject {
        id: _private

        readonly property var userMeta: Session.get("checkUserResponse")
    }
}
