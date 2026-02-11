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
        if( !(_parent && Object.isOfType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        item = Qt.createQmlObject("import QtQuick 2.15; Item { }", _parent)
        item.objectName = "FloatingDockLayer"
        item.anchors.fill = _parent
        item.visible = _parent

        // Init dock panels within this module
        FloatingMarkupToolsDock.init()
    }
}
