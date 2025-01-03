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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    readonly property bool modal: true
    readonly property string title: "Welcome to Scrite!"

    Image {
        anchors.fill: parent
        source: "qrc:/images/useraccountdialogbg.png"
        fillMode: Image.PreserveAspectCrop
        opacity: 0.25
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20

        spacing: 10
        enabled: !sendActivationCodeCall.busy
        opacity: enabled ? 1 : 0.5

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                anchors.margins: -5

                color: Runtime.colors.primary.c50.background
                opacity: 0.5
                border.width: plansSubsView.ScrollBar.vertical.needed ? 1 : 0
                border.color: Runtime.colors.primary.borderColor
            }

            Flickable {
                id: plansSubsView

                anchors.fill: parent
                anchors.margins: 0

                clip: ScrollBar.vertical.needed
                contentWidth: contentHeight > height ? width - 20 : width
                contentHeight: plansSubsViewLayout.height

                ScrollBar.vertical: VclScrollBar { flickable: plansSubsView }

                ColumnLayout {
                    id: plansSubsViewLayout

                    width: plansSubsView.contentWidth
                    spacing: 30

                    VclLabel {
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap

                        text: _private.userMeta.name === "" ? "Hello, there!" : "Hello " + _private.userMeta.name + ","
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap

                        text: {
                            let ret = ""
                            if(_private.userMeta.hasActiveSubscription) {
                                ret += "You have an active subscription plan as outlined below. "
                                if(_private.userMeta.plans.length > 0)
                                    ret += "Upon verifying this installation, you can either continue using your active subscrition or subscribe to any of the plans listed below."
                                else
                                    ret += "Upon verifying this installation, your active subscription will be enabled on this device."
                            } else {
                                ret += "You have no active subscrition plan. "
                                ret += "Upon verifying this installation, please subscribe or activate any of the plans listed below in order to use Scrite."
                            }

                            return ret
                        }
                    }

                    Loader {
                        Layout.fillWidth: true

                        active: _private.userMeta.subscriptions.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Subscriptions"

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: _private.userMeta.subscriptions

                                    PlanCard {
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.plan.title
                                        duration: Utils.dateSpanAsString(modelData.isActive ? Utils.todayWithZeroTime() : new Date(modelData.from), new Date(modelData.until))
                                        exclusive: modelData.plan.exclusive
                                        durationNote: "(" + Utils.formatDateRangeAsString(new Date(modelData.from), new Date(modelData.until)) + ")"
                                        price: modelData.isActive ? "Active" : "Upcoming"
                                        priceNote: Utils.toTitleCase(modelData.kind)
                                        actionLink: "Details »"
                                        actionLinkEnabled: false
                                        actionLinkVisible: false
                                    }
                                }
                            }
                        }
                    }

                    Loader {
                        Layout.fillWidth: true

                        active: _private.userMeta.plans.length > 0
                        visible: active

                        sourceComponent: VclGroupBox {
                            title: "Plans"

                            ColumnLayout {
                                width: parent.width
                                spacing: 20

                                Repeater {
                                    model: _private.userMeta.plans

                                    PlanCard {
                                        required property var modelData

                                        Layout.fillWidth: true

                                        name: modelData.title
                                        duration: Utils.daysSpanAsString(modelData.duration)
                                        exclusive: modelData.exclusive
                                        durationNote: modelData.featureNote
                                        price: {
                                            if(modelData.pricing.price === 0)
                                                return "FREE"

                                            const currencySymbol = Scrite.currencySymbol(modelData.pricing.currency)
                                            return currencySymbol + modelData.pricing.price + " *"
                                        }
                                        priceNote: modelData.subtitle
                                        actionLink: Utils.toTitleCase(modelData.action.kind) + " »"
                                        actionLinkEnabled: false
                                        actionLinkVisible: false
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 20

            Item {
                Layout.fillWidth: true

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.width

                    visible: _private.userMeta.plans.length > 0
                    text: "* All prices are subject to change without notice."
                    wrapMode: Text.WordWrap
                }
            }

            VclButton {
                text: "Continue »"
                onClicked: sendActivationCodeCall.call()
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: sendActivationCodeCall.busy
    }

    AppRequestActivationCodeRestApiCall {
        id: sendActivationCodeCall
        onFinished: {
            if(hasError || !hasResponse) {
                const errMsg = hasError ? errorMessage : "Couldn't request activation code. Please try again."
                MessageBox.information("Error", errMsg)
                return
            }

            Announcement.shout(Runtime.announcementIds.userAccountDialogScreen, "ActivationCodeScreen")
        }
    }

    QtObject {
        id: _private

        readonly property var userMeta: Session.get("checkUserResponse")
    }
}
