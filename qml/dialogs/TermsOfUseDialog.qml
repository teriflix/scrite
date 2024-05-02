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
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "TermsOfUseDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "Terms Of Use"
        width: {
            const bgImageAspectRatio = 1464.0/978.0
            return height * bgImageAspectRatio * 0.9
        }
        height: {
            const bgImageHeight = 978
            return Math.min(bgImageHeight*0.8, Scrite.window.height * 0.8) * 0.9
        }

        content: TextEdit {
            padding: 40
            readOnly: true
            font.family: "Courier Prime"
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            text: Scrite.app.fileContents(":/LICENSE.txt")
            selectByMouse: true
        }
    }
}
