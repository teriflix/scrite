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

Item {
    id: root

    parent: Scrite.window.contentItem
    anchors.fill: parent

    function init(_parent) { parent = _parent }
    function launch(profileScreen) {
        userAccountDialog.screenName = Scrite.user.loggedIn ? "UserProfileScreen" : _private.startScreen
        userAccountDialog.open()

        if(Scrite.user.loggedIn && profileScreen && profileScreen !== "") {
            Utils.execLater(userAccountDialog, 500, () => {
                                Announcement.shout(Runtime.announcementIds.userProfileScreenPage, profileScreen)
                            })
        }
    }

    VclDialog {
        id: userAccountDialog

        property string screenName: _private.startScreen
        property Item screenItem: contentInstance ? contentInstance.item : null

        width: 900
        height: 620
        title: screenItem ? screenItem.title : "Activation Workflow"

        onOpened: {
            HomeScreen.closeSingleInstance()
            MessageBox.discardMessageBoxes()
        }

        onClosed: HomeScreen.firstLaunch()

        titleBarCloseButtonVisible: screenItem ? !screenItem.modal : Runtime.allowAppUsage
        content: Loader {
            source: "qrc:/qml/dialogs/useraccountdialog/" + userAccountDialog.screenName + ".qml"
        }

        Announcement.onIncoming: (type, data) => {
            if(type === Runtime.announcementIds.loginRequest) {
                if(typeof data === "string" && data !== "")
                    screenName = data
                else
                    screenName = _private.startScreen

                if(!visible)
                    userAccountDialog.open()
            } else if(type === Runtime.announcementIds.userAccountDialogScreen) {
                if(data && data !== "")
                    screenName = data
                else
                    screenName = Scrite.user.loggedIn ? "UserProfileScreen" : _private.startScreen

                if(!visible)
                    userAccountDialog.open()
            }
        }
    }

    QtObject {
        id: _private

        property string startScreen: Runtime.userAccountDialogSettings.welcomeScreenShown ? "AccountEmailScreen" : "WelcomeScreen"

        property SessionNewRestApiCall newSessionTokenCall: SessionNewRestApiCall {
            property VclDialog waitDialog

            onAboutToCall: MessageBox.discardMessageBoxes()
            onJustIssuedCall: waitDialog = WaitDialog.launch("Fetching new access tokens ...")
            onFinished: waitDialog.close()
        }

        readonly property Connections trackRestApi: Connections {
            target: Scrite.restApi

            function onNewSessionTokenRequired() {
                _private.newSessionTokenCall.queue(Scrite.restApi.sessionApiQueue)
            }

            function onFreshActivationRequired() {
                MessageBox.discardMessageBoxes()

                if(userAccountDialog.visible)
                    userAccountDialog.screenName = _private.startScreen
                else
                    MessageBox.information("Activation Required", "Please activate this installation of Scrite again.", () => {
                                               root.launch()
                                           })
            }

            function onInvalidApiKey() {
                MessageBox.discardMessageBoxes()

                MessageBox.information("Unsupported Version or Build", "This version or build of Scrite you are using is not supported anymore. Please install the latest version of Scrite from our website.", () => { Qt.quit() })
            }
        }

        readonly property Connections trackSubscriptionExpiry: Connections {
            enabled: Scrite.user.loggedIn

            target: Scrite.user

            Notification.active: false
            Notification.title: "Subscription Expiry"
            Notification.text: "Your active subscription is about to expire in a few days."
            Notification.buttons: ["View Plans", "Dismiss"]
            Notification.onButtonClicked: (index) => {
                if(index === 0) {
                    launch("Subscriptions")
                }
            }

            function onSubscriptionAboutToExpire(nrDays) {
                Notification.text = "Your active subscription is about to expire in " + nrDays + " day(s)."
                Notification.active = true
            }

            function onInfoChanged() {
                if(!Scrite.user.info.hasActiveSubscription) {
                    launch("Subscriptions")
                    return
                }

                if(Notification.active && Scrite.user.info.hasUpcomingSubscription)
                    Notification.active = false

                _private.trackSessionStatus.configure()
            }
        }

        readonly property Connections trackImportantMessages: Connections {
            enabled: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription

            target: Scrite.user

            Notification.active: false
            Notification.title: "Unread Notifications"
            Notification.text: "You have one or more important unread notifications. Would you like to see them now?"
            Notification.buttons: ["View Notifications", "Dismiss"]
            Notification.onButtonClicked: (index) => {
                if(index === 0) {
                    launch("Notifications")
                }
            }

            function onNotifyImportantMessages(messages) {
                Notification.title = (() => {
                                        const count = Scrite.user.unreadMessageCount
                                        if(count === 1)
                                          return "You have 1 unread notification"
                                        return "You have " + count + " unread notifications."
                                      })()
                Notification.text = (() => {
                                         const count = Scrite.user.unreadMessageCount
                                         let ret = messages[0].subject
                                         if(count > 1)
                                            ret += ", and " + (count-1) + " more .."
                                         return ret
                                     })()
                Notification.active = true
            }
        }

        readonly property Connections trackApplicationState: Connections {
            property bool tracking: false

            enabled: tracking && Scrite.user.loggedIn

            target: Scrite.app

            function onAppStateChanged() {
                _private.trackSessionStatus.configure()
            }

            Component.onCompleted: Utils.execLater(_private, 1000, () => {
                                                       _private.trackApplicationState.tracking = true
                                                       _private.trackSessionStatus.configure()
                                                   })
        }

        readonly property Timer trackSessionStatus: Timer {
            property bool enabled: true

            repeat: false
            running: enabled && Scrite.user.loggedIn
            interval: (requiresFrequentChecks() ? 5 : 60)*60*1000

            function requiresFrequentChecks() {
                const userInfo = Scrite.user.info
                if(!userInfo.hasActiveSubscription || userInfo.subscriptions.length === 0)
                    return true

                if(userInfo.subscriptions[0].kind === "trial" && !userInfo.hasUpcomingSubscription)
                    return true

                if(userInfo.daysToSubscribedUntil() < Runtime.subscriptionTreshold)
                    return true

                return false
            }

            function configure() {
                const state = Scrite.app.appState
                if(state === Scrite.ApplicationActive) {
                    if(requiresFrequentChecks())
                        _private.sessionStatusApi.call()
                    _private.trackSessionStatus.enabled = true
                } else {
                    _private.trackSessionStatus.enabled = false
                }
            }

            onTriggered: {
                _private.sessionStatusApi.queue(Scrite.restApi.sessionApiQueue)
            }
        }

        readonly property SessionStatusRestApiCall sessionStatusApi: SessionStatusRestApiCall {
            onFinished: {
                if(hasResponse && !hasError) {
                    _private.trackSessionStatus.start()
                }
            }
        }
    }
}
