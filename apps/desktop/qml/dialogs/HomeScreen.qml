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
import "./homescreen"

DialogLauncher {
    id: root

    function launch(mode) { return doLaunch({"mode": mode}) }
    function firstLaunch() {
        if(_private.launchCounter === 0)
            launch()
    }

    name: "HomeScreen"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        property string mode

        width: Math.min(800, Scrite.window.width*0.9)
        height: Math.min(width*1.2, Scrite.window.height*0.9)
        title: "scrite.io"

        contentItem: HomeScreenImpl {
            mode: _dialog.mode
            onCloseRequest: Qt.callLater(_dialog.close)
        }

        onOpened: _private.launchCounter = _private.launchCounter+1

        Announcement.onIncoming: (type, data) => {
            if(type === Runtime.announcementIds.closeHomeScreenRequest || type === Runtime.announcementIds.loginRequest)
                Qt.callLater(_dialog.close)
        }
    }

    QtObject {
        id: _private

        property int launchCounter: 0
    }
}
