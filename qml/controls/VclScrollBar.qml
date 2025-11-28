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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

ScrollBar {
    id: root

    property bool needed: DelayedProperty.get
    property Flickable flickable

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    DelayedProperty.initial: false
    DelayedProperty.set: (flickable ? (orientation === Qt.Vertical ? flickable.contentHeight : flickable.contentWidth) : 0) > (flickable ? (orientation === Qt.Vertical ? flickable.height : flickable.width) : 0)

    Component.onCompleted: {
        if(flickable === null)
            flickable = Object.firstParentByType(root, "QQuickFlickable")
    }

    policy: needed ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    minimumSize: 0.1
    palette {
        mid: Qt.rgba(0,0,0,0.25)
        dark: Qt.rgba(0,0,0,0.75)
    }
    opacity: active ? 1 : 0.4

    Behavior on opacity {
        enabled: Runtime.applicationSettings.enableAnimations
        NumberAnimation { duration: Runtime.stdAnimationDuration }
    }
}
