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
    readonly property string title: "Setup your Scrite account"

    Component.onCompleted: Qt.callLater(emailField.forceActiveFocus)

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

        TabSequenceManager {
            id: userInfoFields
        }

        ColumnLayout {
            anchors.centerIn: parent

            width: parent.width
            spacing: 20
            enabled: !checkUserCall.isBusy
            opacity: enabled ? 1 : 0.5

            VclLabel {
                Layout.fillWidth: true

                text: "Please provide us your email."
                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                wrapMode: Text.WordWrap
            }

            VclLabel {
                Layout.fillWidth: true

                text: "Your free trial and other subscription plans will be linked to this email."
                wrapMode: Text.WordWrap
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }

            VclTextField {
                id: emailField

                Layout.fillWidth: true

                TabSequenceItem.manager: userInfoFields
                TabSequenceItem.sequence: 0

                text: checkUserCall.email
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                maximumLength: 128
                selectByMouse: true
                undoRedoEnabled: true

                onReturnPressed: if(submit.enabled) submit.clicked()
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }

            VclButton {
                id: submit

                Component.onCompleted: determineEnabled()
                Layout.alignment: Qt.AlignRight

                function determineEnabled() {
                    enabled = Runtime.validateEmail(emailField.text.trim())
                }

                text: "Continue »"

                Connections {
                    target: emailField

                    function onTextChanged() {
                        Qt.callLater(submit.determineEnabled)
                    }
                }

                onClicked: checkUserCall.check()
            }
        }

        BusyIndicator {
            anchors.centerIn: parent

            running: checkUserCall.busy
        }
    }

    AppCheckUserRestApiCall {
        id: checkUserCall

        function check() {
            const _email = emailField.text.trim()
            if(Runtime.validateEmail(_email)) {
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
            Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "JunctionScreen")
        }
    }
}


