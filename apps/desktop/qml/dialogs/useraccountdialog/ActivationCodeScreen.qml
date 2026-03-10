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

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../"
import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    readonly property bool modal: true
    readonly property string title: "Activation"
    readonly property bool checkForRestartRequest: false
    readonly property bool checkForSessionStatus: false

    Component.onCompleted: Qt.callLater(_activationCodeField.forceActiveFocus)

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
            id: _activationFormLayout

            anchors.centerIn: parent

            width: parent.width
            spacing: 50
            enabled: !_activateCall.busy && !_sendActivationCodeCall.busy
            opacity: enabled ? 1 : 0.5

            VclLabel {
                Layout.fillWidth: true

                text: "A verification code was sent to <b>" + _private.userMeta.email + "</b>. Please paste it in the text field below, and click Verify."
                wrapMode: Text.WordWrap
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            }

            TextField {
                id: _activationCodeField

                Layout.fillWidth: true

                DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                placeholderText: "Verification Code"
                horizontalAlignment: Text.AlignHCenter

                Keys.onReturnPressed: if(_activateButton.enabled) _activateButton.clicked()
            }

            RowLayout {
                Layout.fillWidth: true

                VclButton {
                    text: "Resend" + (_resendTimer.running ? " (" + _resendTimer.secondsLeft + ")" : "")
                    enabled: !_resendTimer.running

                    onClicked: _sendActivationCodeCall.call()

                    Timer {
                        id: _resendTimer

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

                        visible: _clipboard.text.length === 20

                        text: "Paste"

                        onClicked: {
                            _activationCodeField.text = _clipboard.text
                            _clipboard.text = ""
                        }
                    }
                }

                VclButton {
                    id: _activateButton

                    text: "Verify »"
                    enabled: _activationCodeField.text.length == 20

                    onClicked: _activateCall.call()
                }
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: _activateCall.busy || _sendActivationCodeCall.busy
        }
    }

    SystemClipboard {
        id: _clipboard
    }

    AppActivateDeviceRestApiCall {
        id: _activateCall

        activationCode: _activationCodeField.text.trim()

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
        id: _sendActivationCodeCall
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
                                        _resendTimer.secondsLeft = 30
                                        _resendTimer.start()
                                   })
        }
    }

    QtObject {
        id: _private

        readonly property var userMeta: Session.get("checkUserResponse")
    }
}
