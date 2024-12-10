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

    PageView {
        id: userProfilePageView
        anchors.fill: parent

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.userProfileScreenPage) {
                                         currentIndex = Math.max(0, pagesArray.indexOf(data))
                                     }
                                 }

        pagesArray: ["Profile", "Subscriptions", "Installations"]
        currentIndex: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription ? 0 : 1
        maxPageListWidth: {
            if(pagesArray.length < 2)
                return 120

            let textMetrics = Qt.createQmlObject("import QtQuick 2.15; TextMetrics { }", userProfilePageView)
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
            switch(userProfilePageView.currentIndex) {
            case 1: return userSubscriptionsPageComponent
            case 2: return userInstallationsPageComponent
            default: break
            }
            return userProfilePageComponent
        }
        onPageContentItemChanged: {
            if(userProfilePageView.currentIndex !== 1)
                pageContentItem.height = availablePageContentHeight
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
                    anchors.fill: parent

                    ToolTip.text: "Ask questions, post feedback, request features and connect with other Scrite users."
                    ToolTip.visible: containsMouse
                    ToolTip.delay: 1000

                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/forum/")
                }
            }
        }
    }

    Component {
        id: userProfilePageComponent

        Item {
            id: userProfilePage

            property var userInfo: Scrite.user.info
            property RestApiCallList callList : RestApiCallList {
                calls: [refreshUserCall, deactivateDeviceCall, saveUserCall]
            }

            TabSequenceManager {
                id: userInfoFields

                property bool needsSaving: false
            }

            Connections {
                target: Scrite.user

                function onBusyChanged() { userInfoFields.needsSaving = false }
            }

            ColumnLayout {
                id: layout

                anchors.fill: parent
                anchors.margins: 20
                anchors.leftMargin: 0

                spacing: 20
                opacity: enabled ? 1 : 0.5
                enabled: !userProfilePage.callList.busy

                VclLabel {
                    Layout.fillWidth: true

                    text: "You're logged in via <b>" + userProfilePage.userInfo.email + "</b>."
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                VclTextField {
                    id: nameField

                    Layout.fillWidth: true

                    TabSequenceItem.manager: userInfoFields
                    TabSequenceItem.sequence: 0

                    text: userProfilePage.userInfo.fullName
                    maximumLength: 128
                    placeholderText: "Name"
                    undoRedoEnabled: true

                    onTextEdited: userInfoFields.needsSaving = true
                }

                VclTextField {
                    id: experienceField

                    Layout.fillWidth: true

                    TabSequenceItem.manager: userInfoFields
                    TabSequenceItem.sequence: 1

                    text: userProfilePage.userInfo.experience
                    maximumLength: 128
                    maxVisibleItems: 6
                    placeholderText: "Experience"
                    undoRedoEnabled: true
                    completionStrings: [
                        "Hobby Writer",
                        "Actively Pursuing a Writing Career",
                        "Working Writer",
                        "Have Produced Credits"
                    ]
                    maxCompletionItems: -1
                    minimumCompletionPrefixLength: 0

                    onTextEdited: userInfoFields.needsSaving = true
                }

                RowLayout {
                    Layout.fillWidth: true

                    VclTextField {
                        id: cityField

                        Layout.fillWidth: true

                        TabSequenceItem.manager: userInfoFields
                        TabSequenceItem.sequence: 2

                        text: userProfilePage.userInfo.city
                        maximumLength: 128
                        placeholderText: "City"
                        undoRedoEnabled: true

                        onTextEdited: userInfoFields.needsSaving = true
                    }

                    VclTextField {
                        id: countryField

                        Layout.fillWidth: true

                        placeholderText: "Country"
                        text: userProfilePage.userInfo.country
                        readOnly: true
                    }
                }

                VclTextField {
                    id: wdyhasField

                    Layout.fillWidth: true

                    TabSequenceItem.manager: userInfoFields
                    TabSequenceItem.sequence: 3

                    text: userProfilePage.userInfo.wdyhas
                    placeholderText: "Where did you hear about Scrite?"
                    maximumLength: 128
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
                    undoRedoEnabled: true

                    onTextEdited: userInfoFields.needsSaving = true
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 25

                    VclCheckBox {
                        id: chkAnalyticsConsent

                        TabSequenceItem.manager: userInfoFields
                        TabSequenceItem.sequence: 4

                        text: "Send analytics data."
                        checked: userProfilePage.userInfo.consentToActivityLog
                        padding: 0

                        onToggled: userInfoFields.needsSaving = true
                    }

                    VclCheckBox {
                        id: chkEmailConsent

                        TabSequenceItem.manager: userInfoFields
                        TabSequenceItem.sequence: 5

                        text: "Send marketing email."
                        checked: userProfilePage.userInfo.consentToEmail
                        padding: 0

                        onToggled: userInfoFields.needsSaving = true
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                StackLayout {
                    currentIndex: userInfoFields.needsSaving ? 1 : 0

                    Layout.fillWidth: true

                    RowLayout {
                        Layout.fillWidth: true

                        spacing: 20

                        VclButton {
                            text: "Refresh"
                            onClicked: refreshUserCall.call()

                            UserMeRestApiCall {
                                id: refreshUserCall
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        VclButton {
                            text: "Logout"

                            onClicked: deactivateDeviceCall.call()

                            InstallationDeactivateRestApiCall {
                                id: deactivateDeviceCall

                                onFinished: {
                                    if(!hasError)
                                        Announcement.shout(Runtime.announcementIds.userAccountDialogScreen, "AccountEmailScreen")
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
                                var names = nameField.text.trim().split(' ')

                                const _lastName = names.length > 1 ? names[names.length-1] : ""
                                if(names.length > 1)
                                    names.pop()
                                const _firstName = names.join(" ")
                                const locale = Scrite.locale

                                const newInfo = {
                                    firstName: _firstName,
                                    lastName: _lastName,
                                    experience: experienceField.text.trim(),
                                    city: cityField.text.trim(),
                                    country: locale.country.name,
                                    currency: locale.currency.code,
                                    wdyhas: wdyhasField.text.trim(),
                                    consentToActivityLog: chkAnalyticsConsent.checked,
                                    consentToEmail: chkEmailConsent.checked,
                                }

                                saveUserCall.updatedFields = newInfo
                                saveUserCall.call()
                            }

                            UserMeRestApiCall {
                                id: saveUserCall
                                onFinished: {
                                    updatedFields = {}
                                    userInfoFields.needsSaving = false
                                }
                            }
                        }
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent

                running: userProfilePage.callList.busy
            }
        }
    }

    Component {
        id: userSubscriptionsPageComponent

        Item {
            id: userSubscriptionsPage

            height: Math.max(userSubsView.height, userProfilePageView.availablePageContentHeight)

            property RestApiCallList callList: RestApiCallList {
                calls: [queryUserSubsCall]
            }

            Item {
                id: userSubsView
                width: parent.width
                height: userSubsLayout.height + 40

                enabled: !userSubscriptionsPage.callList.busy
                opacity: enabled ? 1 : 0.5

                ColumnLayout {
                    id: userSubsLayout

                    y: 20
                    width: parent.width-20
                    visible: !queryUserSubsCall.hasError

                    spacing: 30

                    // Active Subscription
                    Loader {
                        Layout.fillWidth: true

                        active: queryUserSubsCall.activeSubscription !== undefined
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Active Subscription"

                            PlanCard {
                                property var activeSub: queryUserSubsCall.activeSubscription

                                width: parent.width

                                name: activeSub.plan.title
                                duration: Utils.dateSpanAsString(Utils.todayWithZeroTime(), new Date(activeSub.until))
                                durationNote: "(" + Utils.formatDateRangeAsString(new Date(activeSub.from), new Date(activeSub.until)) + ")"
                                price: "Active"
                                priceNote: Utils.toTitleCase(activeSub.kind)
                                actionLink: "Details »"
                                actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                onActionLinkClicked: SubscriptionDetailsDialog.launch(activeSub)
                            }
                        }
                    }

                    // Upcoming Subscription (if any)
                    Loader {
                        Layout.fillWidth: true

                        active: queryUserSubsCall.upcomingSubscription !== undefined
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Upcoming Subscription"

                            PlanCard {
                                property var upcomingSub: queryUserSubsCall.upcomingSubscription

                                width: parent.width

                                name: upcomingSub.plan.title
                                duration: Utils.dateSpanAsString(new Date(upcomingSub.from), new Date(upcomingSub.until))
                                durationNote: "(" + Utils.formatDateRangeAsString(new Date(upcomingSub.from), new Date(upcomingSub.until)) + ")"
                                price: Utils.toTitleCase(upcomingSub.kind)
                                priceNote: Utils.dateSpanAsString(Utils.todayWithZeroTime(), new Date(upcomingSub.from))
                                actionLink: "Details »"
                                actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                onActionLinkClicked: SubscriptionDetailsDialog.launch(upcomingSub)
                            }
                        }
                    }

                    // Available Plans (if any)
                    Loader {
                        id: availablePlansLoader
                        Layout.fillWidth: true

                        active: queryUserSubsCall.availablePlans.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Available Plans"

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: queryUserSubsCall.availablePlans

                                    PlanCard {
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.title
                                        duration: Utils.daysSpanAsString(modelData.duration)
                                        durationNote: modelData.featureNote
                                        price: {
                                            if(modelData.pricing.price === 0)
                                                return "FREE"

                                            const currencySymbol = Scrite.currencySymbol(modelData.pricing.currency)
                                            return currencySymbol + modelData.pricing.price + " *"
                                        }
                                        priceNote: modelData.subtitle
                                        actionLink: Utils.toTitleCase(modelData.action.kind) + " »"
                                        onActionLinkClicked: SubscriptionPlanOperations.subscribeTo(modelData)
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        visible: queryUserSubsCall.availablePlans.length > 0

                        VclLabel {
                            Layout.fillWidth: true

                            text: "* All prices are subject to change without notice."
                            wrapMode: Text.WordWrap
                        }

                        Link {
                            enabled: SubscriptionPlanOperations.taxonomy !== undefined
                            text: queryUserSubsCall.availablePlans.length > 1 ? "Compare Plans »" : "Feature Details »"
                            font.bold: true
                            onClicked: {
                                const name = Scrite.user.info.fullName === "" ? Scrite.user.info.email : Scrite.user.info.fullName
                                const title = (queryUserSubsCall.availablePlans.length > 1 ? "Plan Comparison for " : "Plan Details for ") + name
                                SubscriptionPlanComparisonDialog.launch(queryUserSubsCall.availablePlans, title)
                            }
                        }
                    }

                    // Subscrition History
                    Loader {
                        Layout.fillWidth: true

                        active: queryUserSubsCall.pastSubscriptions.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Subscription History"

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: queryUserSubsCall.pastSubscriptions

                                    PlanCard {
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.plan.title
                                        duration: Utils.dateSpanAsString(new Date(modelData.from), new Date(modelData.until))
                                        durationNote: Utils.formatDateRangeAsString(new Date(modelData.from), new Date(modelData.until))
                                        price: Utils.toTitleCase(modelData.kind)
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
                    visible: queryUserSubsCall.hasError
                    onClicked: queryUserSubsCall.go()
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: !queryUserSubsCall.ready || queryUserSubsCall.busy
            }

            Connections {
                target: Scrite.restApi

                function onSessionTokenAvailable() {
                    queryUserSubsCall.go()
                }
            }

            SubscriptionPlansRestApiCall {
                id: queryUserSubsCall

                property bool ready: false
                property var activeSubscription
                property var upcomingSubscription
                property var pastSubscriptions: []
                property var availablePlans: []

                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage, () => { userProfilePageView.currentIndex = 0 })
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
                    ready = call()
                    if(!ready)
                        Utils.execLater(queryUserSubsCall, 500, go)
                }

                Component.onCompleted: go()
            }
        }
    }

    Component {
        id: userInstallationsPageComponent

        Item {
            id: userInstallationsPage

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                anchors.leftMargin: 0

                spacing: 10

                VclLabel {
                    Layout.fillWidth: true

                    wrapMode: Text.WordWrap
                    text: "Installations of Scrite linked to your account:"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    enabled: !deactivateOtherCall.busy && !Scrite.user.busy
                    opacity: enabled ? 1 : 0.5

                    color: Runtime.colors.primary.c10.background
                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    ListView {
                        id: installationsView

                        anchors.fill: parent
                        anchors.margins: 1

                        ScrollBar.vertical: VclScrollBar {
                            flickable: installationsView
                        }

                        clip: true
                        model: Scrite.user.info.installations
                        spacing: 20
                        currentIndex: -1

                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        delegate: Item {
                            required property int index
                            required property var modelData

                            width: installationsView.width
                            height: installationsViewDelegateLayout.height + 20

                            MouseArea {
                                anchors.fill: parent
                                onClicked: installationsView.currentIndex = index
                            }

                            RowLayout {
                                id: installationsViewDelegateLayout

                                anchors.centerIn: parent

                                width: parent.width-10

                                ColumnLayout {
                                    Layout.fillWidth: true

                                    spacing: 5

                                    VclLabel {
                                        Layout.fillWidth: true

                                        font.bold: modelData.isCurrent
                                        text: modelData.platform + " " + modelData.platformVersion + " (" + modelData.platformType + ")"
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: "Runs Scrite " + modelData.appVersion
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: "Since: " + Scrite.app.relativeTime(new Date(modelData.creationDate))
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: "Last Login: " + Scrite.app.relativeTime(new Date(modelData.lastSessionDate))
                                        elide: Text.ElideRight
                                    }
                                }

                                FlatToolButton {
                                    id: logoutButton
                                    iconSource: "qrc:/icons/action/logout.png"
                                    enabled: modelData.activated && !modelData.isCurrent
                                    opacity: enabled ? 1 : 0.2
                                    onClicked: {
                                        deactivateOtherCall.installationId = modelData.id
                                        deactivateOtherCall.call()
                                    }
                                }
                            }
                        }
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: deactivateOtherCall.busy || Scrite.user.busy
                    }
                }
            }

            InstallationDeactivateOtherRestApiCall {
                id: deactivateOtherCall
            }
        }
    }
}