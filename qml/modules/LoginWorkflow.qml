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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/modules"

Item {
    id: root

    parent: Scrite.window.contentItem
    anchors.fill: parent

    function init(_parent) { parent = _parent }
    function launch() {
        loginWizard.screenName = Scrite.user.loggedIn ? "UserProfileScreen" : _private.startScreen
        loginWizard.open()
    }

    VclDialog {
        id: loginWizard

        property string screenName: _private.startScreen
        property Item screenItem: contentInstance ? contentInstance.item : null

        width: 800
        height: 520
        title: screenItem ? screenItem.title : "Activation Workflow"

        onOpened: {
            MessageBox.discardMessageBoxes()
            requiresRestartCall.go()
        }

        titleBarCloseButtonVisible: screenItem ? screenItem.modal === false : false
        content: Loader {
            source: "qrc:/qml/modules/loginworkflow/" + loginWizard.screenName + ".qml"
        }

        Announcement.onIncoming: (type, data) => {
            if(type === Runtime.announcementIds.loginRequest) {
                if(typeof data === "string" && data !== "")
                    screenName = data
                else
                    screenName = _private.startScreen

                if(!visible)
                    loginWizard.open()
            } else if(type === Runtime.announcementIds.loginWorkflowScreen) {
                if(data && data !== "")
                    screenName = data
                else
                    screenName = Scrite.user.loggedIn ? "UserProfileScreen" : _private.startScreen

                if(!visible)
                    loginWizard.open()
            }
        }
    }

    Connections {
        target: Scrite.user
        enabled: !loginWizard.visible

        function onForceLoginRequest() {
            loginWizard.screenName = "AccountEmailScreen"
            loginWizard.open()
        }
    }

    Loader {
        active: {
            if(loginWizard.visible)
                return loginWizard.screenItem ? loginWizard.screenItem.checkForUserProfileErrors : false
            return Scrite.user.loggedIn
        }
        sourceComponent: Item {
            property ErrorReport userErrorReport: Aggregation.findErrorReport(Scrite.app)
            property bool hasUserError: userErrorReport.hasError

            onHasUserErrorChanged: {
                if(hasUserError) {
                    const msg = userErrorReport.errorMessage
                    userErrorReport.clear()

                    Utils.execLater(root, 100, () => {
                                        SplashScreen.closeSingleInstance()
                                        MessageBox.information("User Profile Error", msg, () => { root.launch() } )
                                    })
                }
            }

            Connections {
                target: Scrite.user

                function onLoggedInChanged() {
                    if(Scrite.user.loggedIn)
                        Utils.execLater(_private, 1000, _private.checkIfSubscriptionIsAboutToExpire)
                    else
                        _private.restartRequest()
                }

                function onSubscriptionsChanged() {
                    Utils.execLater(_private, 1000, _private.checkIfSubscriptionIsAboutToExpire)
                }
            }
        }
    }

    Loader {
        active: {
            if(loginWizard.visible)
                return loginWizard.screenItem ? loginWizard.screenItem.checkForRestartRequest : false
            return Scrite.user.loggedIn
        }
        sourceComponent: Item {
            Component.onCompleted: Utils.execLater(requiresRestartCall, 1000, requiresRestartCall.go)

            Timer {
                id: requiresRestartCallTimer
                running: false
                repeat: false
                interval: loginWizard.visible ? 10000 : 300000
                onTriggered: requiresRestartCall.go()
            }

            JsonHttpRequest {
                id: requiresRestartCall
                type: JsonHttpRequest.POST
                api: "app/requiresRestart"
                token: ""

                function go() {
                    const _email = email()
                    if(_email === "") {
                        requiresRestartCallTimer.start()
                        return
                    }

                    data = {
                        "email": _email,
                        "token": loginToken(),
                        "deviceId": deviceId(),
                        "clientId": clientId()
                    }
                    requiresRestartCallTimer.stop()
                    call()
                }

                onFinished: {
                    if(hasError) {
                        return
                    }

                    if(hasResponse) {
                        const r = responseData
                        if(r.requiresRestart === true) {
                            _private.restartRequest()
                        } else {
                            requiresRestartCallTimer.start()
                        }
                    }
                }
            }
        }
    }

    QtObject {
        id: _private

        property string startScreen: Runtime.loginWorkflowSettings.welcomeScreenShown ? "AccountEmailScreen" : "WelcomeScreen"
        property bool subscriptionReminderShown: {
            const today = Utils.formatDateIncludingYear(Utils.todayWithZeroTime())
            return today === Runtime.loginWorkflowSettings.lastSubscriptionReminderDate
        }

        function restartRequest(msg) {
            if(msg === undefined)
                msg = "There has been a change in your account information. Please restart Scrite and login again."
            MessageBox.discardMessageBoxes()
            MessageBox.information("Requires Restart",
                                   msg,
                                   () => {
                                       let call = Qt.createQmlObject("import io.scrite.components 1.0; JsonHttpRequest { }", _private)
                                       call.store("devices", undefined)
                                       call.store("subscriptions", undefined)
                                       call.store("userInfo", undefined)
                                       call.store("loginToken", undefined)
                                       call.store("sessionToken", undefined)
                                       Qt.quit()
                                   } )
        }

        function checkIfSubscriptionIsAboutToExpire() {
            if(subscriptionReminderShown || !Scrite.user.loggedIn)
                return

            if(Scrite.user.busy) {
                Utils.execLater(_private, 1000, checkIfSubscriptionIsAboutToExpire)
                return
            }

            if(Scrite.user.hasActiveSubscription) {
                const activeSub = Scrite.user.activeSubscription
                const nrDays = Utils.daysBetween(Utils.todayWithZeroTime(), new Date(activeSub.end_date))

                if(nrDays < Runtime.subscriptionTreshold && !Scrite.user.hasUpcomingSubscription) {
                    subscriptionReminderShown = true
                    Runtime.loginWorkflowSettings.lastSubscriptionReminderDate = Utils.formatDateIncludingYear(Utils.todayWithZeroTime())

                    let msg = "Your active subscription \"" + activeSub.plan_name + "\" is about to expire in " + nrDays + " day(s). "
                    msg += "Please sign up for a paid subscription plan to avoid interruption."
                    MessageBox.question("Subscribe Now!", msg, ["View Plans", "Later"],
                                           (answer) => {
                                                if(answer === "View Plans") {
                                                    loginWizard.screenName = "UserProfileScreen"
                                                    loginWizard.open()
                                                    Utils.execLater(loginWizard, 100, () => {
                                                                        Announcement.shout(Runtime.announcementIds.userProfileScreenPage, "Subscriptions")
                                                                    })
                                                }
                                           })
                }
            }
        }
    }
}
