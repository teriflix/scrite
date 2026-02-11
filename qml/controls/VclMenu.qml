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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

Menu {
    id: root

    property bool autoWidth: true

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    closePolicy: Popup.CloseOnEscape|Popup.CloseOnPressOutside

    onAboutToShow: Qt.callLater(determineWidth)

    function determineWidth() {
        if(autoWidth)
            Runtime.execLater(root, Runtime.stdAnimationDuration/2, __determineWidth)
    }

    function __determineWidth() {
        if(autoWidth) {
            let maxWidth = 0
            for(let i=0; i<count; i++) {
                let menuItem = itemAt(i)
                maxWidth = Math.max(menuItem.implicitWidth, maxWidth)
            }
            width = maxWidth + leftPadding + rightPadding
        }
    }
}
