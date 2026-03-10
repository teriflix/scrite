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

import "../../tasks"
import "../../globals"
import "../../controls"
import "../../helpers"
import ".."

Item {
    id: root

    property bool modal: !Scrite.user.info.hasActiveSubscription
    property string title: {
        if(Scrite.user.loggedIn) {
            if(Scrite.user.info.firstName && Scrite.user.info.firstName !== "")
                return "Hi, " + Scrite.user.info.firstName + "."
            if(Scrite.user.info.lastName && Scrite.user.info.lastName !== "")
                return "Hi, " + Scrite.user.info.lastName + "."
        }
        return "Hi, there."
    }

    Component.onCompleted: {
        if(Scrite.user.loggedIn)
            Runtime.showHelpTip("UserProfileDialog")
    }

    PageView {
        id: _userProfilePageView
        anchors.fill: parent

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.userProfileScreenPage) {
                                         currentIndex = Math.max(0, pagesArray.indexOf(data))
                                     }
                                 }

        pagesArray: ["Profile", "Subscriptions", "Installations", "Notifications"]
        currentIndex: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription ? 0 : 1
        maxPageListWidth: {
            if(pagesArray.length < 2)
                return 120

            let textMetrics = Qt.createQmlObject("import QtQuick 2.15; TextMetrics { }", _userProfilePageView)
            textMetrics.font.pointSize = Runtime.idealFontMetrics.font.pointSize
            textMetrics.text = ( () => {
                                    const options = pagesArray
                                    let ret = ""
                                    options.forEach( (option) => {
                                                        if(option.length > ret.length)
                                                        ret = option
                                                    })
                                    return ret
                                })()

            const ret = textMetrics.advanceWidth + 50
            textMetrics.destroy()

            return ret
        }
        pageContent: {
            switch(_userProfilePageView.currentIndex) {
            case 1: return _userSubscriptionsPageComponent
            case 2: return _userInstallationsPageComponent
            case 3: return _userNotificationsPageComponent
            default: break
            }
            return _userProfilePageComponent
        }
        cornerContent: Item {
            Image {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width-20

                source: "qrc:/images/scrite_discord_button.png"
                fillMode: Image.PreserveAspectFit
                enabled: visible
                mipmap: true

                MouseArea {
                    id: _discordButtonMouseArea
                    anchors.fill: parent

                    ToolTipPopup {
                        text: "Ask questions, post feedback, request features and connect with other Scrite users."
                        visible: _discordButtonMouseArea.hovered
                    }

                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: Qt.openUrlExternally("https://www.scrite.io/forum/")
                }
            }
        }
    }

    Component {
        id: _userProfilePageComponent

        Item {
            id: _userProfilePage

            height: _userProfilePageView.availablePageContentHeight

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
                enabled: !_userProfilePage.callList.busy

                VclLabel {
                    Layout.fillWidth: true

                    text: "You're logged in via <b>" + _userProfilePage.userInfo.email + "</b>."
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

                        text: _userProfilePage.userInfo.fullName
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

                        text: _userProfilePage.userInfo.phone
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

                    text: _userProfilePage.userInfo.experience
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

                        text: _userProfilePage.userInfo.city
                        maximumLength: 128
                        placeholderText: "City"
                        undoRedoEnabled: true

                        onTextEdited: _userInfoFields.needsSaving = true
                    }

                    VclTextField {
                        id: _countryField

                        Layout.fillWidth: true

                        placeholderText: "Country"
                        text: _userProfilePage.userInfo.country
                        readOnly: true
                    }
                }

                VclTextField {
                    id: _wdyhasField

                    Layout.fillWidth: true

                    TabSequenceItem.manager: _userInfoFields
                    TabSequenceItem.sequence: 4

                    text: _userProfilePage.userInfo.wdyhas
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
                        checked: _userProfilePage.userInfo.consentToActivityLog
                        padding: 0

                        onToggled: _userInfoFields.needsSaving = true
                    }

                    VclCheckBox {
                        id: _chkEmailConsent

                        TabSequenceItem.manager: _userInfoFields
                        TabSequenceItem.sequence: 6

                        text: "Send marketing email."
                        checked: _userProfilePage.userInfo.consentToEmail
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

                running: _userProfilePage.callList.busy
            }
        }
    }

    Component {
        id: _userNotificationsPageComponent

        Item {
            id: _userNotificationsPage

            height: _userProfilePageView.availablePageContentHeight

            Component.onDestruction: Scrite.user.markMessagesAsRead()

            VclLabel {
                anchors.centerIn: parent

                visible: Scrite.user.totalMessageCount === 0

                text: "There are no notifications for you at the moment."
            }

            ListView {
                id: _userMessagesView

                anchors.fill: parent

                ScrollBar.vertical: VclScrollBar { }
                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                clip: true
                height: parent.height
                visible: Scrite.user.totalMessageCount > 0

                model: Scrite.user.messages
                spacing: 20
                boundsBehavior: Flickable.StopAtBounds

                header: VclLabel {
                    width: _userMessagesView.width
                    padding: 10

                    font.bold: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                    text: {
                        const nrUnread = Scrite.user.unreadMessageCount
                        const nrMessages = Scrite.user.totalMessageCount

                        if(nrMessages === 0) {
                            return "You have no notifications right now."
                        }

                        if(nrUnread > 0) {
                            if(nrUnread === nrMessages)
                                return "You have " + nrUnread + " unread notification" + (nrUnread > 1 ? "s" : "") + "."
                            else
                                return "You have " + nrUnread + " of " + nrMessages + " unread notification" + (nrMessages > 1 ? "s" : "") + "."
                        }

                        return "You have " + nrMessages + " notification" + (nrMessages > 1 ? "s" : "") + "."
                    }

                    wrapMode: Text.WordWrap
                }

                footer: Item {
                    width: _userMessagesView.width
                    height: 20
                }

                delegate: Item {
                    required property int index
                    required property var modelData

                    width: _userMessagesView.width
                    height: _messageRect.height

                    Rectangle {
                        id: _messageRect

                        anchors.centerIn: parent

                        width: 450
                        height: _messageLayout.implicitHeight + 30
                        border {
                            width: modelData.read ? 1 : 2
                            color: Runtime.colors.primary.borderColor
                        }

                        color: Runtime.colors.primary.c200.background

                        ColumnLayout {
                            id: _messageLayout

                            anchors.centerIn: parent

                            width: parent.width - 20
                            spacing: 10

                            VclLabel {
                                Layout.fillWidth: true

                                text: Runtime.formatDateIncludingYear(modelData.timestamp)
                                color: Runtime.colors.primary.c200.text
                                opacity: 0.75
                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                            }

                            VclLabel {
                                Layout.fillWidth: true

                                text: modelData.subject
                                color: Runtime.colors.primary.c200.text
                                wrapMode: Text.WordWrap
                                font.bold: true
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            Image {
                                Layout.fillWidth: true
                                Layout.preferredHeight: sourceSize.height * (width/sourceSize.width)

                                source: modelData.image
                                mipmap: true
                                visible: source !== ""
                                fillMode: Image.PreserveAspectFit

                                MouseArea {
                                    anchors.fill: parent

                                    enabled: _buttonsRepeater.count === 1
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: _buttonsRepeater.itemAt(0).handleClick()
                                }
                            }

                            VclLabel {
                                Layout.fillWidth: true

                                text: modelData.body
                                color: Runtime.colors.primary.c200.text
                                wrapMode: Text.WordWrap
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1

                                color: Runtime.colors.primary.borderColor
                            }

                            Repeater {
                                id: _buttonsRepeater
                                model: modelData.buttons

                                delegate: Link {
                                    required property int index
                                    required property var modelData

                                    Layout.fillWidth: true

                                    padding: 4
                                    text: modelData.text
                                    horizontalAlignment: Text.AlignHCenter

                                    function handleClick() {
                                        if(modelData.action === UserMessageButton.UrlAction) {
                                            Qt.openUrlExternally(modelData.endpoint)
                                            return
                                        }

                                        if(modelData.action === UserMessageButton.CommandAction) {
                                            switch(modelData.endpoint) {
                                            case "$subscribe":
                                                Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, "Subscriptions")
                                                return
                                            case "$profile":
                                                Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, "Profile")
                                                return
                                            case "$installations":
                                                Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, "Installations")
                                                return
                                            case "$homescreen":
                                                HomeScreen.launch()
                                                return
                                            }
                                        }

                                        // Implement API and Code in a future update
                                        enabled = false
                                    }

                                    onClicked: handleClick()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: _userSubscriptionsPageComponent

        Item {
            id: _userSubscriptionsPage

            height: Math.max(_userSubsView.height, _userProfilePageView.availablePageContentHeight)

            property RestApiCallList callList: RestApiCallList {
                calls: [_queryUserSubsCall]
            }

            Item {
                id: _userSubsView
                width: parent.width
                height: _userSubsLayout.height + 40

                enabled: !_userSubscriptionsPage.callList.busy
                opacity: enabled ? 1 : 0.5

                ColumnLayout {
                    id: _userSubsLayout

                    y: 20
                    width: parent.width-20
                    visible: !_queryUserSubsCall.hasError

                    spacing: 30

                    // Active Subscription
                    Loader {
                        Layout.fillWidth: true

                        active: _queryUserSubsCall.activeSubscription !== undefined
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Active Subscription"

                            PlanCard {
                                property var activeSub: _queryUserSubsCall.activeSubscription

                                width: parent.width

                                name: activeSub.plan.title
                                duration: Runtime.dateSpanAsString(Runtime.todayWithZeroTime(), new Date(activeSub.until))
                                exclusive: activeSub.plan.exclusive
                                durationNote: "(" + Runtime.formatDateRangeAsString(new Date(activeSub.from), new Date(activeSub.until)) + ")"
                                price: "Active"
                                priceNote: Runtime.toTitleCase(activeSub.kind)
                                actionLink: "Details »"
                                actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                onActionLinkClicked: SubscriptionDetailsDialog.launch(activeSub)
                            }
                        }
                    }

                    // Upcoming Subscription (if any)
                    Loader {
                        Layout.fillWidth: true

                        active: _queryUserSubsCall.upcomingSubscription !== undefined
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Upcoming Subscription"

                            PlanCard {
                                property var upcomingSub: _queryUserSubsCall.upcomingSubscription

                                width: parent.width

                                name: upcomingSub.plan.title
                                duration: Runtime.dateSpanAsString(new Date(upcomingSub.from), new Date(upcomingSub.until))
                                exclusive: upcomingSub.plan.exclusive
                                durationNote: "(" + Runtime.formatDateRangeAsString(new Date(upcomingSub.from), new Date(upcomingSub.until)) + ")"
                                price: Runtime.toTitleCase(upcomingSub.kind)
                                priceNote: Runtime.dateSpanAsString(Runtime.todayWithZeroTime(), new Date(upcomingSub.from))
                                actionLink: "Details »"
                                actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                onActionLinkClicked: SubscriptionDetailsDialog.launch(upcomingSub)
                            }
                        }
                    }

                    // Available Plans (if any)
                    Loader {
                        id: _availablePlansLoader
                        Layout.fillWidth: true

                        active: _queryUserSubsCall.availablePlans.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            id: _availablePlansGroupBox

                            title: "Available Plans"

                            readonly property Component referralCodeLink : Link {
                                anchors.right: parent.right
                                anchors.bottom: parent.top
                                anchors.bottomMargin: _availablePlansGroupBox.topPadding/3

                                text: _queryUserSubsCall.responseData.referralCodeText + " »"
                                font.bold: true

                                onClicked: _referralCodeDialog.open()

                                VclDialog {
                                    id: _referralCodeDialog

                                    width: 400
                                    height: 240
                                    title: _queryUserSubsCall.responseData.referralCodeText

                                    content: Item {
                                        ColumnLayout {
                                            anchors.centerIn: parent

                                            enabled: !_referralCodeApi.busy
                                            opacity: enabled ? 1 : 0.5

                                            width: parent.width-50
                                            spacing: 20

                                            TextField {
                                                id: _txtReferralCode

                                                Layout.fillWidth: true

                                                focus: true
                                                maximumLength: Runtime.bounded(_queryUserSubsCall.responseData.minReferralCodeLength, _queryUserSubsCall.responseData.maxReferralCodeLength, 128)
                                                placeholderText: _queryUserSubsCall.responseData.referralCodeText
                                                horizontalAlignment: Text.AlignHCenter
                                            }

                                            VclButton {
                                                Layout.alignment: Qt.AlignHCenter

                                                text: "Submit"
                                                enabled: _txtReferralCode.length >= _queryUserSubsCall.responseData.minReferralCodeLength

                                                onClicked: {
                                                    _referralCodeApi.code = _txtReferralCode.text.toUpperCase().trim()
                                                    _referralCodeApi.call()
                                                }
                                            }
                                        }

                                        BusyIndicator {
                                            anchors.centerIn: parent
                                            running: _referralCodeApi.busy
                                        }

                                        SubscriptionReferralCodeRestApiCall {
                                            id: _referralCodeApi

                                            onBusyChanged: _referralCodeDialog.titleBarCloseButtonVisible = !busy

                                            onFinished: {
                                                if(hasError) {
                                                    MessageBox.information("Error", errorText)
                                                    return
                                                }

                                                if(hasResponse) {
                                                    _queryUserSubsCall.go()
                                                    _referralCodeDialog.close()
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            Component.onCompleted: {
                                if(_queryUserSubsCall.responseData.acceptingReferralCode === true)
                                    referralCodeLink.createObject(background)
                            }

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: _queryUserSubsCall.availablePlans

                                    delegate: PlanCard {
                                        required property int index
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.title
                                        duration: Runtime.daysSpanAsString(modelData.duration)
                                        exclusive: modelData.exclusive
                                        durationNote: modelData.featureNote
                                        price: {
                                            if(modelData.pricing.price === 0)
                                                return "FREE"

                                            const currencySymbol = Scrite.currencySymbol(modelData.pricing.currency)
                                            return currencySymbol + modelData.pricing.price + " *"
                                        }
                                        priceNote: modelData.subtitle
                                        actionLink: SubscriptionPlanOperations.planActionLinkText(modelData)
                                        onActionLinkClicked: SubscriptionPlanOperations.subscribeTo(modelData)
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        visible: _queryUserSubsCall.availablePlans.length > 0

                        VclLabel {
                            Layout.fillWidth: true

                            text: "* All prices are subject to change without notice."
                            wrapMode: Text.WordWrap
                        }

                        Link {
                            enabled: SubscriptionPlanOperations.taxonomy !== undefined
                            text: _queryUserSubsCall.availablePlans.length > 1 ? "Compare Plans »" : "Feature Details »"
                            font.bold: true
                            onClicked: {
                                const name = Scrite.user.info.fullName === "" ? Scrite.user.info.email : Scrite.user.info.fullName
                                const title = (_queryUserSubsCall.availablePlans.length > 1 ? "Plan Comparison for " : "Plan Details for ") + name
                                SubscriptionPlanComparisonDialog.launch(_queryUserSubsCall.availablePlans, title)
                            }
                        }
                    }

                    // Subscrition History
                    Loader {
                        Layout.fillWidth: true

                        active: _queryUserSubsCall.pastSubscriptions.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Subscription History"

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: _queryUserSubsCall.pastSubscriptions

                                    delegate: PlanCard {
                                        required property int index
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.plan.title
                                        duration: Runtime.dateSpanAsString(new Date(modelData.from), new Date(modelData.until))
                                        exclusive: modelData.plan.exclusive
                                        durationNote: Runtime.formatDateRangeAsString(new Date(modelData.from), new Date(modelData.until))
                                        price: Runtime.toTitleCase(modelData.kind)
                                        priceNote: modelData.plan.featureNote
                                        actionLink: "Details »"
                                        actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                        onActionLinkClicked: SubscriptionDetailsDialog.launch(modelData)
                                    }
                                }
                            }
                        }
                    }
                }

                VclButton {
                    anchors.centerIn: parent

                    text: "Reload"
                    visible: _queryUserSubsCall.hasError
                    onClicked: _queryUserSubsCall.go()
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: !_queryUserSubsCall.ready || _queryUserSubsCall.busy
            }

            Connections {
                target: Scrite.restApi

                function onSessionTokenAvailable() {
                    _queryUserSubsCall.go()
                }
            }

            SubscriptionPlansRestApiCall {
                id: _queryUserSubsCall

                property bool ready: false
                property var activeSubscription
                property var upcomingSubscription
                property var pastSubscriptions: []
                property var availablePlans: []

                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage, () => { _userProfilePageView.currentIndex = 0 })
                        return
                    }

                    const result = responseData
                    if(result.hasActiveSubscription)
                        activeSubscription = result.subscriptions[0]
                    if(result.hasUpcomingSubscription)
                        upcomingSubscription = result.subscriptions[1]

                    let history = []
                    result.subscriptions.forEach( (sub) => {
                                                     if(sub.hasExpired)
                                                     history.push(sub)
                                                 })
                    if(result.publicBetaSubscription)
                        history.push(result.publicBetaSubscription)

                    pastSubscriptions = history

                    availablePlans = result.plans
                }

                function go() {
                    if(busy)
                        return

                    ready = call()
                    if(!ready)
                        Runtime.execLater(_queryUserSubsCall, 500, go)
                }

                Component.onCompleted: go()
            }
        }
    }

    Component {
        id: _userInstallationsPageComponent

        Item {
            id: _userInstallationsPage

            height: _userProfilePageView.availablePageContentHeight

            ColumnLayout {
                id: _userInstallationsPageLayout

                anchors.fill: parent
                anchors.margins: 20
                anchors.leftMargin: 0

                // Header Section
                VclLabel {
                    Layout.fillWidth: true

                    font.bold: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 1
                    text: {
                        const total = Scrite.user.info.installations.length
                        const active = Scrite.user.info.activeInstallationCount
                        if(active === total)
                            return active + " Installations Active"
                        return total + " Total Installations, " + active + " Active"
                    }
                }

                // Devices Grid
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: Runtime.colors.transparent
                    border.width: _devicesFlickable.contentHeight > _devicesFlickable.height ? 1 : 0
                    border.color: Runtime.colors.primary.borderColor

                    Flickable {
                        id: _devicesFlickable

                        anchors.fill: parent
                        anchors.margins: clip ? 2 : 0

                        enabled: !_deactivateOtherCall.busy && !Scrite.user.busy
                        opacity: enabled ? 1 : 0.5

                        ScrollBar.vertical: VclScrollBar { }
                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                        clip: contentHeight > height
                        contentWidth: width
                        contentHeight: _devicesFlow.height

                        Flow {
                            id: _devicesFlow

                            width: _devicesFlickable.width - (_devicesFlickable.clip ? 20 : 0)
                            spacing: 10

                            Repeater {
                                model: Scrite.user.info.installations

                                delegate: Rectangle {
                                    required property int index
                                    required property var modelData

                                    width: Math.min(450, (_devicesFlow.width - _devicesFlow.spacing) / 2 - 1)
                                    height: _deviceCardLayout.implicitHeight + 40

                                    color: Runtime.colors.primary.c10.background
                                    border.width: modelData.isCurrent ? 3 : 1
                                    border.color: modelData.isCurrent ? Runtime.colors.accent.c600.background : Runtime.colors.primary.borderColor

                                    ColumnLayout {
                                        id: _deviceCardLayout

                                        anchors.fill: parent
                                        anchors.margins: 20

                                        spacing: 10

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 15

                                            Rectangle {
                                                Layout.preferredWidth: 60
                                                Layout.preferredHeight: 60
                                                Layout.alignment: Qt.AlignTop

                                                color: Runtime.colors.primary.c100.background
                                                border.width: 1
                                                border.color: Runtime.colors.primary.borderColor
                                                radius: 4

                                                VclText {
                                                    anchors.centerIn: parent

                                                    text: "Mac"
                                                    visible: modelData.platform === "macOS"

                                                    font.capitalization: Font.AllUppercase
                                                    font.pixelSize: parent.height * 0.25
                                                }

                                                Image {
                                                    anchors.fill: parent
                                                    anchors.margins: 5

                                                    mipmap: true
                                                    fillMode: Image.PreserveAspectFit
                                                    opacity: modelData.isCurrent ? 1 : 0.6
                                                    visible: modelData.platform !== "macOS"

                                                    source: {
                                                        switch(modelData.platform.toLowerCase()) {
                                                            case "windows": return "qrc:/icons/hardware/windows-platform.png"
                                                            case "linux": return "qrc:/icons/hardware/linux-platform.png"
                                                        }
                                                        return "qrc:/icons/hardware/desktop-platform.png"
                                                    }
                                                }
                                            }

                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                spacing: 5

                                                RowLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 10

                                                    VclLabel {
                                                        Layout.fillWidth: true

                                                        font.bold: modelData.isCurrent
                                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 1
                                                        text: {
                                                            let ret = ""
                                                            if(modelData.hostName !== "")
                                                                ret = modelData.hostName
                                                            else
                                                                ret = modelData.platform + " Device"
                                                            return ret
                                                        }
                                                        elide: Text.ElideRight
                                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                                        maximumLineCount: 2
                                                    }

                                                    Rectangle {
                                                        visible: modelData.isCurrent

                                                        Layout.preferredHeight: _thisDeviceLabel.implicitHeight + 8
                                                        Layout.preferredWidth: _thisDeviceLabel.implicitWidth + 12

                                                        color: Runtime.colors.accent.c500.background
                                                        radius: 4

                                                        VclLabel {
                                                            id: _thisDeviceLabel
                                                            anchors.centerIn: parent

                                                            text: "THIS DEVICE"
                                                            color: Runtime.colors.accent.c500.text
                                                            font.bold: true
                                                            font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                                                        }
                                                    }
                                                }

                                                VclLabel {
                                                    Layout.fillWidth: true

                                                    text: modelData.platform + " " + modelData.platformVersion
                                                    elide: Text.ElideRight
                                                    color: Runtime.colors.primary.c600.text
                                                }

                                                VclLabel {
                                                    Layout.fillWidth: true

                                                    text: "Scrite " + modelData.appVersion
                                                    elide: Text.ElideRight
                                                    color: Runtime.colors.primary.c600.text
                                                }
                                            }
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 1
                                            color: Runtime.colors.primary.borderColor
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 10

                                            VclLabel {
                                                Layout.fillWidth: true

                                                text: modelData.isCurrent ?
                                                          "Active since: " + Qt.formatDateTime(new Date(modelData.lastSessionDate), "dddd, h:mm AP (MMM dd, yyyy)") :
                                                          "Last Login: " + TMath.relativeTime(new Date(modelData.lastSessionDate))
                                                elide: Text.ElideRight
                                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                                maximumLineCount: 2
                                            }

                                            VclButton {
                                                visible: !modelData.isCurrent
                                                enabled: modelData.activated

                                                text: "Sign Out"
                                                icon.source: "qrc:/icons/action/logout.png"
                                                icon.width: 16
                                                icon.height: 16

                                                onClicked: {
                                                    _deactivateOtherCall.installationId = modelData.id
                                                    _deactivateOtherCall.call()
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: _deactivateOtherCall.busy || Scrite.user.busy
            }

            InstallationDeactivateOtherRestApiCall {
                id: _deactivateOtherCall
            }
        }
    }
}
