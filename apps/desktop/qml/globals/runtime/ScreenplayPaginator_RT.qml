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

import QtQuick

import io.scrite.components

ScreenplayPaginator {
    property bool paused: false
    required property ScreenplayElement currentElement

    enabled: !paused && !Scrite.document.loading
    format: Scrite.document.printFormat
    cursorPosition: currentElement ? (currentElement.scene ? Math.max(currentElement.scene.cursorPosition,0) : 0) : -1

    function toggle() { paused = !paused }
    function pause() { paused = true }
    function resume() { paused = false }
}