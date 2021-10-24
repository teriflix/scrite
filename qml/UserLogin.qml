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
            enabled: appToolBar.visible && !User.busy
            onClicked: {
                if(User.loggedIn)
                    modalDialog.sourceComponent = userProfileDialog
                else
                    modalDialog.sourceComponent = userLoginDialog
                modalDialog.popupSource = profilePic
                modalDialog.active = true
            }
        }
    }

    Announcement.onIncoming: {
        var stype = "" + type
        if(stype === "97369507-721E-4A7F-886C-4CE09A5BCCFB") {
            if(User.loggedIn)
                showAccountProfileDialog()
            else
                showLoginDialog()
        }
    }

    function showLoginDialog() {
        modalDialog.sourceComponent = userLoginDialog
        modalDialog.popupSource = profilePic
        modalDialog.active = true
    }

    function showAccountProfileDialog() {
        modalDialog.sourceComponent = userProfileDialog
        modalDialog.popupSource = profilePic
        modalDialog.active = true
    }

    Component {
        id: userLoginDialog

        Item {
            width: 500
            height: 400
            property bool activationCodeSent: false
            property string activationEmail: ""

            Component.onCompleted: modalDialog.closeable = false
            Component.onDestruction: modalDialog.closeable = true

            TabSequenceManager {
                id: loginFieldsTabManager
            }

            Column {
                width: parent.width*0.8
                spacing: 20
                anchors.centerIn: parent

                Label {
                    font.pointSize: app.idealFontPointSize + 4
                    text: "Login & Activate"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                TextField {
                    id: emailField
                    width: parent.width
                    placeholderText: "Email"
                    text: activateHttpRequest.email()
                    validator: RegExpValidator {
                        regExp: /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
                    }
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    TabSequenceItem.manager: loginFieldsTabManager
                    TabSequenceItem.sequence: 0
                    onTextEdited: {
                        if(activationEmail !== "")
                            activationCodeSent = activationEmail === text
                    }
                }

                Column {
                    width: parent.width
                    spacing: parent.spacing/3
                    visible: activationCodeSent

                    Label {
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: "An activation code has been sent to your email."
                        horizontalAlignment: Text.AlignHCenter
                    }

                    TextField {
                        id: activationCodeField
                        width: parent.width
                        placeholderText: "Paste the activation code here."
                        selectByMouse: true
                        horizontalAlignment: Text.AlignHCenter
                        TabSequenceItem.manager: loginFieldsTabManager
                        TabSequenceItem.sequence: 1
                    }
                }

                Item { width: parent.width; height: 20 }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Button2 {
                        text: activationCodeSent ? "Activate" : "Send Activation Code"
                        enabled: activationCodeSent ? activationCodeField.length > 0 : emailField.acceptableInput
                        onClicked: {
                            if(activationCodeSent) {
                                activateHttpRequest.data = {
                                    email: activationEmail,
                                    activationCode: activationCodeField.text,
                                    clientId: activateHttpRequest.clientId(),
                                    deviceId: activateHttpRequest.deviceId(),
                                    platform: activateHttpRequest.platform(),
                                    platformType: activateHttpRequest.platformType(),
                                    appVersion: activateHttpRequest.appVersion()
                                }
                            } else {
                                activateHttpRequest.data = {
                                    email: emailField.text,
                                    request: "resendActivationCode"
                                }
                            }

                            activateHttpRequest.call()
                        }
                    }

                    Button2 {
                        text: "Cancel"
                        onClicked: modalDialog.closeRequest()
                    }
                }
            }

            Label {
                id: statusText
                width: parent.width*0.8
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 10
                color: "red"
                text: activateHttpRequest.hasError ? (activateHttpRequest.errorCode + ":" + activateHttpRequest.errorText) : ""
            }

            BusyOverlay {
                anchors.fill: parent
                visible: activateHttpRequest.busy
                busyMessage: "Please wait.."
            }

            JsonHttpRequest {
                id: activateHttpRequest
                type: JsonHttpRequest.POST
                api: "app/activate"
                token: ""
                reportNetworkErrors: true
                onFinished: {
                    if(hasError || !hasResponse)
                        return

                    if(activationCodeSent) {
                        activateHttpRequest.store("email", activationEmail)
                        activateHttpRequest.store("loginToken", activateHttpRequest.responseData.loginToken)
                        activateHttpRequest.store("sessionToken", activateHttpRequest.responseData.sessionToken)
                        User.reload()
                        app.execLater(userLogin, 500, showAccountProfileDialog)
                        modalDialog.close()
                    } else {
                        activationEmail = data.email
                        activationCodeSent = true
                    }
                }
            }
        }
    }

    Component {
        id: userProfileDialog

        Item {
            width: 400
            height: Math.min(ui.height*0.8,600)

            Component.onDestruction: modalDialog.closeable = true

            TabSequenceManager {
                id: userProfileTabManager
            }

            Text {
                id: titleText
                font.pointSize: Screen.devicePixelRatio > 1 ? 24 : 20
                font.bold: true
                text: "Account Information"
                anchors.top: parent.top
                anchors.topMargin: Math.max(20, parent.height-userInfoFlickable.contentHeight-footerRow.height-80-height)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Flickable {
                id: userInfoFlickable
                anchors.top: titleText.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: footerRow.top
                anchors.margins: 20
                clip: true
                contentWidth: userInfoLayout.width
                contentHeight: userInfoLayout.height
                ScrollBar.vertical: ScrollBar2 {
                    flickable: userInfoFlickable
                }

                Column {
                    id: userInfoLayout
                    width: userInfoFlickable.width
                    spacing: 10

                    TextField {
                        readOnly: true
                        width: parent.width
                        text: User.info.email
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 0
                    }

                    TextField2 {
                        id: firstNameField
                        placeholderText: "First Name"
                        width: parent.width
                        text: User.info.firstName
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 1
                    }

                    TextField2 {
                        id: lastNameField
                        placeholderText: "Last Name"
                        width: parent.width
                        text: User.info.lastName
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 2
                    }

                    TextField2 {
                        id: experienceField
                        width: parent.width
                        text: User.info.experience
                        placeholderText: "Experience"
                        completionStrings: ["Novice", "Film School Student", "Wannabe Screenwriter", "Professional Screenwriter"]
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 3
                    }

                    TextField2 {
                        id: cityField
                        placeholderText: "City"
                        width: parent.width
                        text: User.info.city
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 4
                    }

                    TextField2 {
                        id: stateField
                        placeholderText: "State"
                        width: parent.width
                        text: User.info.state
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 5
                    }

                    TextField2 {
                        id: countryField
                        placeholderText: "Country"
                        width: parent.width
                        text: User.info.country
                        TabSequenceItem.manager: userProfileTabManager
                        TabSequenceItem.sequence: 6
                    }
                }
            }

            Column {
                id: footerRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 20
                spacing: 10

                Row {
                    id: buttonsRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    visible: firstNameField.text !== User.info.firstName ||
                             lastNameField.text !== User.info.lastName ||
                             experienceField.text !== User.info.experience ||
                             cityField.text !== User.info.city ||
                             stateField.text !== User.info.state ||
                             countryField.text !== User.info.country

                    Button2 {
                        text: "Apply"
                        onClicked: {
                            userInfoHttpRequest.data = {
                                firstName: firstNameField.text,
                                lastName: lastNameField.text,
                                experience: experienceField.text,
                                city: cityField.text,
                                state: stateField.text,
                                country: countryField.text
                            }
                            userInfoHttpRequest.call()
                        }
                    }

                    Button2 {
                        text: "Cancel"
                        onClicked: modalDialog.close()
                    }
                }

                Row {
                    spacing: 20
                    visible: !buttonsRow.visible
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button2 {
                        text: "Refresh"
                        onClicked: User.reload()
                    }

                    Button2 {
                        text: "Deactivate"
                        onClicked: deactivateHttpRequest.call()
                    }
                }

                Label {
                    id: statusText
                    width: parent.width
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    color: "red"
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        if(userInfoHttpRequest.hasError)
                            return userInfoHttpRequest.errorCode + ": " + userInfoHttpRequest.errorText
                        if(deactivateHttpRequest.hasError)
                            return deactivateHttpRequest.errorCode + ": " + deactivateHttpRequest.errorText
                        return ""
                    }
                }
            }

            BusyOverlay {
                id: busyOverlay
                anchors.fill: parent
                visible: User.busy || userInfoHttpRequest.busy || deactivateHttpRequest.busy
                busyMessage: deactivateHttpRequest.busy ? "Deactivating..." : "Saving changes..."
            }

            JsonHttpRequest {
                id: userInfoHttpRequest
                type: JsonHttpRequest.POST
                api: "user/me"
                reportNetworkErrors: true
                onFinished: {
                    if(hasError || !hasResponse)
                        return

                    User.reload()
                }
            }

            JsonHttpRequest {
                id: deactivateHttpRequest
                type: JsonHttpRequest.POST
                api: "app/deactivate"
                reportNetworkErrors: true
                onFinished: {
                    if(hasError || !hasResponse)
                        return

                    store("email", "")
                    store("loginToken", "")
                    store("sessionToken", "");
                    User.reload()
                    modalDialog.close()
                }
            }

            property bool modalDialogClosable: !buttonsRow.visible && !busyOverlay.visible
            onModalDialogClosableChanged: modalDialog.closeable = modalDialogClosable
        }
    }

    property ErrorReport userErrorReport: Aggregation.findErrorReport(User)
    Notification.active: userErrorReport.hasError
    Notification.title: "User Account / Device Activation Error"
    Notification.text: userErrorReport.details.code + ": " + userErrorReport.errorMessage
    Notification.autoClose: false
    Notification.onDismissed: userErrorReport.clear()
}
