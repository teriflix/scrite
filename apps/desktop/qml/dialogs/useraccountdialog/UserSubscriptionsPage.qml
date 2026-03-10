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

    implicitHeight: _userSubsView.height

    property RestApiCallList callList: RestApiCallList {
        calls: [_queryUserSubsCall]
    }

    Item {
        id: _userSubsView
        width: parent.width
        height: _userSubsLayout.height + 40

        enabled: !root.callList.busy
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
                                id: _delegate2
                                required property int index
                                required property var modelData

                                Layout.fillWidth: true

                                name: _delegate2.modelData.title
                                duration: Runtime.daysSpanAsString(_delegate2.modelData.duration)
                                exclusive: _delegate2.modelData.exclusive
                                durationNote: _delegate2.modelData.featureNote
                                price: {
                                    if(_delegate2.modelData.pricing.price === 0)
                                        return "FREE"

                                    const currencySymbol = Scrite.currencySymbol(_delegate2.modelData.pricing.currency)
                                    return currencySymbol + _delegate2.modelData.pricing.price + " *"
                                }
                                priceNote: _delegate2.modelData.subtitle
                                actionLink: SubscriptionPlanOperations.planActionLinkText(_delegate2.modelData)
                                onActionLinkClicked: SubscriptionPlanOperations.subscribeTo(_delegate2.modelData)
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
                                id: _delegate3
                                required property int index
                                required property var modelData

                                Layout.fillWidth: true

                                name: _delegate3.modelData.plan.title
                                duration: Runtime.dateSpanAsString(new Date(_delegate3.modelData.from), new Date(_delegate3.modelData.until))
                                exclusive: _delegate3.modelData.plan.exclusive
                                durationNote: Runtime.formatDateRangeAsString(new Date(_delegate3.modelData.from), new Date(_delegate3.modelData.until))
                                price: Runtime.toTitleCase(_delegate3.modelData.kind)
                                priceNote: _delegate3.modelData.plan.featureNote
                                actionLink: "Details »"
                                actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                onActionLinkClicked: SubscriptionDetailsDialog.launch(_delegate3.modelData)
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
