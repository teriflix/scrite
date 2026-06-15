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

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "ViewLicenseDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        title: "License"

        width: Math.min(880, Scrite.window.width * 0.85)
        height: Math.min(680, Scrite.window.height * 0.85)

        content: Item {
            ScrollView {
                anchors.fill: parent
                anchors.margins: 16
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TextArea {
                    text: Scrite.licenseText()
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.NoWrap
                    font.family: "Courier Prime"
                    font.pointSize: 10
                    background: Item { }
                }
            }
        }
    }
}
