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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

Item {
    readonly property bool modal: true
    readonly property string title: "Account Email"

    Image {
        anchors.fill: parent
        source: "qrc:/images/useraccountdialogbg.png"
        fillMode: Image.PreserveAspectCrop
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: 50
        anchors.leftMargin: 50
        anchors.rightMargin: 175
        anchors.bottomMargin: 50

        ColumnLayout {
            id: emailForm

            anchors.centerIn: parent

            width: parent.width
            spacing: 50
            enabled: !checkUserCall.busy
            opacity: enabled ? 1 : 0.5

            VclLabel {
                Layout.fillWidth: true

                color: Runtime.colors.accent.c400.background
                wrapMode: Text.WordWrap
                font.pointSize: emailField.font.pointSize
                horizontalAlignment: Text.AlignHCenter

                Component.onCompleted: {
                    const email = checkUserCall.email
                    if(email === "")
                        text = "Please provide your email-id to setup your Scrite profile."
                    else
                        text = "Please confirm your email-id to setup your Scrite installation."
                }
            }

            TextField {
                id: emailField

                Layout.fillWidth: true

                text: checkUserCall.email
                selectByMouse: true
                placeholderText: "Your Email ID"
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                horizontalAlignment: Text.AlignHCenter

                Keys.onReturnPressed: checkUserCall.check(text)
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 30

                VclButton {
                    text: "More Info"

                    // TODO:
                    onClicked: Qt.openUrlExternally("https://www.scrite.io")
                }

                Item {
                    Layout.fillWidth: true
                }

                VclButton {
                    id: submit

                    text: "Continue »"

                    onClicked: checkUserCall.check(emailField.text)

                    Connections {
                        target: emailField

                        function onTextChanged() {
                            Qt.callLater(submit.determineEnabled)
                        }
                    }

                    function determineEnabled() {
                        enabled = Utils.validateEmail(emailField.text)
                    }

                    Component.onCompleted: determineEnabled()
                }
            }
        }

        BusyIndicator {
            anchors.centerIn: parent

            running: checkUserCall.busy
        }
    }

    AppCheckUserRestApiCall {
        id: checkUserCall

        function check(_email) {
            if(Utils.validateEmail(_email)) {
                email = _email
                call()
            }
        }

        onFinished: {
            if(hasError || !hasResponse) {
                const errMsg = hasError ? errorMessage : "Error determining information about your account. Please try again."
                MessageBox.information("Error", errMsg)
                return
            }

            Session.set("checkUserResponse", userInfo)
            Announcement.shout(Runtime.announcementIds.userAccountDialogScreen, "JunctionScreen")
        }
    }    
}


