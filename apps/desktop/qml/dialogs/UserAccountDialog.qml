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

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

Item {
    id: root

    parent: Scrite.window.contentItem
    anchors.fill: parent

    function init(_parent) { parent = _parent }
    function launch(profileScreen) {
        _userAccountDialog.screenName = Scrite.user.loggedIn ? (Runtime.requiresUserOnboarding() ? "UserOnboardingScreen" : "UserProfileScreen") : _private.startScreen
        _userAccountDialog.open()

        if(Scrite.user.loggedIn && profileScreen && profileScreen !== "" && !Runtime.requiresUserOnboarding()) {
            Runtime.execLater(_userAccountDialog, 500, () => {
                                Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, profileScreen)
                            })
        }
    }

    function handleMessageEndpoint(endpoint) {
        switch(endpoint) {
        case "$subscribe":
            launch("Subscriptions")
            return
        case "$profile":
            launch("Profile")
            return
        case "$installations":
            launch("Installations")
            return
        case "$homescreen":
            HomeScreen.launch()
            return
        }
    }

    VclDialog {
        id: _userAccountDialog

        property string screenName: _private.startScreen
        property Item screenItem: contentInstance ? contentInstance.item : null

        width: 900
        height: 620
        objectName: "UserAccountDialog"

        handleLanguageShortcuts: true
        title: screenItem ? screenItem.title : "Activation Workflow"

        onOpened: {
            HomeScreen.closeSingleInstance()
            MessageBox.discardMessageBoxes()
        }

        onClosed: HomeScreen.firstLaunch()

        titleBarCloseButtonVisible: screenItem ? !screenItem.modal : Runtime.allowAppUsage
        content: Loader {
            source: "./useraccountdialog/" + _userAccountDialog.screenName + ".qml"
        }

        Announcement.onIncoming: (type, data) => {
            if(type === Runtime.announcementIds.loginRequest) {
                if(typeof data === "string" && data !== "")
                    screenName = data
                else
                    screenName = _private.startScreen

                if(!visible)
                    _userAccountDialog.open()
            } else if(type === Runtime.announcementIds.userAccountDialogScreen) {
                if(data && data !== "")
                    screenName = data
                else
                    screenName = Scrite.user.loggedIn ? (Runtime.requiresUserOnboarding() ? "UserOnboardingScreen" : "UserProfileScreen") : _private.startScreen

                if(!visible)
                    _userAccountDialog.open()
            }
        }
    }

    QtObject {
        id: _private

        property string startScreen: "WelcomeScreen" // Runtime.userAccountDialogSettings.welcomeScreenShown ? "AccountEmailScreen" : "WelcomeScreen"

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

                if(_userAccountDialog.visible)
                    _userAccountDialog.screenName = _private.startScreen
                else
                    MessageBox.information("Activation Required", "Please activate this installation of Scrite again.", () => {
                                               root.launch()
                                           })
            }

            function onInvalidApiKey() {
                MessageBox.discardMessageBoxes()

                MessageBox.information("Unsupported Version or Build", "This version or build of Scrite you are using is not supported anymore. Please install the latest version of Scrite from our website.",
                                       () => {
                                           Qt.openUrlExternally("https://www.scrite.io/downloads")
                                           Qt.quit()
                                       })
            }
        }

        property int unreadMessageCount: 0
        property scriteUserMessage firstMessage

        readonly property Connections trackImportantMessages: Connections {
            enabled: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription

            target: Scrite.user

            Notification.active: false
            Notification.title: _private.unreadMessageCount === 1 ? _private.firstMessage.subject : "You have " + _private.unreadMessageCount + " unread messages."
            Notification.text: _private.unreadMessageCount === 1 ? _private.firstMessage.body : (_private.firstMessage.subject + ", and " + (_private.unreadMessageCount-1) + " more ..")
            Notification.image: _private.unreadMessageCount === 1 ? _private.firstMessage.image : ""
            Notification.buttons: {
                let ret = []
                if(_private.unreadMessageCount === 1) {
                    const buttons = _private.firstMessage.buttons
                    for(let i=0; i<buttons.length; i++) {
                        ret.push(buttons[i].text)
                    }
                    ret.push("Show All Messages")
                } else {
                    ret.push("Read Messages")
                    ret.push("Dismiss")
                }

                return ret
            }
            Notification.onButtonClicked: (index) => {
                                              let offset=0
                                              if(_private.unreadMessageCount === 1) {
                                                  const buttons = _private.firstMessage.buttons
                                                  offset = buttons.length
                                                  if(index < offset) {
                                                      const button = buttons[index]
                                                      Scrite.user.markMessagesAsRead()
                                                      if(button.action === UserMessageButton.UrlAction) {
                                                          Qt.openUrlExternally(button.endpoint)
                                                          return
                                                      }
                                                      if(button.action === UserMessageButton.CommandAction) {
                                                          root.handleMessageEndpoint(button.endpoint)
                                                          return
                                                      }
                                                  }
                                              }

                                              if(index-offset=== 0)
                                                  root.launch("Notifications")
                                          }

            function onNotifyImportantMessages(messages) {
                _private.unreadMessageCount = Scrite.user.unreadMessageCount
                _private.firstMessage = messages[0]

                Notification.active = true
            }
        }

        readonly property Connections trackApplicationState: Connections {
            property bool tracking: false

            enabled: tracking && Scrite.user.loggedIn

            target: Scrite.app

            function onAppStateChanged() {
                const state = Scrite.app.appState
                if(state === Scrite.ApplicationActive) {
                    const checkNow = (() => {
                                          const userInfo = Scrite.user.info
                                          if(!userInfo.hasActiveSubscription || userInfo.subscriptions.length === 0)
                                              return true

                                          if(userInfo.subscriptions[0].kind === "trial" && !userInfo.hasUpcomingSubscription)
                                              return true

                                          if(userInfo.daysToSubscribedUntil() < Runtime.subscriptionTreshold)
                                              return true

                                          return false
                                      })()
                    if(checkNow)
                        _private.sessionStatusApi.queue(Scrite.restApi.sessionApiQueue)
                } else {
                    Scrite.restApi.sessionApiQueue.remove(_private.sessionStatusApi)
                }
            }

            Component.onCompleted: Runtime.execLater(_private, 1000, () => { _private.trackApplicationState.tracking = true })
        }

        readonly property SessionStatusRestApiCall sessionStatusApi: SessionStatusRestApiCall {
            // onFinished: {
            //     if(hasResponse && !hasError) {
            //         _private.trackSessionStatus.start()
            //     }
            // }
        }
    }
}
