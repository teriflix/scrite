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

    readonly property int e_BUSY_PAGE: -1
    readonly property int e_LOGIN_EMAIL_PAGE: 0
    readonly property int e_LOGIN_ACTIVATION_PAGE: 1
    readonly property int e_USER_PROFILE_PAGE: 2
    readonly property int e_USER_INSTALLATIONS_PAGE: 3

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
            onClicked: showLoginWizard()
        }
    }

    QtObject {
        id: privateData
        property bool showLoginWizardOnForceLoginRequest: true
        property bool receivedForceLoginRequest: true
        property bool loginPageShownForTheFirstTime: true
    }

    Connections {
        target: User
        enabled: privateData.showLoginWizardOnForceLoginRequest
        onForceLoginRequest: {
            if(privateData.showLoginWizardOnForceLoginRequest) {
                if(splashLoader.active)
                    splashLoader.activeChanged.connect( () => {
                        showLoginWizard()
                    })
                else
                    showLoginWizard()
                privateData.showLoginWizardOnForceLoginRequest = false
            }
        }
    }

    function showLoginWizard() {
        modalDialog.sourceComponent = loginWizard
        modalDialog.popupSource = profilePic
        modalDialog.active = true
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
                    width: parent.width-25
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    elide: Text.ElideRight
                    font.pixelSize: parent.height * 0.3
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
                property int page: e_BUSY_PAGE

                sourceComponent: {
                    switch(page) {
                    case e_BUSY_PAGE: return loginWizardBusyPage
                    case e_LOGIN_EMAIL_PAGE: return loginWizardEmailPage
                    case e_LOGIN_ACTIVATION_PAGE: return loginWizardActivationCodePage
                    case e_USER_PROFILE_PAGE: return loginWizardUserProfilePage
                    case e_USER_INSTALLATIONS_PAGE: return loginWizardUserInstallationsPage
                    default: break
                    }
                    return User.busy ? loginWizardBusyPage : (User.loggedIn ? loginWizardUserProfilePage : loginWizardEmailPage)
                }

                Component.onCompleted: page = User.busy ? e_BUSY_PAGE : (User.loggedIn ? e_USER_PROFILE_PAGE : e_LOGIN_EMAIL_PAGE)

                Announcement.onIncoming: {
                    const stype = "" + type
                    const idata = data
                    if(stype === "93DC1133-58CA-4EDD-B803-82D9B6F2AA50")
                        page = idata
                    else if(stype === "76281526-A16C-4414-8129-AD8770A17F16") {
                        active = false
                        Qt.callLater( function() { pageLoader.active = true } )
                    }
                }
            }
        }
    }

    Component {
        id: loginWizardBusyPage

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
                onTriggered: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", User.loggedIn ? e_USER_PROFILE_PAGE : e_LOGIN_EMAIL_PAGE)
            }
        }
    }

    Component {
        id: loginWizardEmailPage

        Item {
            property string pageTitle: privateData.loginPageShownForTheFirstTime ? "Something's New! Please Login to Continue" : "Sign Up / Login"
            Component.onCompleted: modalDialog.closeable = false
            Component.onDestruction: privateData.loginPageShownForTheFirstTime = false

            Item {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: releaseNotesLink.bottom
                anchors.bottomMargin: Math.max(releaseNotesLink.height, noLoginContinueLink.height)

                Column {
                    width: parent.width*0.8
                    anchors.centerIn: parent
                    spacing: 40

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pointSize: app.idealFontPointSize + 4
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Signup / login with your email to unlock Structure, Notebook and many more features in Scrite."
                        color: Qt.darker("#65318f")
                        visible: !privateData.loginPageShownForTheFirstTime
                    }

                    TextField {
                        id: emailField
                        width: parent.width
                        placeholderText: length > 0 && acceptableInput ? "Hit Return to Continue" : "Enter Email ID and hit Return"
                        font.pointSize: app.idealFontPointSize + 4
                        text: sendActivationCodeCall.email()
                        validator: RegExpValidator {
                            regExp: /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
                        }
                        selectByMouse: true
                        horizontalAlignment: Text.AlignHCenter
                        Component.onCompleted: Qt.callLater( () => {
                                forceActiveFocus()
                                cursorPosition = Math.max(0,length)
                            })
                        Keys.onReturnPressed: requestActivationCode()

                        function requestActivationCode() {
                            if(acceptableInput) {
                                sendActivationCodeCall.data = {
                                    "email": emailField.text,
                                    "request": "resendActivationCode"
                                }
                                sendActivationCodeCall.call()
                            }
                        }

                        Link {
                            id: continueLink
                            anchors.top: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.topMargin: 10
                            text: "Continue »"
                            defaultColor: releaseNotesLink.defaultColor
                            hoverColor: releaseNotesLink.hoverColor
                            font.underline: false
                            enabled: parent.focus ? parent.acceptableInput : true
                            opacity: enabled ? 1.0 : 0.5
                            onClicked: parent.requestActivationCode()
                        }

                        Text {
                            id: errorText
                            width: parent.width
                            anchors.top: continueLink.bottom
                            anchors.topMargin: 20
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
                            maximumLineCount: 3
                            color: "red"
                            text: sendActivationCodeCall.hasError ? (sendActivationCodeCall.errorCode + ": " + sendActivationCodeCall.errorText) : ""
                        }
                    }
                }
            }

            Link {
                id: releaseNotesLink
                font.underline: false
                text: "Wondering why you are being asked to login? <u>Click here</u> ..."
                onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/login-and-activation/")
                width: parent.width*0.35
                wrapMode: Text.WordWrap
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 30
                enabled: !sendActivationCodeCall.busy
                defaultColor: "#65318f"
                hoverColor: Qt.darker(defaultColor)
            }

            Link {
                id: noLoginContinueLink
                font.underline: false
                text: "Or <u>Continue Without Logging In</u> »"
                horizontalAlignment: Text.AlignRight
                width: parent.width*0.25
                wrapMode: Text.WordWrap
                onClicked: modalDialog.close()
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 30
                enabled: !sendActivationCodeCall.busy
                defaultColor: "#65318f"
                hoverColor: Qt.darker(defaultColor)
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
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_ACTIVATION_PAGE)
                }
                onBusyChanged: modalDialog.closeable = !busy
            }
        }
    }

    Component {
        id: loginWizardActivationCodePage

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
                        Keys.onReturnPressed: nextButton.click()
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
                onClicked: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
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
                    font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
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
                enabled: activationCodeField.length >= 20
                onClicked: click()
                function click() {
                    if(!enabled)
                        return
                    activateCall.data = {
                        "email": activateCall.email(),
                        "activationCode": activationCodeField.text.trim(),
                        "clientId": activateCall.clientId(),
                        "deviceId": activateCall.deviceId(),
                        "platform": activateCall.platform(),
                        "platformType": activateCall.platformType(),
                        "platformVersion": activateCall.platformVersion(),
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
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_USER_PROFILE_PAGE)
                }
                onBusyChanged: modalDialog.closeable = !busy
            }
        }
    }

    Component {
        id: loginWizardUserProfilePage

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

            property bool userLoggedIn: User.loggedIn
            onUserLoggedInChanged: {
                if(!userLoggedIn)
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
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

                    Column {
                        width: parent.width
                        spacing: 5

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            font.pointSize: app.idealFontPointSize
                            horizontalAlignment: Text.AlignHCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: User.loggedIn
                            text: "You're currently logged in via <b>" + User.info.email + "</b>."
                        }

                        Link {
                            width: parent.width
                            anchors.horizontalCenter: parent.horizontalCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
                            text: "Review Your Scrite Installations »"
                            onClicked: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_USER_INSTALLATIONS_PAGE)
                        }

                        Item {
                            width: parent.width
                            height: 15
                        }
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

            property bool needsSaving: nameField.text.trim() !== User.fullName ||
                                       cityField.text.trim() !== User.city ||
                                       countryField.text.trim() !== User.country ||
                                       experienceField.text.trim() !== User.experience ||
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
                    font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
                    opacity: needsSaving ? 0.75 : 1
                    onClicked: {
                        if(needsSaving) {
                            Announcement.shout("76281526-A16C-4414-8129-AD8770A17F16", undefined)
                        } else {
                            User.logout()
                            if(!User.loggedIn)
                                Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
                        }
                    }
                }

                Link {
                    text: "Privacy Policy"
                    font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
                    opacity: needsSaving ? 0.75 : 1
                    anchors.right: parent.right
                    onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/privacy-policy/")
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
                    id: errorText
                    width: parent.width
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                    font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
                    maximumLineCount: 3
                    color: "red"
                    text: {
                        if(userError.hasError)
                            return userError.details && userError.details.code && userError.details.message ? (userError.details.code + ": " + userError.details.message) : ""
                        return ""
                    }
                    property ErrorReport userError: Aggregation.findErrorReport(User)
                }

                Image {
                    source: "../images/scrite_discord_button.png"
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                    visible: User.info.discordInviteUrl && User.info.discordInviteUrl !== "" && errorText.text === ""
                    enabled: visible

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally(User.info.discordInviteUrl)
                        ToolTip.text: "Ask questions, post feedback, request features and connect with other Scrite users."
                        ToolTip.visible: containsMouse
                        ToolTip.delay: 1000
                        hoverEnabled: true
                    }
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
                    font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
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
                    text: "Feedback / About"
                    font.pointSize: (app.isMacOSPlatform ? app.idealFontPointSize-2 : app.idealFontPointSize)
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

            BusyOverlay {
                anchors.fill: parent
                visible: User.busy
                busyMessage: "Please wait.."
                onVisibleChanged: modalDialog.closeable = !visible
            }
        }
    }

    Component {
        id: loginWizardUserInstallationsPage

        Item {
            property string pageTitle: "Your Scrite Installations"

            property bool userLoggedIn: User.loggedIn
            onUserLoggedInChanged: {
                if(!userLoggedIn)
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
            }

            Component.onCompleted: {
                busyOverlay.busyMessage = "Fetching installations information ..."
                User.refreshInstallations()
            }

            Link {
                id: backLink
                text: "« Back"
                font.pointSize: app.idealFontPointSize
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 20
                onClicked: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_USER_PROFILE_PAGE)
            }

            ListView {
                id: installationsView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: backLink.bottom
                anchors.bottom: parent.bottom
                anchors.margins: 20
                clip: true

                model: User.installations
                ScrollBar.vertical: ScrollBar2 {
                    flickable: installationsView
                }
                spacing: 20
                property real availableDelegateWidth: width - (contentHeight > height ? 20 : 0)
                header: Text {
                    width: installationsView.availableDelegateWidth
                    wrapMode: Text.WordWrap
                    font.pointSize: app.idealFontPointSize
                    text: "<strong>" + User.email + "</strong> is currently logged in at " + (User.installations.length) + " computers(s)."
                    horizontalAlignment: Text.AlignHCenter
                    padding: 10
                }

                delegate: Rectangle {
                    property var colors: index%2 ? primaryColors.c200 : primaryColors.c300
                    width: installationsView.availableDelegateWidth
                    height: Math.max(infoLayout.height, logoutButton.height) + 16
                    color: colors.background
                    radius: 8

                    Item {
                        anchors.fill: parent
                        anchors.margins: 8

                        Column {
                            id: infoLayout
                            anchors.left: parent.left
                            anchors.right: logoutButton.left
                            anchors.top: parent.top
                            anchors.leftMargin: 30
                            anchors.rightMargin: 30
                            spacing: 4

                            Text {
                                font.pointSize: app.idealFontPointSize
                                font.bold: true
                                text: modelData.platform + " " + modelData.platformVersion + " (" + modelData.platformType + ")"
                                color: colors.text
                                width: parent.width
                                elide: Text.ElideRight
                            }

                            Text {
                                font.pointSize: app.idealFontPointSize
                                text: "Runs Scrite " + modelData.appVersions[0]
                                color: colors.text
                                width: parent.width
                                elide: Text.ElideRight
                            }

                            Text {
                                font.pointSize: app.idealFontPointSize-4
                                text: "Since: " + app.relativeTime(new Date(modelData.firstActivationDate))
                                color: colors.text
                                opacity: 0.90
                                width: parent.width
                                elide: Text.ElideRight
                            }

                            Text {
                                font.pointSize: app.idealFontPointSize-4
                                text: "Last Login: " + app.relativeTime(new Date(modelData.lastActivationDate))
                                color: colors.text
                                opacity: 0.75
                                width: parent.width
                                elide: Text.ElideRight
                            }
                        }

                        ToolButton3 {
                            id: logoutButton
                            iconSource: "../icons/action/logout.png"
                            anchors.top: parent.top
                            anchors.right: parent.right
                            enabled: index !== User.currentInstallationIndex
                            opacity: enabled ? 1 : 0.2
                            onClicked: {
                                busyOverlay.busyMessage = "Logging out of selected installation ..."
                                User.deactivateInstallation(modelData._id)
                            }
                        }
                    }
                }
            }

            BusyOverlay {
                id: busyOverlay
                anchors.fill: parent
                visible: User.busy
                busyMessage: "Please wait ..."
            }
        }
    }

    property ErrorReport userErrorReport: Aggregation.findErrorReport(User)
    Notification.active: userErrorReport.hasError
    Notification.title: "User Account"
    Notification.text: (userErrorReport.details && userErrorReport.details.code ? (userErrorReport.details.code + ": ") : "") + userErrorReport.errorMessage
    Notification.autoClose: false
    Notification.onDismissed: userErrorReport.clear()
}
