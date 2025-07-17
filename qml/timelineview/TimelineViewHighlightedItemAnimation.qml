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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Shapes 1.5
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/screenplay"

Item {
    id: root

    required property ListView screenplayElementList

    Image {
        id: _highlightBackdrop
        opacity: 0.75*Math.max(scale-1.0,0)
        transformOrigin: Item.Bottom
    }

    ResetOnChange {
        trackChangesOn: screenplayElementList.currentIndex
        from: false
        to: true
        onValueChanged: {
            if(value) {
                const ci = screenplayElementList.currentItem
                if(ci) {
                    ci.grabToImage( function(result) {
                        _highlightBackdrop.source = result.url
                        _highlightAnimation.running = true
                    }, Qt.size(ci.width*2,ci.height*2))
                }
            } else
                _highlightAnimation.running = false
        }
    }

    SequentialAnimation {
        id: _highlightAnimation
        running: false
        loops: 1

        ScriptAction {
            script: {
                root.clip = false

                const ci = screenplayElementList.currentItem
                const cipos = root.mapFromItem(ci,0,0)

                _highlightBackdrop.scale = 1
                _highlightBackdrop.x = cipos.x
                _highlightBackdrop.y = cipos.y
                _highlightBackdrop.width = ci.width
                _highlightBackdrop.height = ci.height
            }
        }

        NumberAnimation {
            target: _highlightBackdrop
            property: "scale"
            from: 1; to: 2
            duration: 250
            easing.type: Easing.InBack
        }

        PauseAnimation {
            duration: 50
        }

        NumberAnimation {
            target: _highlightBackdrop
            property: "scale"
            from: 2; to: 1
            duration: 250
            easing.type: Easing.InBack
        }

        ScriptAction {
            script: root.clip = true
        }
    }
}
