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

Item {
    readonly property bool modal: true
    readonly property string title: "Fetching Profile Data ..."

    Connections {
        target: Scrite.user

        function onLoggedInChanged() {
            _private.maybeShowUserProfileScreen()
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: Scrite.user.busy
    }

    Component.onCompleted: Utils.execLater(_private, 100, _private.maybeShowUserProfileScreen)

    QtObject {
        id: _private

        function maybeShowUserProfileScreen() {
            if(Scrite.user.loggedIn)
                Announcement.shout(Runtime.announcementIds.userAccountDialogScreen, "UserProfileScreen")
        }
    }
}
