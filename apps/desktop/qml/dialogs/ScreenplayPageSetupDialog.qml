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

import "../globals"
import "../helpers"
import "../controls"
import "./settingsdialog"

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
