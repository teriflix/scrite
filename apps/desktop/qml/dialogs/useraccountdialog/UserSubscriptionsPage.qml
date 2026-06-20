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
            width: parent.width - 20
            visible: !_queryUserSubsCall.hasError

            spacing: 30

            // Available Plans
            Loader {
                id: _availablePlansLoader
                Layout.fillWidth: true
                active: _queryUserSubsCall.availablePlans.length > 0
                visible: active

                sourceComponent: VclGroupBox {
                    title: "Available Plans"

                    ColumnLayout {
                        id: _plansColumn
                        width: parent.width
                        spacing: 20

                        property int bestValueIndex: -1

                        Component.onCompleted: {
                            const plans = Scrite.user.asSubscriptionPlanInfoList(_queryUserSubsCall.availablePlans)
                            let bestIdx = -1
                            let bestPct = 0
                            for (let i = 0; i < plans.length; i++) {
                                if (plans[i].exclusive) continue
                                const p = plans[i].pricing
                                if (p.actual > 0 && p.actual > p.price) {
                                    const pct = (p.actual - p.price) / p.actual
                                    if (pct > bestPct) { bestPct = pct; bestIdx = i }
                                }
                            }
                            bestValueIndex = bestIdx
                        }

                        Repeater {
                            model: Scrite.user.asSubscriptionPlanInfoList(_queryUserSubsCall.availablePlans)

                            delegate: PlanCard {
                                id: _planDelegate
                                required property int index
                                required property scriteUserSubscriptionPlanInfo modelData

                                Layout.fillWidth: true

                                name: _planDelegate.modelData.title
                                duration: Runtime.daysSpanAsString(_planDelegate.modelData.duration)
                                exclusive: _planDelegate.modelData.exclusive
                                durationNote: _planDelegate.modelData.featureNote
                                price: {
                                    const p = _planDelegate.modelData.pricing
                                    return p.price === 0 ? "FREE" : Scrite.currencySymbol(p.currency) + p.price
                                }
                                actualPrice: {
                                    const p = _planDelegate.modelData.pricing
                                    return (p.actual > 0 && p.actual > p.price)
                                           ? Scrite.currencySymbol(p.currency) + p.actual
                                           : ""
                                }
                                savingsLabel: {
                                    const p = _planDelegate.modelData.pricing
                                    return (p.actual > 0 && p.actual > p.price)
                                           ? Math.round((1 - p.price / p.actual) * 100) + "% discount"
                                           : ""
                                }
                                isBestValue: _planDelegate.index === _plansColumn.bestValueIndex
                                priceNote: _planDelegate.modelData.subtitle
                                actionLink: SubscriptionPlanOperations.planActionLinkText(_planDelegate.modelData)
                                onActionLinkClicked: SubscriptionPlanOperations.subscribeTo(_planDelegate.modelData)
                            }
                        }
                    }
                }

                Link {
                    anchors.right: parent.right
                    anchors.bottom: parent.top
                    anchors.bottomMargin: _availablePlansLoader.topPadding / 3
                    z: 100
                    text: _queryUserSubsCall.responseData.referralCodeText + " »"
                    font.bold: true
                    visible: _queryUserSubsCall.responseData.acceptingReferralCode === true
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
                                width: parent.width - 50
                                spacing: 20

                                TextField {
                                    id: _txtReferralCode
                                    Layout.fillWidth: true
                                    focus: true
                                    maximumLength: Runtime.bounded(_queryUserSubsCall.responseData.minReferralCodeLength,
                                                                   _queryUserSubsCall.responseData.maxReferralCodeLength, 128)
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
                                    if (hasError) { MessageBox.information("Error", errorText); return }
                                    if (hasResponse) { _queryUserSubsCall.go(); _referralCodeDialog.close() }
                                }
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
                    text: "Plans & prices are subject to change without notice."
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

            // Current Subscription: shown when there is an active plan.
            // Header toggles between single-plan layout and two-card layout.
            // Feature table is a single shared instance.
            Loader {
                Layout.fillWidth: true
                active: _queryUserSubsCall.activeSubscription !== undefined
                visible: active

                sourceComponent: VclGroupBox {
                    title: "Current Status"

                    ColumnLayout {
                        id: _subGroup
                        width: parent.width
                        spacing: 16

                        // Resolved subscription objects. _queryUserSubsCall is file-scope.
                        readonly property scriteUserSubscriptionInfo activeSub: Scrite.user.asSubscriptionInfo(_queryUserSubsCall.activeSubscription)
                        readonly property bool hasUpcoming: _queryUserSubsCall.upcomingSubscription !== undefined
                        readonly property scriteUserSubscriptionInfo upcomingSub: _subGroup.hasUpcoming
                                                                                  ? Scrite.user.asSubscriptionInfo(_queryUserSubsCall.upcomingSubscription)
                                                                                  : Scrite.user.asSubscriptionInfo({})

                        // ── Single-plan header (no upcoming) ──────────────────────────────
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16
                            visible: !_subGroup.hasUpcoming

                            // Left: plan identity
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                VclLabel {
                                    Layout.fillWidth: true
                                    text: _subGroup.activeSub.plan.title
                                    font.bold: true
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                                    wrapMode: Text.WordWrap
                                }

                                VclLabel {
                                    Layout.fillWidth: true
                                    text: _subGroup.activeSub.plan.subtitle
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    visible: text !== ""
                                    wrapMode: Text.WordWrap
                                }

                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Row {
                                        spacing: 4

                                        VclLabel {
                                            text: Runtime.daysSpanAsString(_subGroup.activeSub.plan.duration) +
                                                  "  ·  Device Count: " + _subGroup.activeSub.plan.devices
                                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                        }

                                        VclLabel {
                                            text: "ⓘ"
                                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    const n = _subGroup.activeSub.plan.devices
                                                    const opening = n === 1
                                                        ? "This plan allows Scrite to be activated on 1 device at a time."
                                                        : "This plan allows Scrite to be activated on up to " + n + " devices at a time."
                                                    MessageBox.question(
                                                        "About Device Activations",
                                                        opening + "\n\nEach activation is tied to the specific OS installation and user account used at sign-in. This means reinstalling your OS, or signing in from a different user account on the same machine, counts as a separate activation.\n\nTo activate on a new device once the limit is reached, sign out from one of your existing activated devices first. Exceeding the limit without doing so will result in an activation error.",
                                                        ["More Info", "Ok"],
                                                        (btn) => {
                                                            if (btn === "More Info") {
                                                                const url = HelpCenter.lookup("device limits")
                                                                if (url.toString() !== "") Qt.openUrlExternally(url)
                                                            }
                                                        }
                                                    )
                                                }
                                            }
                                        }
                                    }

                                    Row {
                                        spacing: 4
                                        visible: !Scrite.isFeatureNameEnabled("support/email", _subGroup.activeSub.plan.features)

                                        VclLabel {
                                            text: "·  Discord community support only."
                                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                        }

                                        VclLabel {
                                            text: "ⓘ"
                                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: JoinDiscordCommunity.launch()
                                            }
                                        }
                                    }
                                }

                                VclLabel {
                                    Layout.fillWidth: true
                                    text: "★  Exclusive Plan"
                                    font.bold: true
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    visible: _subGroup.activeSub.plan.exclusive
                                }
                            }

                            // Right: status badge, dates, order link
                            ColumnLayout {
                                spacing: 4

                                VclLabel {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Active"
                                    font.family: Runtime.shortcutFontMetrics.font.family
                                    font.bold: true
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 10
                                    color: Runtime.colors.primary.c700.background
                                }

                                VclLabel {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: Runtime.formatDateIncludingYear(new Date(_subGroup.activeSub.from)) +
                                          "  —  " +
                                          Runtime.formatDateIncludingYear(new Date(_subGroup.activeSub.until))
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                }

                                Link {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Order #" + _subGroup.activeSub.wc_order_id + " »"
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                    visible: _subGroup.activeSub.wc_order_id !== undefined &&
                                             _subGroup.activeSub.wc_order_id !== ""
                                    onClicked: Qt.openUrlExternally(_subGroup.activeSub.detailsUrl)
                                }
                            }
                        }

                        // ── Two-card header (active + upcoming) ───────────────────────────
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            visible: _subGroup.hasUpcoming

                            SubCard {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                subscription: _subGroup.activeSub
                                badgeText: "Current Plan"
                                badgeColor: Runtime.colors.primary.c700.background
                                color: Runtime.colors.primary.c100.background
                                border.color: Runtime.colors.primary.c400.background
                                statusLine: "Ends on " +
                                            Runtime.formatDateIncludingYear(new Date(_subGroup.activeSub.until)) +
                                            "  (" + _subGroup.activeSub.daysToUntil + " days remaining)"
                            }

                            SubCard {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                subscription: _subGroup.upcomingSub
                                badgeText: "Upcoming Plan"
                                badgeColor: Runtime.colors.primary.c500.background
                                color: Runtime.colors.primary.c50.background
                                border.color: Runtime.colors.primary.c300.background
                                statusLine: "Begins on " +
                                            Runtime.formatDateIncludingYear(new Date(_subGroup.upcomingSub.from)) +
                                            "  (" + _subGroup.upcomingSub.daysToFrom + " days from now)"
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: Runtime.colors.primary.c400.background
                        }

                        // ── Feature table — single instance, always from UserInfo ─────────
                        Item {
                            id: _featureTable
                            Layout.fillWidth: true
                            Layout.preferredHeight: 260

                            ListModel { id: _includedModel }
                            ListModel { id: _excludedModel }

                            Component.onCompleted: {
                                if (!SubscriptionPlanOperations.taxonomy) return
                                const available = Scrite.user.info.availableFeatures
                                const allEnabled = available.indexOf("*") >= 0

                                SubscriptionPlanOperations.taxonomy.features.forEach((feature) => {
                                    if (feature.group === true) return
                                    if (feature.display === false) return

                                    const enabled = allEnabled || available.indexOf(feature.name) >= 0
                                    if (enabled)
                                        _includedModel.append({ "featureTitle": feature.title, "featureDescription": feature.description })
                                    else
                                        _excludedModel.append({ "featureTitle": feature.title, "featureDescription": feature.description })
                                })
                            }

                            RowLayout {
                                anchors.fill: parent
                                spacing: 10

                                // Included
                                FeatureListPanel {
                                    id: _includedPanel
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    headerText: "✓   Included"
                                    headerBgColor: Runtime.colors.tx(Runtime.colors.accent.c600.background)
                                    headerTextColor: Runtime.colors.accent.c600.text
                                    headerBorderWidth: 1
                                    listModel: _includedModel
                                    highlightColor: Runtime.colors.accent.c200.background
                                    listInteractive: false
                                }

                                // Not Included
                                FeatureListPanel {
                                    id: _excludedPanel
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    headerText: "✗   Not Included"
                                    headerBgColor: Runtime.colors.primary.c600.background
                                    headerTextColor: Runtime.colors.primary.c600.text
                                    listModel: _excludedModel
                                    highlightColor: Runtime.colors.primary.c300.background
                                    titlePrefix: "✗  "
                                    visible: _excludedModel.count > 0
                                    listInteractive: false
                                }
                            }
                        }
                    }
                }
            }

            // Subscription History
            Loader {
                Layout.fillWidth: true
                active: _queryUserSubsCall.pastSubscriptions.length > 0
                visible: active

                sourceComponent: VclGroupBox {
                    title: "Plan History"

                    ColumnLayout {
                        width: parent.width
                        spacing: 20

                        Repeater {
                            model: Scrite.user.asSubscriptionInfoList(_queryUserSubsCall.pastSubscriptions)

                            delegate: PlanCard {
                                id: _histDelegate
                                required property int index
                                required property scriteUserSubscriptionInfo modelData

                                Layout.fillWidth: true

                                name: _histDelegate.modelData.plan.title
                                duration: Runtime.dateSpanAsString(new Date(_histDelegate.modelData.from), new Date(_histDelegate.modelData.until))
                                exclusive: _histDelegate.modelData.plan.exclusive
                                durationNote: Runtime.formatDateRangeAsString(new Date(_histDelegate.modelData.from), new Date(_histDelegate.modelData.until))
                                price: Runtime.toTitleCase(_histDelegate.modelData.kind)
                                priceNote: _histDelegate.modelData.plan.featureNote
                                actionLink: "Details »"
                                useFixedFontForPrice: false
                                actionLinkEnabled: SubscriptionPlanOperations.taxonomy !== undefined
                                onActionLinkClicked: SubscriptionDetailsDialog.launch(_histDelegate.modelData)
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
        function onSessionTokenAvailable() { _queryUserSubsCall.go() }
    }

    SubscriptionPlansRestApiCall {
        id: _queryUserSubsCall

        property bool ready: false
        property var activeSubscription
        property var upcomingSubscription
        property var pastSubscriptions: []
        property var availablePlans: []

        onFinished: {
            if (hasError) {
                MessageBox.information("Error", errorMessage, () => { _userProfilePageView.currentIndex = 0 })
                return
            }

            const result = responseData
            if (result.hasActiveSubscription)
                activeSubscription = result.subscriptions[0]
            if (result.hasUpcomingSubscription)
                upcomingSubscription = result.subscriptions[1]

            let history = []
            result.subscriptions.forEach((sub) => { if (sub.hasExpired) history.push(sub) })
            if (result.publicBetaSubscription)
                history.push(result.publicBetaSubscription)

            pastSubscriptions = history
            availablePlans = result.plans
        }

        function go() {
            if (busy) return
            ready = call()
            if (!ready)
                Runtime.execLater(_queryUserSubsCall, 500, go)
        }

        Component.onCompleted: go()
    }

    component SubCard: Rectangle {
        id: _subCard

        property scriteUserSubscriptionInfo subscription
        property string badgeText
        property color badgeColor
        property string statusLine

        implicitHeight: _subCardContent.implicitHeight + 24
        radius: 4
        border.width: 1

        ColumnLayout {
            id: _subCardContent
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
            spacing: 4

            VclLabel {
                Layout.fillWidth: true
                text: _subCard.badgeText
                font.bold: true
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                color: _subCard.badgeColor
            }

            VclLabel {
                Layout.fillWidth: true
                text: _subCard.subscription.plan.title
                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                wrapMode: Text.WordWrap
            }

            VclLabel {
                Layout.fillWidth: true
                text: _subCard.subscription.plan.subtitle
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                font.italic: true
                wrapMode: Text.WordWrap
                visible: text !== ""
            }

            Row {
                spacing: 4

                VclLabel {
                    text: Runtime.daysSpanAsString(_subCard.subscription.plan.duration) +
                          "  ·  Device Count: " + _subCard.subscription.plan.devices
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                }

                VclLabel {
                    text: "ⓘ"
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            const n = _subCard.subscription.plan.devices
                            const opening = n === 1
                                ? "This plan allows Scrite to be activated on 1 device at a time."
                                : "This plan allows Scrite to be activated on up to " + n + " devices at a time."
                            MessageBox.question(
                                "About Device Activations",
                                opening + "\n\nEach activation is tied to the specific OS installation and user account used at sign-in. This means reinstalling your OS, or signing in from a different user account on the same machine, counts as a separate activation.\n\nTo activate on a new device once the limit is reached, sign out from one of your existing activated devices first. Exceeding the limit without doing so will result in an activation error.",
                                ["More Info", "Ok"],
                                (btn) => {
                                    if (btn === "More Info") {
                                        const url = HelpCenter.lookup("device limits")
                                        if (url.toString() !== "") Qt.openUrlExternally(url)
                                    }
                                }
                            )
                        }
                    }
                }
            }

            VclLabel {
                Layout.fillWidth: true
                text: _subCard.statusLine
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                wrapMode: Text.WordWrap
            }

            Link {
                text: "Order #" + _subCard.subscription.wc_order_id + " »"
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                visible: _subCard.subscription.wc_order_id !== undefined &&
                         _subCard.subscription.wc_order_id !== ""
                onClicked: Qt.openUrlExternally(_subCard.subscription.detailsUrl)
            }
        }
    }

}
