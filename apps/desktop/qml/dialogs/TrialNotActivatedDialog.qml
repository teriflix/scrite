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

pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

// TODO: Needs to be reviewed and tested.

DialogLauncher {
    id: root

    function launch() {
        if(!Scrite.user.loggedIn || Scrite.user.info.hasTrialSubscription || Scrite.user.info.hasActiveSubscription || Runtime.requiresUserOnboarding())
            return null

        return doLaunch()
    }

    name: "TrialNotActivatedDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

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

        DelayedProperty {
            set: Scrite.user.info.hasTrialSubscription || Scrite.user.info.hasActiveSubscription
            onGetChanged: {
                if(get)
                    _dialog.close()
            }
        }

        content: Item {
            ButtonGroup {
                id: _buttonGroup
            }

            ColumnLayout {
                anchors.centerIn: parent

                width: parent.width-40
                spacing: 20

                enabled: !_apiCalls.busy
                opacity: enabled ? 1 : 0.5

                VclLabel {
                    Layout.fillWidth: true

                    text: "No credit card needed. Explore all features with zero commitment."
                    font.bold: true
                    wrapMode: Text.WordWrap
                }

                ColumnLayout {
                    VclRadioButton {
                        id: _activateTrialNowButton

                        Layout.fillWidth: true
                        ButtonGroup.group: _buttonGroup

                        text: "I want to activate my free trial now."
                    }

                    VclRadioButton {
                        id: _activateTrialLaterButton

                        Layout.fillWidth: true
                        ButtonGroup.group: _buttonGroup

                        text: "I'll activate the trial later."
                    }

                    VclRadioButton {
                        id: _subscriptionRequirementButton

                        Layout.fillWidth: true
                        ButtonGroup.group: _buttonGroup

                        text: "I didn’t realize a subscription is required after the trial."
                    }

                    VclRadioButton {
                        id: _othersButton

                        Layout.fillWidth: true
                        ButtonGroup.group: _buttonGroup

                        text: "Other (please specify)"
                    }

                    ScrollView {
                        id: _othersReasonView

                        Layout.fillWidth: true
                        Layout.leftMargin: _othersButton.indicator.width + _othersButton.spacing + parent.spacing
                        Layout.preferredHeight: 80

                        enabled: _othersButton.checked
                        opacity: enabled ? 1 : 0.5

                        TextArea {
                            id: _otherReason

                            width: _othersReasonView.width-17

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

                    enabled: _buttonGroup.checkState !== Qt.Unchecked &&
                             (_otherReason.enabled ? _otherReason.text !== "" : true)

                    text: {
                        if(_activateTrialNowButton.checked)
                            return "Activate Trial Now »"

                        if(_othersButton.checked)
                            return "Share Feedback »"

                        return "Submit »"
                    }

                    onClicked: {
                        if(_activateTrialNowButton.checked) {
                            _activateTrialApi.call()
                            return
                        }

                        if(_activateTrialLaterButton.checked)
                            _reasonApi.reason = _activateTrialLaterButton.text
                        else if(_subscriptionRequirementButton.checked)
                            _reasonApi.reason = _subscriptionRequirementButton.text
                        else if(_othersButton.checkable)
                            _reasonApi.reason = _otherReason.text

                        _reasonApi.call()
                    }
                }
            }

            RestApiCallList {
                id: _apiCalls
                calls: [_activateTrialApi,_reasonApi]
            }

            BusyIndicator {
                running: _apiCalls.busy
                anchors.centerIn: parent
            }

            SubscriptionPlanActivationRestApiCall {
                id: _activateTrialApi

                api: SubscriptionPlanOperations.taxonomy.trialActivationApi

                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage)
                    } else {
                        UserAccountDialog.launch("Subscriptions")
                        _dialog.close()
                    }
                }
            }

            SubscriptionTrialDeclineReasonApiCall {
                id: _reasonApi

                onFinished: {
                    if(hasError) {
                        MessageBox.information("Error", errorMessage)
                    } else {
                        _private.closeMainWindow()
                        _dialog.close()
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
