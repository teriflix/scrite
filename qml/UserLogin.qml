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

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

Item {
    id: userLogin
    width: 32+20+10
    height: 32

    Image {
        id: profilePic
        property int counter: 0
        source: User.loggedIn ? "image://userIcon/me" + counter : "image://userIcon/default"
        x: 20
        height: parent.height
        width: parent.height
        smooth: true
        mipmap: true
        fillMode: Image.PreserveAspectFit
        transformOrigin: Item.Right
        ToolTip.text: User.loggedIn ? "Account Information" : "Login"

        BusyIndicator {
            visible: User.busy
            running: User.busy
            anchors.centerIn: parent
            onRunningChanged: parent.counter = parent.counter+1
        }

        MouseArea {
            hoverEnabled: true
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onEntered: parent.ToolTip.visible = true
            onExited: parent.ToolTip.visible = false
            enabled: appToolBar.visible
            onClicked: {
                modalDialog.sourceComponent = loginWizard
                modalDialog.popupSource = profilePic
                modalDialog.active = true
            }
        }
    }

    Announcement.onIncoming: {
        var stype = "" + type
        if(stype === "97369507-721E-4A7F-886C-4CE09A5BCCFB") {
            modalDialog.sourceComponent = loginWizard
            modalDialog.popupSource = profilePic
            modalDialog.active = true
        }
    }

    Component {
        id: loginWizard

        Item {
            width: 800
            height: 520

            Rectangle {
                id: titleBar
                color: "#65318f"
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 80

                Text {
                    anchors.centerIn: parent
                    font.pointSize: parent.height * 0.3
                    text: pageLoader.item.pageTitle
                    color: "white"
                }
            }

            Loader {
                id: pageLoader
                anchors.top: titleBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                property int page: -1

                sourceComponent: {
                    switch(page) {
                    case -1: return loginWizardPage0
                    case 0: return loginWizardPage1
                    case 1: return loginWizardPage2
                    default: break
                    }
                    return loginWizardPage3
                }

                Component.onCompleted: page = User.busy ? -1 : (User.loggedIn ? 2 : 0)

                Announcement.onIncoming: {
                    const stype = "" + type
                    if(stype === "93DC1133-58CA-4EDD-B803-82D9B6F2AA50")
                        page = page + data
                    else if(stype === "76281526-A16C-4414-8129-AD8770A17F16") {
                        active = false
                        Qt.callLater( function() { pageLoader.active = true } )
                    }
                }
            }
        }
    }

    Component {
        id: loginWizardPage0

        Item {
            property string pageTitle: "Account Information"

            BusyOverlay {
                anchors.fill: parent
                visible: true
                busyMessage: "Please wait ..."
            }

            Timer {
                running: !User.busy
                interval: 100
                onTriggered: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", User.loggedIn ? 3 : 1)
            }
        }
    }

    Component {
        id: loginWizardPage1

        Item {
            property string pageTitle: "Sign Up / Login"
            Component.onCompleted: modalDialog.closeable = true

            Item {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: nextButton.top

                Column {
                    width: parent.width*0.8
                    anchors.centerIn: parent
                    spacing: 40

                    TextField {
                        id: emailField
                        width: parent.width
                        placeholderText: "Email"
                        font.pointSize: app.idealFontPointSize + 2
                        text: sendActivationCodeCall.email()
                        validator: RegExpValidator {
                            regExp: /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
                        }
                        selectByMouse: true
                        horizontalAlignment: Text.AlignHCenter
                        Component.onCompleted: forceActiveFocus()
                    }

                    Text {
                        width: parent.width * 0.8
                        wrapMode: Text.WordWrap
                        font.pointSize: app.idealFontPointSize
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Sign up / login with your email to unlock Structure, Notebook and many more features in Scrite."
                    }
                }
            }

            Button2 {
                id: cancelButton
                text: "Cancel"
                anchors.left: parent.left
                anchors.verticalCenter: nextButton.verticalCenter
                anchors.leftMargin: 30
                onClicked: modalDialog.close()
            }

            Item {
                anchors.top: nextButton.top
                anchors.left: cancelButton.right
                anchors.right: nextButton.left
                anchors.bottom: nextButton.bottom
                anchors.leftMargin: 20
                anchors.rightMargin: 20

                Text {
                    width: parent.width
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                    font.pointSize: app.idealFontPointSize-2
                    maximumLineCount: 3
                    color: "red"
                    text: sendActivationCodeCall.hasError ? (sendActivationCodeCall.errorCode + ": " + sendActivationCodeCall.errorText) : ""
                }
            }

            Button2 {
                id: nextButton
                text: "Next »"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 30
                onClicked: {
                    sendActivationCodeCall.data = {
                        "email": emailField.text,
                        "request": "resendActivationCode"
                    }
                    sendActivationCodeCall.call()
                }
            }

            BusyOverlay {
                anchors.fill: parent
                visible: sendActivationCodeCall.busy
                busyMessage: "Please wait.."
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

                    store("email", emailField.text)
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", 1)
                }
                onBusyChanged: modalDialog.closeable = !busy
            }
        }
    }

    Component {
        id: loginWizardPage2

        Item {
            property string pageTitle: "Activate"
            Component.onCompleted: modalDialog.closeable = true

            Item {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: nextButton.top

                Column {
                    width: parent.width*0.8
                    anchors.centerIn: parent
                    spacing: 40

                    TextField {
                        id: activationCodeField
                        width: parent.width
                        placeholderText: "Paste the activation code here..."
                        font.pointSize: app.idealFontPointSize + 2
                        selectByMouse: true
                        horizontalAlignment: Text.AlignHCenter
                        Component.onCompleted: forceActiveFocus()
                    }

                    Text {
                        width: parent.width * 0.8
                        wrapMode: Text.WordWrap
                        font.pointSize: app.idealFontPointSize
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "We have emailed an activation code to <b>" + activateCall.fetch("email") + "</b>."
                    }
                }
            }

            Button2 {
                id: changeEmailButton
                text: "« Change Email"
                anchors.left: parent.left
                anchors.verticalCenter: nextButton.verticalCenter
                anchors.leftMargin: 30
                onClicked: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", -1)
            }

            Item {
                anchors.top: nextButton.top
                anchors.left: changeEmailButton.right
                anchors.right: nextButton.left
                anchors.bottom: nextButton.bottom
                anchors.leftMargin: 20
                anchors.rightMargin: 20

                Text {
                    width: parent.width
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                    font.pointSize: app.idealFontPointSize-2
                    maximumLineCount: 3
                    color: "red"
                    text: activateCall.hasError ? (activateCall.errorCode + ": " + activateCall.errorText) : ""
                }
            }

            Button2 {
                id: nextButton
                text: "Activate »"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 30
                onClicked: {
                    activateCall.data = {
                        "email": activateCall.email(),
                        "activationCode": activationCodeField.text,
                        "clientId": activateCall.clientId(),
                        "deviceId": activateCall.deviceId(),
                        "platform": activateCall.platform(),
                        "platformType": activateCall.platformType(),
                        "appVersion": activateCall.appVersion()
                    }
                    activateCall.call()
                }
            }

            BusyOverlay {
                anchors.fill: parent
                visible: activateCall.busy
                busyMessage: "Please wait.."
            }

            JsonHttpRequest {
                id: activateCall
                type: JsonHttpRequest.POST
                api: "app/activate"
                token: ""
                reportNetworkErrors: true
                onFinished: {
                    if(hasError || !hasResponse)
                        return

                    store("loginToken", responseData.loginToken)
                    store("sessionToken", responseData.sessionToken)
                    User.reload()
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", 1)
                }
                onBusyChanged: modalDialog.closeable = !busy
            }
        }
    }

    Component {
        id: loginWizardPage3

        Item {
            property string pageTitle: {
                if(User.loggedIn) {
                    if(User.info.firstName && User.info.firstName !== "")
                        return "Hi, " + User.info.firstName + "."
                    if(User.info.lastName && User.info.lastName !== "")
                        return "Hi, " + User.info.lastName + "."
                }
                return "Hi, there."
            }

            Component.onCompleted: modalDialog.closeable = Qt.binding( () => { return !needsSaving } )

            TabSequenceManager {
                id: userInfoFields
            }

            Item {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: leftSideLinks.bottom
                anchors.bottomMargin: Math.max(leftSideLinks.height, rightSideLinks.height)

                Column {
                    width: parent.width*0.8
                    anchors.centerIn: parent
                    spacing: 30

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pointSize: app.idealFontPointSize
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: User.loggedIn
                        bottomPadding: 20
                        text: "You're currently logged in via <b>" + User.info.email + "</b>."
                    }

                    Grid {
                        columns: 2
                        width: parent.width
                        rowSpacing: parent.spacing/2
                        columnSpacing: parent.spacing/2

                        TextField2 {
                            id: nameField
                            width: (parent.width-parent.columnSpacing)/2
                            placeholderText: "Name"
                            text: {
                                if(User.info.firstName && User.info.lastName)
                                    return User.info.firstName + " " + User.info.lastName
                                if(User.info.firstName)
                                    return User.info.firstName
                                return User.info.lastName ? User.info.lastName : ""
                            }
                            Component.onCompleted: forceActiveFocus()
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 0
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                        }

                        TextField2 {
                            id: experienceField
                            width: (parent.width-parent.columnSpacing)/2
                            text: User.info.experience
                            placeholderText: "Experience"
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 1
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            completionStrings: ["Novice", "Learning", "Written Few, None Made", "Have Produced Credits", "Experienced"]
                            minimumCompletionPrefixLength: 0
                        }

                        TextField2 {
                            id: cityField
                            width: (parent.width-parent.columnSpacing)/2
                            text: User.info.city
                            placeholderText: "City"
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 2
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            completionStrings: User.cityNames
                            minimumCompletionPrefixLength: 0
                            onEditingComplete: {
                                const countries = User.countries(text)
                                countryField.text = countries.length === 0 ? "" : countries[0]
                            }
                        }

                        TextField2 {
                            id: countryField
                            width: (parent.width-parent.columnSpacing)/2
                            text: User.info.country
                            placeholderText: "Country"
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 3
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            completionStrings: User.countryNames
                            minimumCompletionPrefixLength: 0
                        }

                        CheckBox2 {
                            id: chkAnalyticsConsent
                            checked: User.info.consent.activity
                            text: "Send analytics data."
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 4
                            onToggled: allowHighlightSaveAnimation = true
                        }

                        CheckBox2 {
                            id: chkEmailConsent
                            checked: User.info.consent.email
                            text: "Send marketing email."
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 5
                            onToggled: allowHighlightSaveAnimation = true
                        }
                    }
                }
            }

            property bool needsSaving: nameField.text.trim() !== (User.info.firstName + " " + User.info.lastName).trim() ||
                                       cityField.text.trim() !== User.info.city ||
                                       countryField.text.trim() !== User.info.country ||
                                       experienceField.text.trim() !== User.info.experience ||
                                       chkAnalyticsConsent.checked !== User.info.consent.activity ||
                                       chkEmailConsent.checked !== User.info.consent.email

            property bool allowHighlightSaveAnimation: false
            property bool animationFlags: needsSaving || allowHighlightSaveAnimation

            onAnimationFlagsChanged: Qt.callLater( function() {
                if(allowHighlightSaveAnimation)
                    highlightSaveAnimation.restart()
            })

            Column {
                id: leftSideLinks
                spacing: 10
                anchors.left: parent.left
                anchors.bottom: rightSideLinks.bottom
                anchors.leftMargin: 30

                Link {
                    text: needsSaving ? "Cancel" : "Logout"
                    opacity: needsSaving ? 0.75 : 1
                    onClicked: {
                        if(needsSaving) {
                            Announcement.shout("76281526-A16C-4414-8129-AD8770A17F16", undefined)
                        } else {
                            User.logout()
                            Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", -2)
                        }
                    }
                }

                Link {
                    text: "Feedback / About"
                    opacity: needsSaving ? 0.75 : 1
                    onClicked: {
                        modalDialog.close()
                        var time = 100
                        if(modalDialog.animationsEnabled)
                            time += modalDialog.animationDuration
                        Announcement.shout("72892ED6-BA58-47EC-B045-E92D9EC1C47A", time)
                    }
                }
            }

            Item {
                anchors.top: rightSideLinks.top
                anchors.left: leftSideLinks.right
                anchors.right: rightSideLinks.left
                anchors.bottom: rightSideLinks.bottom
                anchors.leftMargin: 20
                anchors.rightMargin: 20

                Text {
                    width: parent.width
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                    font.pointSize: app.idealFontPointSize-2
                    maximumLineCount: 3
                    color: "red"
                    text: userError.hasError ? userError.details.code + ": " + userError.details.message : ""
                    property ErrorReport userError: Aggregation.findErrorReport(User)
                }
            }

            Column {
                id: rightSideLinks
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 30
                spacing: 10

                Link {
                    id: saveRefreshLink
                    text: needsSaving ? "Save" : "Refresh"
                    transformOrigin: Item.BottomRight
                    anchors.right: parent.right
                    property real characterSpacing: 0
                    font.letterSpacing: characterSpacing
                    onClicked: {
                        if(needsSaving) {
                            const names = nameField.text.split(' ')
                            const newInfo = {
                                firstName: names.length > 0 ? names[0] : "",
                                lastName: names.length > 1 ? names[names.length-1] : "",
                                experience: experienceField.text,
                                city: cityField.text,
                                country: countryField.text,
                                consent: {
                                    activity: chkAnalyticsConsent.checked,
                                    email: chkEmailConsent.checked
                                }
                            }
                            allowHighlightSaveAnimation = false
                            User.update(newInfo)
                        } else
                            User.reload()
                    }

                    Connections {
                        target: User
                        onInfoChanged: saveRefreshLink.restore()
                    }

                    function restore() {
                        saveRefreshLink.font.bold = needsSaving
                        saveRefreshLink.font.pointSize = app.idealFontPointSize + (needsSaving ? 3 : 0)
                    }

                    SequentialAnimation {
                        id: highlightSaveAnimation
                        loops: 1
                        running: false

                        ParallelAnimation {
                            NumberAnimation {
                                target: saveRefreshLink
                                property: "characterSpacing"
                                to: 2.5
                                duration: 350
                            }
                            NumberAnimation {
                                target: saveRefreshLink
                                property: "opacity"
                                to: 0.3
                                duration: 350
                            }
                        }

                        PauseAnimation {
                            duration: 100
                        }

                        ParallelAnimation {
                            NumberAnimation {
                                target: saveRefreshLink
                                property: "characterSpacing"
                                to: 0
                                duration: 250
                            }
                            NumberAnimation {
                                target: saveRefreshLink
                                property: "opacity"
                                to: 1
                                duration: 250
                            }
                        }

                        ScriptAction {
                            script: saveRefreshLink.restore()
                        }
                    }
                }

                Link {
                    text: "Privacy Policy"
                    opacity: needsSaving ? 0.75 : 1
                    anchors.right: parent.right
                    onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/privacy-policy/")
                }
            }

            BusyOverlay {
                anchors.fill: parent
                visible: User.busy
                busyMessage: "Please wait.."
                onVisibleChanged: modalDialog.closeable = !visible
            }
        }
    }

    property ErrorReport userErrorReport: Aggregation.findErrorReport(User)
    Notification.active: userErrorReport.hasError
    Notification.title: "User Account / Device Activation Error"
    Notification.text: userErrorReport.details.code + ": " + userErrorReport.errorMessage
    Notification.autoClose: false
    Notification.onDismissed: userErrorReport.clear()
}
