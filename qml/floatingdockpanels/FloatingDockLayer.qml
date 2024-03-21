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

/**
  This QML item provides a layer on which all floating docks will be shown
  */

pragma Singleton

import QtQuick 2.15

import io.scrite.components 1.0

QtObject {
    // Public API
    property Item item: null
    property bool valid: item !== null

    function init(_parent) {
        if( !(_parent && Scrite.app.verifyType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        item = Qt.createQmlObject("import QtQuick 2.15; Item { }", _parent)
        item.anchors.fill = _parent
        item.visible = _parent
    }
}
