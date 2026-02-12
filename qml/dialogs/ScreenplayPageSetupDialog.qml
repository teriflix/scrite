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

    function launch() { return doLaunch() }

    name: "ScreenplayPageSetupDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        width: Math.min(Scrite.window.width-80, 800)
        height: Math.min(Scrite.window.height-80, 750)

        title: "Page Setup"

        content: Flickable {
            id: _flickable

            contentWidth: _pageContainer.width
            contentHeight: _pageContainer.height

            ScrollBar.vertical: VclScrollBar { }

            Item {
                id: _pageContainer

                width: _flickable.width
                height: _page.height

                ScreenplayPageSetupPage {
                    id: _page

                    x: 20
                    width: parent.width-20
                }
            }
        }
    }
}
