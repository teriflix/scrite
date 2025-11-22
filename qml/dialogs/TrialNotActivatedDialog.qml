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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    function launch() {
        if(!Scrite.user.loggedIn || Scrite.user.info.hasTrialSubscription || Scrite.user.info.hasActiveSubscription)
            return null

        return doLaunch()
    }

    name: "TrialNotActivatedDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: {
            if(SubscriptionPlanOperations.taxonomy)
                return "Don't Miss Out: Activate Your " + SubscriptionPlanOperations.taxonomy["trialPeriod"] + "-day Free Trial Now!"

            return "Don't Miss Out: Activate Your Free Trial Now!"
        }

        width: 640
        height: 550
        titleBarCloseButtonVisible: false

        onOpened: Scrite.window.closeButtonVisible = false
        onClosed: Scrite.window.closeButtonVisible = true

        DelayedPropertyBinder {
            set: Scrite.user.info.hasTrialSubscription || Scrite.user.info.hasActiveSubscription
            onGetChanged: {
                if(get)
                    dialog.close()
            }
        }

        content: Item {
            ButtonGroup {
                id: buttonGroup
            }

            ColumnLayout {
                anchors.centerIn: parent

                width: parent.width-40
                spacing: 20

                enabled: !apiCalls.busy
                opacity: enabled ? 1 : 0.5

                VclLabel {
                    Layout.fillWidth: true

                    text: "No credit card needed. Explore all features with zero commitment."
                    font.bold: true
                    wrapMode: Text.WordWrap
                }

                ColumnLayout {
                    VclRadioButton {
                        id: activateTrialNowButton

                        Layout.fillWidth: true
                        ButtonGroup.group: buttonGroup

                        text: "I want to activate my free trial now."
                    }

                    VclRadioButton {
                        id: activateTrialLaterButton

                        Layout.fillWidth: true
                        ButtonGroup.group: buttonGroup

                        text: "I'll activate the trial later."
                    }

                    VclRadioButton {
                        id: subscriptionRequirementButton

                        Layout.fillWidth: true
                        ButtonGroup.group: buttonGroup

                        text: "I didn’t realize a subscription is required after the trial."
                    }

                    VclRadioButton {
                        id: othersButton

                        Layout.fillWidth: true
                        ButtonGroup.group: buttonGroup

                        text: "Other (please specify)"
                    }

                    ScrollView {
                        id: othersReasonView

                        Layout.fillWidth: true
                        Layout.leftMargin: othersButton.indicator.width + othersButton.spacing + parent.spacing
                        Layout.preferredHeight: 80

                        enabled: othersButton.checked
                        opacity: enabled ? 1 : 0.5

                        TextArea {
                            id: otherReason

                            width: othersReason.width-17

                            wrapMode: Text.WordWrap
                            placeholderText: "Your input helps us improve. Any feedback will be appreciated!"
                            onEnabledChanged: {
                                if(enabled)
                                    forceActiveFocus()
                            }
                        }
                    }
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    enabled: buttonGroup.checkState !== Qt.Unchecked &&
                             (otherReason.enabled ? otherReason.text !== "" : true)

                    text: {
                        if(activateTrialNowButton.checked)
                            return "Activate Trial Now »"

                        if(othersButton.checked)
                            return "Share Feedback »"

                        return "Submit »"
                    }

                    onClicked: {
                        if(activateTrialNowButton.checked) {
                            activateTrialApi.call()
                            return
                        }

                        if(activateTrialLaterButton.checked)
                            reasonApi.reason = activateTrialLaterButton.text
                        else if(subscriptionRequirementButton.checked)
                            reasonApi.reason = subscriptionRequirementButton.text
                        else if(othersButton.checkable)
                            reasonApi.reason = otherReason.text

                        reasonApi.call()
                    }
                }
            }

            RestApiCallList {
                id: apiCalls
                calls: [activateTrialApi,reasonApi]
            }

            BusyIndicator {
                running: apiCalls.busy
                anchors.centerIn: parent
            }

            SubscriptionPlanActivationRestApiCall {
                id: activateTrialApi
                api: SubscriptionPlanOperations.taxonomy.trialActivationApi

                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage)
                    } else {
                        UserAccountDialog.launch("Subscriptions")
                        dialog.close()
                    }
                }
            }

            SubscriptionTrialDeclineReasonApiCall {
                id: reasonApi

                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage)
                    } else {
                        _private.closeMainWindow()
                        dialog.close()
                    }
                }
            }
        }
    }

    QtObject {
        id: _private

        function closeMainWindow() {
            Qt.callLater(() => { Scrite.window.close() })
        }
    }
}
