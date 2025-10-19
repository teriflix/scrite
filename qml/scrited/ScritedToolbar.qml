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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import io.scrite.components 1.0

import "qrc:/qml/controls"
import "qrc:/qml/helpers"

ActionManagerToolBar {
    id: root

    actionManager: _private.actions

    QtObject {
        id: _private

        readonly property ActionManager actions : ActionManager {
            title: "Scrited"
            objectName: "scritedOptions"

            Action {
                readonly property string tooltip: "Load a video file for this screenplay"

                enabled: ActionManager.canHandle
                objectName: "loadMovie"
                text: "Load Movie"

                icon.source: "qrc:/icons/mediaplayer/movie.png"
            }

            Action {
                readonly property string tooltip: "Toggle media playback"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Space)

                checkable: true
                enabled: ActionManager.canHandle
                objectName: "loadMovie"
                shortcut: defaultShortcut
                text: checked ? "Pause" : "Play"

                icon.source: checked ? "qrc:/icons/mediaplayer/pause.png" : "qrc:/icons/mediaplayer/play_arrow.png"
            }

            Action {
                readonly property string tooltip: "Rewind 10 seconds"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Control, Qt.Key_Left)

                enabled: ActionManager.canHandle
                objectName: "rewind10"
                shortcut: defaultShortcut
                text: "Rewind 10s"

                icon.source: "qrc:/icons/mediaplayer/rewind_10.png"
            }

            Action {
                readonly property string tooltip: "Rewind one second"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Left)

                enabled: ActionManager.canHandle
                objectName: "rewind1"
                shortcut: defaultShortcut
                text: "Rewind 1s"

                icon.source: "qrc:/icons/mediaplayer/fast_rewind.png"
            }

            Action {
                readonly property string tooltip: "Forward one second"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Right)

                enabled: ActionManager.canHandle
                objectName: "forward1"
                shortcut: defaultShortcut
                text: "Forward 1s"

                icon.source: "qrc:/icons/mediaplayer/fast_forward.png"
            }

            Action {
                readonly property string tooltip: "Forward ten seconds"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Control, Qt.Key_Right)

                enabled: ActionManager.canHandle
                objectName: "forward10"
                shortcut: defaultShortcut
                text: "Forward 10s"

                icon.source: "qrc:/icons/mediaplayer/forward_10.png"
            }

            Action {
                readonly property string tooltip: "Previous Scene"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Control, Qt.Key_Up)

                enabled: ActionManager.canHandle
                objectName: "previousScene"
                shortcut: defaultShortcut
                text: "Previous Scene"

                icon.source: "qrc:/icons/action/keyboard_arrow_up.png"
            }

            Action {
                readonly property string tooltip: "Next Scene"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Control, Qt.Key_Down)

                enabled: ActionManager.canHandle
                objectName: "nextScene"
                shortcut: defaultShortcut
                text: "Next Scene"

                icon.source: "qrc:/icons/action/keyboard_arrow_down.png"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Up)

                enabled: ActionManager.canHandle
                objectName: "scrollUp"
                shortcut: defaultShortcut
                text: "Scroll Up"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Down)

                enabled: ActionManager.canHandle
                objectName: "scrollDown"
                shortcut: defaultShortcut
                text: "Scroll Down"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Alt, Qt.Key_Up)

                enabled: ActionManager.canHandle
                objectName: "previousPage"
                shortcut: defaultShortcut
                text: "Previous Page"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Alt, Qt.Key_Down)

                enabled: ActionManager.canHandle
                objectName: "nextPage"
                shortcut: defaultShortcut
                text: "Next Page"
            }

            Action {
                readonly property string tooltip: "Use video time as current scene time offset"
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Greater)

                enabled: ActionManager.canHandle
                objectName: "syncTime"
                shortcut: defaultShortcut
                text: "Sync Time"

                icon.source: "qrc:/icons/mediaplayer/sync_with_screenplay.png"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Control, Qt.Key_Greater)

                enabled: ActionManager.canHandle
                objectName: "adjustOffsets"
                shortcut: defaultShortcut
                text: "Adjust Offsets"
            }

            Action {
                readonly property string tooltip: "Reset time offset of all scenes."

                enabled: ActionManager.canHandle
                objectName: "resetOffsets"
                text: "Reset Offsets"

                icon.source: "qrc:/icons/mediaplayer/reset_screenplay_offsets.png"
            }

            Action {
                readonly property string tooltip: "Toggle time column."

                checkable: true
                checked: false
                enabled: ActionManager.canHandle
                objectName: "toggleTimeColumn"
                text: "Time Column"

                icon.source: "qrc:/icons/mediaplayer/time_column.png"
            }

            Action {
                readonly property string tooltip: "Check this to keep media playback and screenplay in sync."

                checkable: true
                checked: false
                enabled: ActionManager.canHandle
                objectName: "autoScroll"
                text: "Auto Scroll"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_BraceLeft)

                enabled: ActionManager.canHandle
                objectName: "decreaseVideoHeight"
                shortcut: defaultShortcut
                text: "Decrease Video Height"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_BraceRight)

                enabled: ActionManager.canHandle
                objectName: "increaseVideoHeight"
                shortcut: defaultShortcut
                text: "Increase Video Height"
            }

            Action {
                readonly property bool visible: false
                readonly property string defaultShortcut: ActionManager.shortcut(Qt.Key_Asterisk)

                enabled: ActionManager.canHandle
                objectName: "resetVideoHeight"
                shortcut: defaultShortcut
                text: "Reset Video Height"
            }
        }
    }
}
