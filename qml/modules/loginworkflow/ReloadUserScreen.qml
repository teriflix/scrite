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

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"

Item {
    readonly property bool modal: true
    readonly property string title: "Fetching Profile Data ..."
    readonly property bool checkForRestartRequest: false
    readonly property bool checkForUserProfileErrors: false

    Connections {
        target: Scrite.user

        function onLoggedInChanged() {
            if(Scrite.user.loggedIn)
                Announcement.shout(Runtime.announcementIds.loginWorkflowScreen, "UserProfileScreen")
        }
    }

    Connections {
        target: Aggregation.findErrorReport(Scrite.user)

        function onHasErrorChanged() {
            if(target.hasError)
                MessageBox.question("Error",
                                    "There was an error fetching user profile data. Please try again.\n\n" + target.errorMessage,
                                    ["Try Again", "Quit Scrite"],
                                   (answer) => {
                                        if(answer === "Try Again")
                                            Scrite.user.reload()
                                        else
                                            Qt.quit()
                                   })
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: Scrite.user.busy
    }

    Component.onCompleted: Scrite.user.reload()
}
