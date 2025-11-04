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
import "qrc:/qml/overlays"
import "qrc:/qml/controls"

QtObject {
    id: root

    property bool visible: false

    property string message: "Busy Doing Something..."

    Component.onDestruction: {
        if(visible)
            BusyOverlay.deref(root)
    }

    onVisibleChanged: {
        if(visible)
            BusyOverlay.ref(message, root)
        else
            BusyOverlay.deref(root)
    }

    onMessageChanged: {
        if(visible)
            BusyOverlay.ref(message, root)
    }
}
