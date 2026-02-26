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

import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/dialogs/settingsdialog"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch() { return doLaunch() }

    name: "LanguageOptionsDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "Language Settings"
        width: Math.min(Scrite.window.width-80, 830)
        height: Math.min(Scrite.window.height-80, 750)

        content: LanguageOptionsPage { }
    }
}
