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


import "../helpers"
import "../globals"
import "../controls"
import "./settingsdialog"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "StructureIndexCardFieldsDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        title: "Customise Index Card Fields"
        width: Math.min(Scrite.window.width-80, 1050)
        height: Math.min(Scrite.window.height-80, 750)

        content: PageView {
            id: _pageView
            pagesArray: ["This Document", "Default Global"]
            currentIndex: 0
            pageContent: Loader {
                width: _pageView.availablePageContentWidth
                height: _pageView.availablePageContentHeight
                sourceComponent: _pageView.currentIndex === 0 ? _thisDocumentPage : _defaultGlobalPage
            }
        }
    }

    Component {
        id: _thisDocumentPage

        StructureIndexCardFieldsPage {
            target: e_CurrentDocumentTarget
        }
    }

    Component {
        id: _defaultGlobalPage

        StructureIndexCardFieldsPage {
            target: e_DefaultGlobalTarget
        }
    }
}
