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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../"
import "../../tasks"
import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    property var userInfo: Scrite.user.info
    property RestApiCallList callList : RestApiCallList {
        calls: [_refreshUserCall, _deactivateDeviceCall, _saveUserCall]
    }

    TabSequenceManager {
        id: _userInfoFields

        property bool needsSaving: false
    }

    Connections {
        target: Scrite.user

        function onBusyChanged() { _userInfoFields.needsSaving = false }
    }

    ColumnLayout {
        id: _layout

        anchors.fill: parent
        anchors.margins: 20
        anchors.leftMargin: 0

        spacing: 20
        opacity: enabled ? 1 : 0.5
        enabled: !root.callList.busy

        VclLabel {
            Layout.fillWidth: true

            text: "You're logged in via <b>" + root.userInfo.email + "</b>."
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.fillWidth: true

            VclTextField {
                id: _nameField

                Layout.fillWidth: true

                TabSequenceItem.manager: _userInfoFields
                TabSequenceItem.sequence: 0

                text: root.userInfo.fullName
                maximumLength: 128
                placeholderText: "Name"
                undoRedoEnabled: true

                onTextEdited: _userInfoFields.needsSaving = true
            }

            VclTextField {
                id: _phoneField

                Layout.fillWidth: true

                TabSequenceItem.manager: _userInfoFields
                TabSequenceItem.sequence: 1

                text: root.userInfo.phone
                maximumLength: 128
                placeholderText: "Phone (optional)"
                undoRedoEnabled: true

                onTextEdited: _userInfoFields.needsSaving = true

                validator: RegularExpressionValidator {
                    regularExpression: /^\+?(\d{1,3})?[\s\-]?\(?\d{1,4}\)?[\s\-]?\d{1,4}[\s\-]?\d{1,4}$/
                }
            }
        }


        VclTextField {
            id: _experienceField

            Layout.fillWidth: true

            TabSequenceItem.manager: _userInfoFields
            TabSequenceItem.sequence: 2

            text: root.userInfo.experience
            maximumLength: 128
            maxVisibleItems: 6
            placeholderText: "Experience (optional)"
            undoRedoEnabled: true
            completionStrings: [
                "Hobby Writer",
                "Actively Pursuing a Writing Career",
                "Working Writer",
                "Have Produced Credits"
            ]
            maxCompletionItems: -1
            minimumCompletionPrefixLength: 0

            onTextEdited: _userInfoFields.needsSaving = true
        }

        RowLayout {
            Layout.fillWidth: true

            VclTextField {
                id: _cityField

                Layout.fillWidth: true

                TabSequenceItem.manager: _userInfoFields
                TabSequenceItem.sequence: 3

                text: root.userInfo.city
                maximumLength: 128
                placeholderText: "City"
                undoRedoEnabled: true

                onTextEdited: _userInfoFields.needsSaving = true
            }

            VclTextField {
                id: _countryField

                Layout.fillWidth: true

                placeholderText: "Country"
                text: root.userInfo.country
                readOnly: true
            }
        }

        VclTextField {
            id: _wdyhasField

            Layout.fillWidth: true

            TabSequenceItem.manager: _userInfoFields
            TabSequenceItem.sequence: 4

            text: root.userInfo.wdyhas
            placeholderText: "Where did you hear about Scrite? (optional)"
            maximumLength: 128
            completionStrings: [
                "Facebook",
                "Reddit",
                "YouTube",
                "Film School",
                "Film Workshop",
                "Instagram",
                "Recommended by Friend",
                "Existing Scrite User",
                "LinkedIn",
                "Twitter",
                "Google Search"
            ]
            minimumCompletionPrefixLength: 0
            maxCompletionItems: -1
            maxVisibleItems: 6
            undoRedoEnabled: true

            onTextEdited: _userInfoFields.needsSaving = true
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 25

            VclCheckBox {
                id: _chkAnalyticsConsent

                TabSequenceItem.manager: _userInfoFields
                TabSequenceItem.sequence: 5

                text: "Send analytics data."
                checked: root.userInfo.consentToActivityLog
                padding: 0

                onToggled: _userInfoFields.needsSaving = true
            }

            VclCheckBox {
                id: _chkEmailConsent

                TabSequenceItem.manager: _userInfoFields
                TabSequenceItem.sequence: 6

                text: "Send marketing email."
                checked: root.userInfo.consentToEmail
                padding: 0

                onToggled: _userInfoFields.needsSaving = true
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        StackLayout {
            currentIndex: _userInfoFields.needsSaving ? 1 : 0

            Layout.fillWidth: true

            RowLayout {
                Layout.fillWidth: true

                spacing: 20

                VclButton {
                    text: "Refresh"
                    onClicked: _refreshUserCall.call()

                    UserMeRestApiCall {
                        id: _refreshUserCall
                    }
                }

                VclButton {
                    text: "Survey"
                    visible: ["recommended", "required"].indexOf(Runtime.userAccountDialogSettings.userOnboardingStatus) >= 0

                    onClicked: UserOnboardingDialog.launch()
                }

                Item {
                    Layout.fillWidth: true
                }

                VclButton {
                    text: "Logout"

                    onClicked: {
                        SaveFileTask.save( () => {
                                              Scrite.document.reset()
                                              _deactivateDeviceCall.call()
                                          })

                    }

                    InstallationDeactivateRestApiCall {
                        id: _deactivateDeviceCall

                        onFinished: {
                            if(!hasError)
                                Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "AccountEmailScreen")
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 20

                Item {
                    Layout.fillWidth: true
                }

                VclButton {
                    text: "Save"

                    onClicked: {
                        var names = _nameField.text.trim().split(' ')

                        const _lastName = names.length > 1 ? names[names.length-1] : ""
                        if(names.length > 1)
                            names.pop()
                        const _firstName = names.join(" ")
                        const locale = Scrite.locale

                        const newInfo = {
                            firstName: _firstName,
                            lastName: _lastName,
                            experience: _experienceField.text.trim(),
                            phone: _phoneField.text.trim(),
                            city: _cityField.text.trim(),
                            country: locale.country.name,
                            currency: locale.currency.code,
                            wdyhas: _wdyhasField.text.trim(),
                            consentToActivityLog: _chkAnalyticsConsent.checked,
                            consentToEmail: _chkEmailConsent.checked,
                        }

                        _saveUserCall.updatedFields = newInfo
                        _saveUserCall.call()
                    }

                    UserMeRestApiCall {
                        id: _saveUserCall

                        onFinished: {
                            updatedFields = {}
                            _userInfoFields.needsSaving = false
                        }
                    }
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent

        running: root.callList.busy
    }
}
