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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs/homescreen"

DialogLauncher {
    id: root

    function launch(mode) { return doLaunch({"mode": mode}) }

    name: "HomeScreen"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        property string mode

        width: Math.min(800, Scrite.window.width*0.9)
        height: Math.min(width*1.2, Scrite.window.height*0.9)
        title: "scrite.io"

        contentItem: HomeScreenImpl {
            mode: dialog.mode
            onCloseRequest: Qt.callLater(dialog.close)
        }

        onOpened: _private.launchCounter = _private.launchCounter + 1
        onClosed: _private.promptToJoinDiscordCommunity()

        Announcement.onIncoming: (type, data) => {
            if(type === Runtime.announcementIds.closeHomeScreenRequest || type === Runtime.announcementIds.loginRequest)
                Qt.callLater(dialog.close)
        }
    }

    QtObject {
        id: _private

        property bool joinDiscordPrompted: false
        property int launchCounter: 0

        function promptToJoinDiscordCommunity() {
            if(joinDiscordPrompted === false &&
                Runtime.applicationSettings.joinDiscordPromptCounter < 3 &&
                _private.launchCounter >= Runtime.applicationSettings.joinDiscordPromptCounter*3) {
                Utils.execLater(root, 1000, () => {
                                    _private.joinDiscordPrompted = true
                                    Runtime.applicationSettings.joinDiscordPromptCounter = Runtime.applicationSettings.joinDiscordPromptCounter+1
                                    let dlg = JoinDiscordCommunity.launch(Math.max(3-Runtime.applicationSettings.joinDiscordPromptCounter,0)*2000)
                                    if(Runtime.applicationSettings.joinDiscordPromptCounter < 3)
                                        dlg.closed.connect( () => {
                                                               if(!dlg.infoUrlOpened)
                                                                Qt.openUrlExternally(JoinDiscordCommunity.infoUrl)
                                                           })
                                })
            }
        }
    }
}
