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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

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
        source: Scrite.user.loggedIn ? "image://userIcon/me" + counter : "image://userIcon/default"
        x: 20
        height: parent.height
        width: parent.height
        smooth: true
        mipmap: true
        fillMode: Image.PreserveAspectFit
        transformOrigin: Item.Right
        ToolTip.text: Scrite.user.loggedIn ? "Account Information" : "Login"

        BusyIcon {
            visible: Scrite.user.busy
            running: Scrite.user.busy
            anchors.centerIn: parent
            forDarkBackground: true
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
    }

    Connections {
        target: Scrite.user
        enabled: privateData.showLoginWizardOnForceLoginRequest
        function onForceLoginRequest() {
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

    Announcement.onIncoming: (type,data) => {
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
                    return Scrite.user.busy ? loginWizardBusyPage : (Scrite.user.loggedIn ? loginWizardUserProfilePage : loginWizardEmailPage)
                }

                Component.onCompleted: page = Scrite.user.busy ? e_BUSY_PAGE : (Scrite.user.loggedIn ? e_USER_PROFILE_PAGE : e_LOGIN_EMAIL_PAGE)

                Announcement.onIncoming: (type,data) => {
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
                running: !Scrite.user.busy
                interval: 100
                onTriggered: Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", Scrite.user.loggedIn ? e_USER_PROFILE_PAGE : e_LOGIN_EMAIL_PAGE)
            }
        }
    }

    Component {
        id: loginWizardEmailPage

        Item {
            property string pageTitle: "Sign Up / Login"
            Component.onCompleted: modalDialog.closeable = false

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
                        font.pointSize: Scrite.app.idealFontPointSize + 4
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Signup / login to unlock Structure, Notebook and many more features in Scrite."
                        color: Qt.darker("#65318f")
                    }

                    TextField {
                        id: emailField
                        width: parent.width
                        placeholderText: "Enter your Email ID"
                        font.pointSize: Scrite.app.idealFontPointSize + 4
                        text: sendActivationCodeCall.email()
                        validator: RegExpValidator {
                            regExp: /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
                        }
                        selectByMouse: true
                        horizontalAlignment: Text.AlignHCenter
                        Component.onCompleted: Qt.callLater( () => {
                                cursorPosition = Math.max(0,length)

                            })
                        Keys.onReturnPressed: requestActivationCode()
                        Keys.onEscapePressed: focus = false

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
                            text: parent.acceptableInput ? "Continue »" : "Provide your Email ID"
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
                            font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
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
                horizontalAlignment: Text.AlignHCenter
                onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/signup-login/")
                width: parent.width*0.8
                wrapMode: Text.WordWrap
                anchors.horizontalCenter: parent.horizontalCenter
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
            Component.onCompleted: modalDialog.closeable = false

            Item {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: nextButton.top

                Column {
                    width: parent.width*0.8
                    anchors.centerIn: parent
                    spacing: 40

                    TextField2 {
                        id: activationCodeField
                        width: parent.width
                        enableTransliteration: false
                        includeEmojiSymbols: false
                        tabItemUponReturn: false
                        placeholderText: text === "" ? "Paste the activation code here..." : "Activation Code: "
                        font.pixelSize: modalDialog.height * 0.025
                        selectByMouse: true
                        horizontalAlignment: Text.AlignHCenter
                        Keys.onReturnPressed: nextButton.click()
                        Keys.onEscapePressed: focus = false
                        onTextChanged: {
                            if(text === "")
                                resendActivationCodeTimer.begin()
                            else {
                                resendActivationCode.timeout = false
                                resendActivationCodeTimer.end()
                            }

                            activateCall.reset()
                            resendActivationCodeCall.reset()
                        }
                    }

                    Text {
                        width: parent.width * 0.8
                        wrapMode: Text.WordWrap
                        font.pointSize: Scrite.app.idealFontPointSize
                        horizontalAlignment: resendActivationCode.visible && resendActivationCode.enabled ? Text.AlignLeft : Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: {
                            const email = activateCall.fetch("email")
                            if(resendActivationCode.visible && resendActivationCode.enabled)
                                return "If you have not yet received the activation code on <b>" + email + "</b>, then<br/>... click on the 'Resend Code' to get a new code, or<br/>... click on 'Change Email' to provide a new email-id."
                            if(activationCodeField.length < 20)
                                return "Paste the activation code sent to your email: <b>" + email + "</b> into the field above."
                            return "Click 'Activate' to complete activation."
                        }
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

                Button2 {
                    id: resendActivationCode
                    text: "Resend Code" + (resendActivationCodeTimer.running ? " (" + resendActivationCodeTimer.secondsLeft + ")" : "")
                    visible: !errorMessageText.visible
                    enabled: timeout
                    anchors.centerIn: parent
                    property bool timeout: false

                    Timer {
                        id: resendActivationCodeTimer
                        running: true
                        interval: 1000
                        repeat: true
                        onTriggered: {
                            secondsLeft = Math.max(secondsLeft-1, 0)
                            if(secondsLeft === 0) {
                                stop()
                                resendActivationCode.timeout = true
                            }
                        }

                        readonly property int maxSeconds: 30
                        property int secondsLeft: maxSeconds

                        function begin() {
                            secondsLeft = maxSeconds
                            start()
                        }
                        function end() {
                            stop()
                        }
                    }

                    JsonHttpRequest {
                        id: resendActivationCodeCall
                        type: JsonHttpRequest.POST
                        api: "app/activate"
                        token: ""
                        reportNetworkErrors: true
                        onFinished: {
                            if(hasError || !hasResponse)
                                return
                            resendActivationCode.timeout = false
                            resendActivationCodeTimer.begin()
                        }
                        onBusyChanged: modalDialog.closeable = !busy
                    }

                    onClicked: {
                        resendActivationCodeCall.data = {
                            "email": activateCall.fetch("email"),
                            "request": "resendActivationCode"
                        }
                        resendActivationCodeCall.call()
                    }
                }

                Text {
                    id: errorMessageText
                    width: parent.width
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                    font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
                    maximumLineCount: 3
                    color: "red"
                    visible: text !== ""
                    text: {
                        if(activateCall.hasError)
                            return (activateCall.errorCode + ": " + activateCall.errorText)
                        if(resendActivationCodeCall.hasError)
                            return (resendActivationCodeCall.errorCode + ": " + resendActivationCodeCall.errorText)
                        return ""
                    }
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
                visible: activateCall.busy || resendActivationCodeCall.busy
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
                    Scrite.user.reload()
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
                if(Scrite.user.loggedIn) {
                    if(Scrite.user.info.firstName && Scrite.user.info.firstName !== "")
                        return "Hi, " + Scrite.user.info.firstName + "."
                    if(Scrite.user.info.lastName && Scrite.user.info.lastName !== "")
                        return "Hi, " + Scrite.user.info.lastName + "."
                }
                return "Hi, there."
            }

            property bool userLoggedIn: Scrite.user.loggedIn
            onUserLoggedInChanged: {
                if(!userLoggedIn)
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
            }

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
                            font.pointSize: Scrite.app.idealFontPointSize
                            horizontalAlignment: Text.AlignHCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: Scrite.user.loggedIn
                            text: "You're currently logged in via <b>" + Scrite.user.info.email + "</b>."
                        }

                        Link {
                            width: parent.width
                            anchors.horizontalCenter: parent.horizontalCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
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
                            text: Scrite.user.fullName
                            Component.onCompleted: forceActiveFocus()
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 0
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            onReturnPressed: if(needsSaving) saveRefreshLink.click()
                            undoRedoEnabled: true
                        }

                        TextField2 {
                            id: experienceField
                            width: (parent.width-parent.columnSpacing)/2
                            text: Scrite.user.experience
                            placeholderText: "Experience"
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 1
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            completionStrings: ["Hobby Writer", "Actively Pursuing a Writing Career", "Working Writer", "Have Produced Credits"]
                            minimumCompletionPrefixLength: 0
                            maxCompletionItems: -1
                            maxVisibleItems: 6
                            onReturnPressed: if(needsSaving) saveRefreshLink.click()
                            undoRedoEnabled: true
                        }

                        TextField2 {
                            id: locationField
                            width: (parent.width-parent.columnSpacing)/2
                            text: Scrite.user.location
                            placeholderText: "Location (City, Country)"
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 2
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            completionAcceptsEnglishStringsOnly: false
                            completionStrings: Scrite.user.locations
                            minimumCompletionPrefixLength: 0
                            onReturnPressed: if(needsSaving) saveRefreshLink.click()
                            undoRedoEnabled: true
                        }

                        TextField2 {
                            id: wdyhasField
                            width: (parent.width-parent.columnSpacing)/2
                            text: Scrite.user.wdyhas
                            placeholderText: "Where did you hear about Scrite?"
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 3
                            maximumLength: 128
                            onTextEdited: allowHighlightSaveAnimation = true
                            completionStrings: [
                                "Colleague",
                                "Email",
                                "Facebook",
                                "Filmschool",
                                "Friend",
                                "Instagram",
                                "Internet Search",
                                "Invited to Collaborate",
                                "LinkedIn",
                                "Twitter",
                                "Workshop",
                                "YouTube"
                            ]
                            minimumCompletionPrefixLength: 0
                            maxCompletionItems: -1
                            maxVisibleItems: 6
                            onReturnPressed: if(needsSaving) saveRefreshLink.click()
                            undoRedoEnabled: true
                        }

                        CheckBox2 {
                            id: chkAnalyticsConsent
                            checked: Scrite.user.info.consent.activity
                            text: "Send analytics data."
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 4
                            onToggled: allowHighlightSaveAnimation = true
                        }

                        CheckBox2 {
                            id: chkEmailConsent
                            checked: Scrite.user.info.consent.email
                            text: "Send marketing email."
                            TabSequenceItem.manager: userInfoFields
                            TabSequenceItem.sequence: 5
                            onToggled: allowHighlightSaveAnimation = true
                        }
                    }
                }
            }

            property bool needsSaving: nameField.text.trim() !== Scrite.user.fullName ||
                                       locationField.text.trim() !== Scrite.user.location ||
                                       experienceField.text.trim() !== Scrite.user.experience ||
                                       wdyhasField.text.trim() !== Scrite.user.wdyhas ||
                                       chkAnalyticsConsent.checked !== Scrite.user.info.consent.activity ||
                                       chkEmailConsent.checked !== Scrite.user.info.consent.email
            onNeedsSavingChanged: modalDialog.closeable = !needsSaving
            Component.onCompleted: modalDialog.closeable = !needsSaving

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
                    font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
                    opacity: needsSaving ? 0.85 : 1
                    onClicked: {
                        if(needsSaving) {
                            Announcement.shout("76281526-A16C-4414-8129-AD8770A17F16", undefined)
                        } else {
                            Scrite.user.logout()
                            if(!Scrite.user.loggedIn)
                                Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
                        }
                    }
                }

                Link {
                    text: "Privacy Policy"
                    opacity: needsSaving ? 0.5 : 1
                    font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
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
                    font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
                    maximumLineCount: 3
                    color: "red"
                    text: {
                        if(userError.hasError)
                            return userError.details && userError.details.code && userError.details.message ? (userError.details.code + ": " + userError.details.message) : ""
                        return ""
                    }
                    property ErrorReport userError: Aggregation.findErrorReport(Scrite.user)
                }

                Image {
                    source: "../images/scrite_discord_button.png"
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                    visible: Scrite.user.info.discordInviteUrl && Scrite.user.info.discordInviteUrl !== "" && errorText.text === ""
                    enabled: visible
                    opacity: needsSaving ? 0.5 : 1
                    mipmap: true

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally(Scrite.user.info.discordInviteUrl)
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
                    font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
                    transformOrigin: Item.BottomRight
                    anchors.right: parent.right
                    property real characterSpacing: 0
                    font.letterSpacing: characterSpacing
                    onClicked: click()

                    function click() {
                        if(needsSaving) {
                            var names = nameField.text.trim().split(' ')

                            const _lastName = names.length > 1 ? names[names.length-1] : ""
                            if(names.length > 1)
                                names.pop()
                            const _firstName = names.join(" ")

                            const newInfo = {
                                firstName: _firstName,
                                lastName: _lastName,
                                experience: experienceField.text.trim(),
                                location: locationField.text.trim(),
                                wdyhas: wdyhasField.text.trim(),
                                consent: {
                                    activity: chkAnalyticsConsent.checked,
                                    email: chkEmailConsent.checked
                                }
                            }
                            allowHighlightSaveAnimation = false
                            Scrite.user.update(newInfo)
                        } else
                            Scrite.user.reload()
                    }

                    Connections {
                        target: Scrite.user
                        function onInfoChanged() {
                            saveRefreshLink.restore()
                        }
                    }

                    function restore() {
                        saveRefreshLink.font.bold = needsSaving
                        saveRefreshLink.font.pointSize = Scrite.app.idealFontPointSize + (needsSaving ? 5 : 0)
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
                    opacity: needsSaving ? 0.5 : 1
                    font.pointSize: (Scrite.app.isMacOSPlatform ? Scrite.app.idealFontPointSize-2 : Scrite.app.idealFontPointSize)
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
                visible: Scrite.user.busy
                busyMessage: "Please wait.."
                onVisibleChanged: modalDialog.closeable = !visible
            }
        }
    }

    Component {
        id: loginWizardUserInstallationsPage

        Item {
            property string pageTitle: "Your Scrite Installations"

            property bool userLoggedIn: Scrite.user.loggedIn
            onUserLoggedInChanged: {
                if(!userLoggedIn)
                    Announcement.shout("93DC1133-58CA-4EDD-B803-82D9B6F2AA50", e_LOGIN_EMAIL_PAGE)
            }

            Component.onCompleted: {
                busyOverlay.busyMessage = "Fetching installations information ..."
                Scrite.user.refreshInstallations()
            }

            Link {
                id: backLink
                text: "« Back"
                font.pointSize: Scrite.app.idealFontPointSize
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

                model: Scrite.user.installations
                ScrollBar.vertical: ScrollBar2 {
                    flickable: installationsView
                }
                spacing: 20
                property real availableDelegateWidth: width - (contentHeight > height ? 20 : 0)
                header: Text {
                    width: installationsView.availableDelegateWidth
                    wrapMode: Text.WordWrap
                    font.pointSize: Scrite.app.idealFontPointSize
                    text: "<strong>" + Scrite.user.email + "</strong> is currently logged in at " + (Scrite.user.installations.length) + " computers(s)."
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
                                font.pointSize: Scrite.app.idealFontPointSize
                                font.bold: true
                                text: modelData.platform + " " + modelData.platformVersion + " (" + modelData.platformType + ")"
                                color: colors.text
                                width: parent.width
                                elide: Text.ElideRight
                            }

                            Text {
                                font.pointSize: Scrite.app.idealFontPointSize
                                text: "Runs Scrite " + modelData.appVersions[0]
                                color: colors.text
                                width: parent.width
                                elide: Text.ElideRight
                            }

                            Text {
                                font.pointSize: Scrite.app.idealFontPointSize-4
                                text: "Since: " + Scrite.app.relativeTime(new Date(modelData.firstActivationDate))
                                color: colors.text
                                opacity: 0.90
                                width: parent.width
                                elide: Text.ElideRight
                            }

                            Text {
                                font.pointSize: Scrite.app.idealFontPointSize-4
                                text: "Last Login: " + Scrite.app.relativeTime(new Date(modelData.lastActivationDate))
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
                            enabled: index !== Scrite.user.currentInstallationIndex
                            opacity: enabled ? 1 : 0.2
                            onClicked: {
                                busyOverlay.busyMessage = "Logging out of selected installation ..."
                                Scrite.user.deactivateInstallation(modelData._id)
                            }
                        }
                    }
                }
            }

            BusyOverlay {
                id: busyOverlay
                anchors.fill: parent
                visible: Scrite.user.busy
                busyMessage: "Please wait ..."
            }
        }
    }

    property ErrorReport userErrorReport: Aggregation.findErrorReport(Scrite.user)
    Notification.active: userErrorReport.hasError
    Notification.title: "User Account"
    Notification.text: (userErrorReport.details && userErrorReport.details.code ? (userErrorReport.details.code + ": ") : "") + userErrorReport.errorMessage
    Notification.autoClose: false
    Notification.onDismissed: userErrorReport.clear()
}
