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
import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch() {
        var dlg = dialogComponent.createObject(root)
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch StructureIndexCardFieldsDialog")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            title: "Customise Index Card Fields"
            width: Math.min(Scrite.window.width-80, 1050)
            height: Math.min(Scrite.window.height-80, 750)

            content: PageView {
                id: pageView
                pagesArray: ["This Document", "Default Global"]
                currentIndex: 0
                pageContent: Loader {
                    width: pageView.availablePageContentWidth
                    height: pageView.availablePageContentHeight
                    source: "./settingsdialog/StructureIndexCardFieldsPage.qml"
                    onLoaded: item.target = Qt.binding( () => { return pageView.currentIndex })
                }
            }
        }
    }
}
