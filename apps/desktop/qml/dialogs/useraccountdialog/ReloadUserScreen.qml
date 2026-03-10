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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components


import "../../globals"
import ".."

Item {
    id: root
    readonly property bool modal: true
    readonly property string title: "Fetching Profile Data ..."

    Connections {
        target: Scrite.user

        function onLoggedInChanged() {
            if(_root_2.timer && _root_2.timer.running) {
                _root_2.timer.stop()
                _root_2.timer.destroy()
            }

            _root_2.maybeShowUserProfileScreen()
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: Scrite.user.busy
    }

    Component.onCompleted: {
        _root_2.timer = Runtime.execLater(_root_2, 100, _root_2.maybeShowUserProfileScreen)
    }

    QtObject {
        id: _root_2

        property Timer timer

        function maybeShowUserProfileScreen() {
            if(Scrite.user.loggedIn) {
                const requiresOnboarding = Runtime.requiresUserOnboarding()
                if(requiresOnboarding) {
                    Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "UserOnboardingScreen")
                } else {
                    Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "UserProfileScreen")
                }
            }
        }
    }
}
