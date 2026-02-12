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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/dialogs/settingsdialog"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "StructureStoryBeatsDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "Customise Story Beats"
        width: Math.min(Scrite.window.width-80, 1050)
        height: Math.min(Scrite.window.height-80, 750)

        content: PageView {
            id: pageView
            pagesArray: ["This Document", "Default Global"]
            currentIndex: 0
            pageContent: Loader {
                width: pageView.availablePageContentWidth
                height: pageView.availablePageContentHeight
                sourceComponent: pageView.currentIndex === 0 ? thisDocumentPage : defaultGlobalPage
            }
        }
    }

    Component {
        id: thisDocumentPage

        StructureStoryBeatsPage {
            target: e_CurrentDocumentTarget
        }
    }

    Component {
        id: defaultGlobalPage

        StructureStoryBeatsPage {
            target: e_DefaultGlobalTarget
        }
    }
}
