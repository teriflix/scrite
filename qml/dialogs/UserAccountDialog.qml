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
    function launch() {
        userAccountDialog.screenName = Scrite.user.loggedIn ? "UserProfileScreen" : _private.startScreen
        userAccountDialog.open()
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
                _private.newSessionTokenCall.call()
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

            function onSubscriptionAboutToExpire(nrDays) {
                MessageBox.information("Subscription Expiry", "Your active subscription is about to expire in " + nrDays + " day(s).", () => {
                                           root.launch()
                                           Utils.execLater(userAccountDialog, 100, () => {
                                                Announcement.shout(Runtime.announcementIds.userProfileScreenPage, "Subscriptions")
                                             })
                                       })
            }

            function onInfoChanged() {
                if(!Scrite.user.info.hasActiveSubscription) {
                    root.launch()
                    Utils.execLater(userAccountDialog, 100, () => {
                         Announcement.shout(Runtime.announcementIds.userProfileScreenPage, "Subscriptions")
                      })
                }
            }
        }

        readonly property Connections trackApplicationState: Connections {
            property bool tracking: false

            enabled: tracking && Scrite.user.loggedIn

            target: Scrite.app

            function onApplicationStateChanged(state) {
                if(state === Scrite.ApplicationActive) {
                    _private.sessionStatusApi.call()
                    _private.trackSessionStatus.enabled = true
                } else {
                    _private.trackSessionStatus.enabled = false
                }
            }

            Component.onCompleted: Utils.execLater(_private, 1000, () => { _private.trackApplicationState.tracking = true })
        }

        readonly property Timer trackSessionStatus: Timer {
            property bool enabled: true

            repeat: false
            running: enabled && Scrite.user.loggedIn
            interval: 30000

            onTriggered: _private.sessionStatusApi.call()
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