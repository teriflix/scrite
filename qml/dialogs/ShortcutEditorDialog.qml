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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/dialogs/settingsdialog"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch(lookup) { return doLaunch({"lookup": lookup}) }

    name: "ShortcutEditorDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        property string lookup
        property int beforeLanguage: -1

        width: Math.min(Scrite.window.width-80, 800)
        height: Math.min(Scrite.window.height-80, 750)

        title: "Shortcuts"

        content: ApplicationShortcutsPage {
            lookup: _dialog.lookup
        }

        onAboutToShow: {
            beforeLanguage = Runtime.language.activeCode
            Runtime.language.setActiveCode(QtLocale.English)
            contentItem.forceActiveFocus()
        }

        onAboutToHide: {
            if(beforeLanguage > 0)
                Runtime.language.setActiveCode(beforeLanguage)
            beforeLanguage = -1
        }
    }
}
