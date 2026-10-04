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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../controls"
import "../../helpers"

SettingsTabPageView {
    id: root

    objectName: "ApplicationSettingsTab"

    pagesModel: _pagesModel

    ListModel {
        id: _pagesModel

        ObjectRegister.name: "Scrite.App.SettingsDialog.ApplicationTabPages"

        ListElement {
            title: "Options"
            qmlSource: "./ApplicationOptionsPage.qml"
        }

        ListElement {
            title: "Theme"
            qmlSource: "./ApplicationThemePage.qml"
        }

        ListElement {
            title: "Shortcuts"
            qmlSource: "./ApplicationShortcutsPage.qml"
            fillHeight: true
            noContentSpacing: true
        }
    }
}
