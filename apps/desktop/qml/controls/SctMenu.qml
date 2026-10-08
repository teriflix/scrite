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

import QtQml
import QtQuick
import QtQuick.Controls

import Scrite.App

import "../globals"

Menu {
    id: root

    property bool autoWidth: true

    property int minInsertIndex: 0
    property int insertIndex: minInsertIndex

    function includeMenu(object) {
        let menu = object as Menu
        if(menu) {
            insertMenu(insertIndex, menu)
            ++insertIndex
            return
        }
    }

    function includeMenuItem(object) {
        let menuItem = object as MenuItem
        if(menuItem) {
            insertItem(insertIndex, menuItem)
            ++insertIndex
            return
        }
    }

    function includeAction(object) {
        let action = object as Action
        if(action) {
            insertAction(insertIndex, action)
            ++insertIndex
            return
        }
    }

    function excludeMenu(object) {
        let menu = object as Menu
        if(menu) {
            for(let i=0; i<count; i++) {
                if(menuAt(i) === menu) {
                    takeMenu(i)
                    insertIndex = Math.max(insertIndex-1, minInsertIndex)
                    return
                }
            }
        }
    }

    function excludeMenuItem(object) {
        let menuItem = object as MenuItem
        if(menuItem) {
            for(let i=0; i<count; i++) {
                if(itemAt(i) === menuItem) {
                    takeItem(i)
                    insertIndex = Math.max(insertIndex-1, minInsertIndex)
                    return
                }
            }
        }
    }

    function excludeAction(object) {
        let action = object as Action
        if(action) {
            for(let i=0; i<count; i++) {
                if(actionAt(i) === action) {
                    takeAction(i)
                    insertIndex = Math.max(insertIndex-1, minInsertIndex)
                    return
                }
            }
        }
    }

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
