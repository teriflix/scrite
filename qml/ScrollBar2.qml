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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

ScrollBar {
    property Flickable flickable
    property int contentSize: flickable ? (orientation === Qt.Vertical ? flickable.contentHeight : flickable.contentWidth) : 0
    property int actualSize: flickable ? (orientation === Qt.Vertical ? flickable.height : flickable.width) : 0
    property bool needed: contentSize > actualSize
    policy: needed ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    minimumSize: 0.1
    palette {
        mid: Qt.rgba(0,0,0,0.25)
        dark: Qt.rgba(0,0,0,0.75)
    }
    opacity: active ? 1 : 0.4
    Behavior on opacity {
        enabled: applicationSettings.enableAnimations
        NumberAnimation { duration: 250 }
    }
}
