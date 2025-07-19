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

import io.scrite.components 1.0

import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

SequentialAnimation {
    id: root

    signal denyCanvasPreviewRequest()
    signal allowCanvasPreviewRequest()

    running: false

    function run(items, type) {
        if(running)
            return

        __layoutItems = items
        __layoutType = type
        start()
    }

    ScriptAction {
        script: {
            root.denyCanvasPreviewRequest()
            root.clear()
        }
    }

    PauseAnimation {
        duration: 50
    }

    ScriptAction {
        script: {
            let oldItems = __layoutItems
            __layoutItems = []
            oldItems.forEach( function(item) {
                item.element.selected = true
            })
            __layoutItemBounds = Scrite.document.structure.layoutElements(__layoutType)
            __layoutType = -1
            oldItems.forEach( function(item) {
                item.element.selected = false
            })
            Scrite.document.structure.forceBeatBoardLayout = false
        }
    }

    PauseAnimation {
        duration: 50
    }

    ScriptAction {
        script: {
            let rect = {
                "top": __layoutItemBounds.top,
                "left": __layoutItemBounds.left,
                "right": __layoutItemBounds.left + __layoutItemBounds.width-1,
                "bottom": __layoutItemBounds.top + __layoutItemBounds.height-1
            };
            __layoutItemBounds = undefined
            root.init(_elementItems, rect)
            root.allowCanvasPreviewRequest()
        }
    }

    // These are private variables
    property int __layoutType: -1
    property var __layoutItems: []
    property var __layoutItemBounds
}

