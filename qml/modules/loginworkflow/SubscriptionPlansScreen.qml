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
    readonly property bool modal: true
    readonly property string title: {
        if(queryPlansCall.busy || queryPlansCall.hasError)
            return "Subscription Plans"

        let userName = [queryPlansCall.responseData.firstName, queryPlansCall.responseData.lastName].join(" ").trim()
        if(userName === "")
            userName = queryPlansCall.responseData.email
        return "Subscription Plans for " + userName
    }
    readonly property bool checkForRestartRequest: true
    readonly property bool checkForUserProfileErrors: false

    Image {
        anchors.fill: parent
        source: "qrc:/images/loginworkflowbg.png"
        fillMode: Image.PreserveAspectCrop
    }

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 16
        anchors.leftMargin: 22
        anchors.rightMargin: 175
        anchors.bottomMargin: 16

        clip: plansViewScroll.needed

        color: Qt.rgba(0,0,0,0)
        border.color: Runtime.colors.primary.borderColor
        border.width: clip ? 1 : 0

        BusyIndicator {
            anchors.centerIn: parent
            running: queryPlansCall.busy
        }

        JsonHttpRequest {
            id: queryPlansCall
            type: JsonHttpRequest.POST
            token: ""
            api: "plans/list"
            data: {
                "email": email(),
                "currency": Scrite.locale.currency.code,
                "country": Scrite.locale.country.name,
                "includeHistory": "true"
            }
            reportNetworkErrors: true
            onFinished: {
                if(hasError) {
                    MessageBox.question("Error", errorMessage,
                                        ["Try Again", "Quit"],
                                        (answer) => {
                                            if(answer === "Try Again")
                                                queryPlansCall.cal()
                                            else
                                                Qt.quit()
                                        })
                    return
                }
            }

            Component.onCompleted: call()
        }

        Flickable {
            id: plansView

            ScrollBar.vertical: VclScrollBar {
                id: plansViewScroll
            }

            anchors.fill: parent
            anchors.margins: 1

            clip: ScrollBar.vertical.needed
            contentWidth: width
            contentHeight: plansViewContent.height

            ColumnLayout {
                id: plansViewContent

                width: plansView.width - (plansViewScroll.needed ? 20 : 0)
                spacing: 30

                Loader {
                    Layout.fillWidth: true

                    active: !queryPlansCall.busy && queryPlansCall.hasResponse && queryPlansCall.responseData.activePlan !== null
                    visible: active

                    sourceComponent: VclGroupBox {
                        title: "Active Subscription"

                        PlanCard {
                            property var activePlan: queryPlansCall.responseData.activePlan

                            width: parent.width

                            name: activePlan.plan_name
                            duration: Utils.dateSpanAsString(new Date(), new Date(activePlan.end_date))
                            durationNote: "(" + Utils.formatDateRangeAsString(new Date(activePlan.start_date), new Date(activePlan.end_date)) + ")"
                            price: "Active"
                            priceNote: Utils.toTitleCase(activePlan.plan_kind)
                            actionLink: "Use »"
                            onActionLinkClicked: Announcement.shout(Runtime.announcementIds.loginWorkflowScreen, "AccountEmailScreen")
                        }
                    }
                }

                Loader {
                    Layout.fillWidth: true

                    active: !queryPlansCall.busy && queryPlansCall.hasResponse && queryPlansCall.responseData.upcomingPlan !== null
                    visible: active

                    sourceComponent: VclGroupBox {
                        title: "Upcoming Subscription"

                        PlanCard {
                            property var upcomingPlan: queryPlansCall.responseData.upcomingPlan

                            width: parent.width

                            name: upcomingPlan.plan_name
                            duration: Utils.dateSpanAsString(new Date(upcomingPlan.start_date), new Date(upcomingPlan.end_date))
                            durationNote: "(" + Utils.formatDateRangeAsString(new Date(upcomingPlan.start_date), new Date(upcomingPlan.end_date)) + ")"
                            price: Utils.toTitleCase(upcomingPlan.plan_kind)
                            priceNote: "Starts in " + Utils.dateSpanAsString(new Date(), new Date(upcomingPlan.start_date))
                            actionLink: "Details »"
                        }
                    }
                }

                Loader {
                    id: availablePlansLoader
                    Layout.fillWidth: true

                    active: !queryPlansCall.busy && queryPlansCall.hasResponse && queryPlansCall.responseData.plans && queryPlansCall.responseData.plans.length > 0
                    visible: active

                    sourceComponent: VclGroupBox {
                        title: "Available Plans"

                        ColumnLayout {
                            width: parent.width
                            spacing: 20

                            Repeater {
                                model: queryPlansCall.responseData.plans

                                PlanCard {
                                    required property var modelData

                                    Layout.fillWidth: true

                                    name: modelData.name
                                    duration: modelData.duration
                                    durationNote: {
                                        if(modelData.enabled)
                                            return "(" + Utils.formatDateRangeAsString(new Date(), modelData.durationInDays) + ")"
                                        return modelData.reason
                                    }
                                    price: {
                                        const currencySymbol = Scrite.currencySymbol(queryPlansCall.responseData.currency)
                                        if(modelData.regularPrice && modelData.regularPrice !== modelData.price)
                                            return "<s>" + currencySymbol + modelData.regularPrice + "</s>&nbsp;&nbsp;&nbsp;" +
                                                    "<b>" + currencySymbol + modelData.price + "</b> *"
                                        return currencySymbol + modelData.price + " *"
                                    }
                                    priceNote: modelData.note
                                    actionLink: "Buy »"
                                    actionLinkEnabled: modelData.enabled
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
                    bottomPadding: 8
                }

                // Subscrition History
                Loader {
                    Layout.fillWidth: true

                    active: !queryPlansCall.busy && queryPlansCall.hasResponse && queryPlansCall.responseData.history && queryPlansCall.responseData.history.length > 0
                    visible: active

                    sourceComponent: VclGroupBox {
                        title: "Subscription History"

                        ColumnLayout {
                            width: parent.width
                            spacing: 20

                            Repeater {
                                model: queryPlansCall.responseData.history

                                PlanCard {
                                    required property var modelData

                                    Layout.fillWidth: true

                                    name: modelData.plan_name
                                    duration: Utils.dateSpanAsString(new Date(modelData.start_date), new Date(modelData.end_date))
                                    durationNote: Utils.formatDateRangeAsString(new Date(modelData.start_date), new Date(modelData.end_date))
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
    }
}
