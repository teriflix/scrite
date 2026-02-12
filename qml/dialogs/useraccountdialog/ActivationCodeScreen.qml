/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

Item {
    readonly property bool modal: true
    readonly property string title: "Activation"
    readonly property bool checkForRestartRequest: false
    readonly property bool checkForSessionStatus: false

    Component.onCompleted: Qt.callLater(activationCodeField.forceActiveFocus)

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
            id: activationForm

            anchors.centerIn: parent

            width: parent.width
            spacing: 50
            enabled: !activateCall.busy && !sendActivationCodeCall.busy
            opacity: enabled ? 1 : 0.5

            VclLabel {
                Layout.fillWidth: true

                text: "A verification code was sent to <b>" + _private.userMeta.email + "</b>. Please paste it in the text field below, and click Verify."
                wrapMode: Text.WordWrap
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            }

            TextField {
                id: activationCodeField

                Layout.fillWidth: true

                DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                placeholderText: "Verification Code"
                horizontalAlignment: Text.AlignHCenter

                Keys.onReturnPressed: if(activateButton.enabled) activateButton.clicked()
            }

            RowLayout {
                Layout.fillWidth: true

                VclButton {
                    text: "Resend" + (resendTimer.running ? " (" + resendTimer.secondsLeft + ")" : "")
                    enabled: !resendTimer.running

                    onClicked: sendActivationCodeCall.call()

                    Timer {
                        id: resendTimer

                        property int secondsLeft: 30

                        repeat: true
                        running: true
                        interval: 1000

                        onTriggered: {
                            secondsLeft = secondsLeft-1
                            if(secondsLeft <= 0) {
                                stop()
                                secondsLeft = 0
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true

                    VclButton {
                        anchors.centerIn: parent

                        visible: clipboard.text.length === 20

                        text: "Paste"

                        onClicked: {
                            activationCodeField.text = clipboard.text
                            clipboard.text = ""
                        }
                    }
                }

                VclButton {
                    id: activateButton

                    text: "Verify »"
                    enabled: activationCodeField.text.length == 20

                    onClicked: activateCall.call()
                }
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: activateCall.busy || sendActivationCodeCall.busy
        }
    }

    SystemClipboard {
        id: clipboard
    }

    AppActivateDeviceRestApiCall {
        id: activateCall

        activationCode: activationCodeField.text.trim()

        onFinished: {
            if(hasError) {
                const faq = _private.userMeta.urls.faq_device_limit
                MessageBox.information("Error", errorMessage, () => {
                                           if(faq)
                                            Qt.openUrlExternally(faq)
                                       })
                return
            }

            if(!hasResponse) {
                MessageBox.information("Error", "No respone received from the server.")
                return
            }

            Runtime.userAccountDialogSettings.userOnboardingStatus = _private.userMeta.onboarding
            Session.unset("checkUserResponse")
            Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "ReloadUserScreen")
        }
    }

    AppRequestActivationCodeRestApiCall {
        id: sendActivationCodeCall
        onFinished: {
            if(hasError) {
                MessageBox.information("Error", errorMessage)
                return
            }

            if(!hasResponse) {
                MessageBox.information("Error", "No respone received from the server.")
                return
            }

            MessageBox.information("Verification Code", responseText, () => {
                                        resendTimer.secondsLeft = 30
                                        resendTimer.start()
                                   })
        }
    }

    QtObject {
        id: _private

        readonly property var userMeta: Session.get("checkUserResponse")
    }
}
