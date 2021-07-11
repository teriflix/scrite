/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Menu {
    id: thisMenu
    Material.accent: primaryColors.key
    Material.background: primaryColors.c100.background
    Material.foreground: primaryColors.c50.text
    objectName: title

    Connections {
        target: dialogUnderlay
        onVisibleChanged: {
            if(dialogUnderlay.visible)
                thisMenu.close()
        }
    }
}
