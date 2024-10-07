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
    readonly property bool modal: false
    property string title: {
        if(Scrite.user.loggedIn) {
            if(Scrite.user.info.firstName && Scrite.user.info.firstName !== "")
                return "Hi, " + Scrite.user.info.firstName + "."
            if(Scrite.user.info.lastName && Scrite.user.info.lastName !== "")
                return "Hi, " + Scrite.user.info.lastName + "."
        }
        return "Hi, there."
    }
    readonly property bool checkForRestartRequest: true
    readonly property bool checkForUserProfileErrors: true

    PageView {
        id: userProfilePageView
        anchors.fill: parent

        pagesArray: ["Profile", "Subscriptions", "Installations"]
        currentIndex: 0
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
            case 1: return userSubscriptionsPage
            case 2: return userInstallationsPage
            default: break
            }
            return userProfilePage
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
        id: userProfilePage

        Item {
            TabSequenceManager {
                id: userInfoFields

                property bool needsSaving: false
            }

            Connections {
                target: Scrite.user

                function onBusyChanged() { userInfoFields.needsSaving = false }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                anchors.leftMargin: 0

                spacing: 20
                opacity: Scrite.user.busy ? 0.5 : 1
                enabled: !Scrite.user.busy

                VclLabel {
                    Layout.fillWidth: true

                    text: "You're logged in via <b>" + Scrite.user.email + "</b>."
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

                    text: Scrite.user.fullName
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

                    text: Scrite.user.experience
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

                VclTextField {
                    id: locationField

                    Layout.fillWidth: true

                    TabSequenceItem.manager: userInfoFields
                    TabSequenceItem.sequence: 2

                    text: Scrite.user.location
                    maximumLength: 128
                    placeholderText: "Location (City, Country)"
                    undoRedoEnabled: true
                    completionStrings: Scrite.user.locations
                    completionAcceptsEnglishStringsOnly: false
                    minimumCompletionPrefixLength: 0

                    onTextEdited: userInfoFields.needsSaving = true
                }

                VclTextField {
                    id: wdyhasField

                    Layout.fillWidth: true

                    TabSequenceItem.manager: userInfoFields
                    TabSequenceItem.sequence: 3

                    text: Scrite.user.wdyhas
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
                        checked: Scrite.user.info.consent.activity
                        padding: 0

                        onToggled: userInfoFields.needsSaving = true
                    }

                    VclCheckBox {
                        id: chkEmailConsent

                        TabSequenceItem.manager: userInfoFields
                        TabSequenceItem.sequence: 5

                        text: "Send marketing email."
                        checked: Scrite.user.info.consent.email
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
                            onClicked: Scrite.user.reload()
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        VclButton {
                            text: "Logout"

                            onClicked: {
                                Scrite.user.logout()
                                if(!Scrite.user.loggedIn)
                                    Announcement.shout(Runtime.announcementIds.loginWorkflowScreen, "AccountEmailScreen")
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
                                    location: locationField.text.trim(),
                                    wdyhas: wdyhasField.text.trim(),
                                    consent: {
                                        activity: chkAnalyticsConsent.checked,
                                        email: chkEmailConsent.checked
                                    },
                                    country: locale.country.name,
                                    currency: locale.currency.code
                                }

                                Scrite.user.update(newInfo)
                            }
                        }
                    }
                }

            }

            BusyIndicator {
                anchors.centerIn: parent

                running: Scrite.user.busy
            }
        }
    }

    Component {
        id: userSubscriptionsPage

        Item {
            height: Math.max(userSubsView.height, 100)

            Item {
                id: userSubsView
                width: parent.width
                height: userSubsLayout.height + 40

                ColumnLayout {
                    id: userSubsLayout

                    y: 20
                    width: parent.width-20

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

                                name: activeSub.plan_name
                                duration: Utils.dateSpanAsString(Utils.todayWithZeroTime(), new Date(activeSub.end_date))
                                durationNote: "(" + Utils.formatDateRangeAsString(new Date(activeSub.start_date), new Date(activeSub.end_date)) + ")"
                                price: "Active"
                                priceNote: Utils.toTitleCase(activeSub.plan_kind)
                                actionLink: "Details »"
                                onActionLinkClicked: Qt.openUrlExternally(activeSub.details)
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

                                name: upcomingSub.plan_name
                                duration: Utils.dateSpanAsString(new Date(upcomingSub.start_date), new Date(upcomingSub.end_date))
                                durationNote: "(" + Utils.formatDateRangeAsString(new Date(upcomingSub.start_date), new Date(upcomingSub.end_date)) + ")"
                                price: Utils.toTitleCase(upcomingSub.plan_kind)
                                priceNote: Utils.dateSpanAsString(Utils.todayWithZeroTime(), new Date(upcomingSub.start_date))
                                actionLink: "Details »"
                                onActionLinkClicked: Qt.openUrlExternally(upcomingSub.details)
                            }
                        }
                    }

                    // Available Plans (if any)
                    Loader {
                        id: availablePlansLoader
                        Layout.fillWidth: true

                        active: queryUserPlansCall.plans.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Available Plans"

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: queryUserPlansCall.plans

                                    PlanCard {
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.name
                                        duration: modelData.duration
                                        durationNote: {
                                            if(modelData.enabled)
                                            return "(" + Utils.formatDateRangeAsString(Utils.todayWithZeroTime(), modelData.durationInDays) + ")"
                                            return modelData.reason
                                        }
                                        price: {
                                            const currencySymbol = Scrite.currencySymbol(Scrite.user.currency)
                                            if(modelData.regularPrice && modelData.regularPrice !== modelData.price)
                                                return "<s>" + currencySymbol + modelData.regularPrice + "</s>&nbsp;&nbsp;&nbsp;" +
                                                       "<b>" + currencySymbol + modelData.price + "</b> *"
                                            return currencySymbol + modelData.price + " *"
                                        }
                                        priceNote: modelData.note
                                        actionLink: "Buy »"
                                        enabled: modelData.enabled
                                        onActionLinkClicked: Qt.openUrlExternally(modelData.shop)
                                    }
                                }
                            }
                        }
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        visible: availablePlansLoader.active
                        text: "* All prices are subject to change without notice."
                        wrapMode: Text.WordWrap
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

                                        name: modelData.plan_name
                                        duration: Utils.formatDateRangeAsString(new Date(modelData.start_date), new Date(modelData.end_date))
                                        price: Utils.toTitleCase(modelData.plan_kind)
                                        actionLink: "Details »"
                                        onActionLinkClicked: Qt.openUrlExternally(modelData.details)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: queryUserSubsCall.busy || queryUserPlansCall.busy
            }

            JsonHttpRequest {
                id: queryUserSubsCall

                property var activeSubscription
                property var upcomingSubscription
                property var pastSubscriptions: []

                type: JsonHttpRequest.GET
                api: "user/subscriptions"
                reportNetworkErrors: true
                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage, () => {
                                                    Announcement.shout(_private.userProfilePageRequest, undefined)
                                               })
                    }

                    if(hasResponse) {
                        const list = responseData.list
                        const today = Utils.todayWithZeroTime()
                        let subHistory = []
                        list.forEach( (item) => {
                                        if(item.active)
                                            activeSubscription = item
                                        else if(new Date(item.start_date) > today)
                                             upcomingSubscription = item
                                        else
                                            subHistory.push(item)
                                     })
                        pastSubscriptions = subHistory
                        clearResponse()
                    }
                }
                Component.onCompleted: call()
            }

            JsonHttpRequest {
                id: queryUserPlansCall

                property var plans: []

                type: JsonHttpRequest.GET
                api: "plans/list"
                reportNetworkErrors: true
                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage, () => {
                                                    userProfilePageView.currentIndex = 0
                                               })
                    }

                    if(hasResponse) {
                        const response = responseData
                        if(response.upcomingPlan === null) {
                            plans = response.plans
                        }
                        clearResponse()
                    }
                }
                Component.onCompleted: call()
            }
        }
    }

    Component {
        id: userInstallationsPage

        Item {
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
                        model: Scrite.user.installations
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

                            RowLayout {
                                id: installationsViewDelegateLayout

                                anchors.centerIn: parent

                                width: parent.width-10

                                ColumnLayout {
                                    Layout.fillWidth: true

                                    spacing: 5

                                    VclLabel {
                                        Layout.fillWidth: true

                                        font.bold: index === Scrite.user.currentInstallationIndex
                                        text: modelData.platform + " " + modelData.platformVersion + " (" + modelData.platformType + ")"
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: "Runs Scrite " + modelData.appVersions[0]
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: "Since: " + Scrite.app.relativeTime(new Date(modelData.firstActivationDate))
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: "Last Login: " + Scrite.app.relativeTime(new Date(modelData.lastActivationDate))
                                        elide: Text.ElideRight
                                    }
                                }

                                FlatToolButton {
                                    id: logoutButton
                                    iconSource: "qrc:/icons/action/logout.png"
                                    enabled: index !== Scrite.user.currentInstallationIndex
                                    opacity: enabled ? 1 : 0.2
                                    onClicked: {
                                        busyOverlay.busyMessage = "Logging out of selected installation ..."
                                        Scrite.user.deactivateInstallation(modelData._id)
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: installationsView.currentIndex = index
                            }
                        }
                    }
                }

            }
        }
    }
}
