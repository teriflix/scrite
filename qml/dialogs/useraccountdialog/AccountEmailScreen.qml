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
    readonly property string title: "Setup your Scrite login"

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

                text: "Please provide us the following information to setup your Scrite login."
                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
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
                placeholderText: "Email *"
            }

            VclTextField {
                id: nameField

                Layout.fillWidth: true

                TabSequenceItem.manager: userInfoFields
                TabSequenceItem.sequence: 1

                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                maximumLength: 128
                selectByMouse: true
                undoRedoEnabled: true
                placeholderText: "Name"
            }

            VclTextField {
                id: experienceField

                Layout.fillWidth: true

                TabSequenceItem.manager: userInfoFields
                TabSequenceItem.sequence: 2

                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                maximumLength: 128
                selectByMouse: true
                undoRedoEnabled: true
                placeholderText: "Experience"

                completionStrings: [
                    "Hobby Writer",
                    "Actively Pursuing a Writing Career",
                    "Working Writer",
                    "Have Produced Credits"
                ]
                minimumCompletionPrefixLength: 0
            }

            VclTextField {
                id: wdyhasField

                Layout.fillWidth: true

                TabSequenceItem.manager: userInfoFields
                TabSequenceItem.sequence: 3

                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                maximumLength: 128
                selectByMouse: true
                undoRedoEnabled: true
                placeholderText: "Where did you hear about Scrite?"

                completionStrings: [
                    "From another Scrite User",
                    "I am already a Scrite User",
                    "Google Search",
                    "YouTube",
                    "Film School",
                    "Film Workshop",
                    "Twitter",
                    "Recommended by a friend",
                    "Instagram",
                    "Reddit",
                    "LinkedIn",
                    "Facebook",
                ]
                maxVisibleItems: 11
                minimumCompletionPrefixLength: 0
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }

            VclButton {
                id: submit

                Layout.alignment: Qt.AlignRight

                text: "Continue »"

                onClicked: checkUserCall.check()

                Connections {
                    target: emailField

                    function onTextChanged() {
                        Qt.callLater(submit.determineEnabled)
                    }
                }

                function determineEnabled() {
                    enabled = Utils.validateEmail(emailField.text.trim())
                }

                Component.onCompleted: determineEnabled()
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
            if(Utils.validateEmail(_email)) {
                email = _email

                let nameComps = nameField.text.trim().split(" ")
                if(nameComps.length >= 1) {
                    firstName = nameComps[0]
                    nameComps.shift()
                }

                lastName = nameComps.join(" ")
                experience = experienceField.text.trim()
                wdyhas = wdyhasField.text.trim()

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


