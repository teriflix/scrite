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

import "qrc:/qml/globals"

Item {
    id: root

    required property TextEdit textEdit

    property alias cursorFlashTime: _private.cursorFlashTime

    property color color: Runtime.colors.primary.c100.text

    function highlight() {
        if(textEdit.activeFocus)
            _private.startFocusAnimation()
    }

    x: _private.cursorRect.x
    y: _private.cursorRect.y
    width: _private.cursorRect.width
    height: _private.cursorRect.height

    Rectangle {
        id: _blinkingCursor

        anchors.fill: parent

        color: root.color
    }

    Rectangle {
        id: _focusCursor

        readonly property real tMin: 1
        readonly property real tMax: 2
        readonly property real minOpacity: 0.5
        readonly property real maxOpacity: 1

        property real t: 1 // Assumed to change from 1 to 10
        property bool textEditActiveFocus: root.textEdit.activeFocus

        anchors.centerIn: parent

        width: _private.cursorRect.width * 5 * t
        height: _private.cursorRect.height * t

        color: root.color
        opacity: maxOpacity - (t-tMin)*(maxOpacity-minOpacity)/(tMax-tMin)

        visible: false

        onTextEditActiveFocusChanged: {
            if(textEditActiveFocus) {
                _private.startFocusAnimation()
            } else {
                _private.stopFocusAnimation()
            }
        }
    }

    QtObject {
        id: _private

        property int cursorFlashTime: Qt.styleHints.cursorFlashTime

        property rect cursorRect: root.textEdit.cursorRectangle

        property SequentialAnimation focusAnimation

        readonly property SequentialAnimation blinkAnimation: SequentialAnimation {
            loops: Animation.Infinite
            running: true

            ScriptAction {
                script: _blinkingCursor.visible = true
            }

            PauseAnimation { duration: _private.cursorFlashTime/2  }

            ScriptAction {
                script: _blinkingCursor.visible = false
            }

            PauseAnimation { duration: _private.cursorFlashTime/2  }
        }

        readonly property Component focusAnimationComponent: SequentialAnimation {
            loops: 1

            ScriptAction {
                script: {
                    _focusCursor.t = _focusCursor.tMin
                    _focusCursor.visible = true
                }
            }

            NumberAnimation {
                to: _focusCursor.tMax
                from: _focusCursor.tMin
                target: _focusCursor
                duration: _private.cursorFlashTime/4
                property: "t"
            }

            NumberAnimation {
                to: _focusCursor.tMin
                from: _focusCursor.tMax
                target: _focusCursor
                duration: _private.cursorFlashTime/4
                property: "t"
            }

            ScriptAction {
                script: {
                    _focusCursor.t = _focusCursor.tMin
                    _focusCursor.visible = false
                }
            }
        }

        function startFocusAnimation() {
            stopFocusAnimation()

            focusAnimation = focusAnimationComponent.createObject(_focusCursor)
            focusAnimation.stopped.connect(focusAnimation.destroy)
            focusAnimation.start()
        }

        function stopFocusAnimation() {
            if(focusAnimation) {
                focusAnimation.stop()
                focusAnimation.destroy()
            }

            _focusCursor.t = _focusCursor.tMin
            _focusCursor.visible = false
        }

        onCursorRectChanged: {
            blinkAnimation.restart()
        }
    }
}
