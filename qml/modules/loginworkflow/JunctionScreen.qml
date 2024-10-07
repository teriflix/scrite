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
    readonly property bool checkForRestartRequest: false
    readonly property bool checkForUserProfileErrors: false

    Image {
        anchors.fill: parent
        source: "qrc:/images/loginworkflowbg.png"
        fillMode: Image.PreserveAspectCrop
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: 50
        anchors.leftMargin: 50
        anchors.rightMargin: 175
        anchors.bottomMargin: 50

        ColumnLayout {
            anchors.fill: parent

            spacing: 40
            enabled: !sendActivationCodeCall.busy
            opacity: enabled ? 1 : 0.5

            Flickable {
                id: textAreaFlick
                Layout.fillWidth: true
                Layout.fillHeight: true

                contentWidth: contentHeight > height ? width - 20 : width
                contentHeight: userMessage.contentHeight

                ScrollBar.vertical: VclScrollBar { flickable: textAreaFlick }

                VclLabel {
                    id: userMessage

                    readonly property var userMeta: Session.get("userMeta")

                    width: textAreaFlick.contentWidth
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize+2

                    text: userMeta.message
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            }

            VclButton {
                Layout.alignment: Qt.AlignRight

                text: userMessage.userMeta && userMessage.userMeta.allowUse ? "Continue »" : "View Plans"
                onClicked: {
                    if(userMessage.userMeta && userMessage.userMeta.allowUse) {
                        sendActivationCodeCall.data = {
                            "email": Session.get("email"),
                            "request": "resendActivationCode"
                        }
                        sendActivationCodeCall.call()
                    } else {
                        Announcement.shout(Runtime.announcementIds.loginWorkflowScreen, "SubscriptionPlansScreen")
                    }
                }
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: sendActivationCodeCall.busy
        }
    }

    JsonHttpRequest {
        id: sendActivationCodeCall
        type: JsonHttpRequest.POST
        api: "app/activate"
        token: ""
        reportNetworkErrors: true
        onFinished: {
            if(hasError || !hasResponse)
                return

            Announcement.shout(Runtime.announcementIds.loginWorkflowScreen, "ActivationCodeScreen")
        }
    }
}
