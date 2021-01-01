/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13

import Scrite 1.0

Item {
    property Item scritedView
    height: 45

    Loader {
        height: parent.height
        active: scritedView !== null
        anchors.centerIn: parent
        sourceComponent: Row {
            spacing: 1

            ToolButton3 {
                iconSource: "../icons/mediaplayer/movie.png"
                ToolTip.text: "Load a video file for this screenplay."
                enabled: scritedView.screenplaySplitsCount > 0
                onClicked: scritedView.loadMedia()
            }

            ToolButton3 {
                iconSource: scritedView.mediaIsPlaying ? "../icons/mediaplayer/pause.png" : "../icons/mediaplayer/play_arrow.png"
                ToolTip.text: "Toggle media playback.\t(Space)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.togglePlayback()

                ShortcutsModelItem.priority: 0
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: scritedView.mediaIsPlaying ? "Pause" : "Play"
                ShortcutsModelItem.shortcut: "Space"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/rewind_10.png"
                ToolTip.text: "Rewind 10 seconds.\t(" + app.polishShortcutTextForDisplay("Ctrl") + " + ←)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.rewind()

                ShortcutsModelItem.priority: -1
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Rewind 10s"
                ShortcutsModelItem.shortcut: "Ctrl+←"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/fast_rewind.png"
                ToolTip.text: "Rewind half second.\t(←)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.miniRewind()

                ShortcutsModelItem.priority: -2
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Rewind 0.5s"
                ShortcutsModelItem.shortcut: "←"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/fast_forward.png"
                ToolTip.text: "Forward half second.\t(" + app.polishShortcutTextForDisplay("Ctrl") + "+→)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.miniForward()

                ShortcutsModelItem.priority: -3
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Forward 0.5s"
                ShortcutsModelItem.shortcut: "→"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/forward_10.png"
                ToolTip.text: "Forward 10 seconds.\t(→)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.forward()

                ShortcutsModelItem.priority: -4
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Forward 10s"
                ShortcutsModelItem.shortcut: "Ctrl+→"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/action/keyboard_arrow_up.png"
                ToolTip.text: "Previous Scene\t(" + app.polishShortcutTextForDisplay("Ctrl") + "+↑)"
                enabled: scritedView.previousSceneAvailable
                onClicked: scritedView.scrollPreviousScene()

                ShortcutsModelItem.priority: -5
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Previous Scene"
                ShortcutsModelItem.shortcut: "Ctrl+↑"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/action/keyboard_arrow_down.png"
                ToolTip.text: "Next Scene\t(" + app.polishShortcutTextForDisplay("Ctrl") + "+↓)"
                enabled: scritedView.nextSceneAvailable
                onClicked: scritedView.scrollNextScene()

                ShortcutsModelItem.priority: -6
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Next Scene"
                ShortcutsModelItem.shortcut: "Ctrl+↓"
                ShortcutsModelItem.enabled: enabled
            }

            QtObject {
                ShortcutsModelItem.priority: -7
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Scroll Up"
                ShortcutsModelItem.shortcut: "↑"
                ShortcutsModelItem.enabled: scritedView.canScrollUp
            }

            QtObject {
                ShortcutsModelItem.priority: -8
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Scroll Down"
                ShortcutsModelItem.shortcut: "↓"
                ShortcutsModelItem.enabled: scritedView.canScrollDown
            }

            QtObject {
                ShortcutsModelItem.priority: -9
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Previous Page"
                ShortcutsModelItem.shortcut: "Alt+↑"
                ShortcutsModelItem.enabled: scritedView.canScrollUp
            }

            QtObject {
                ShortcutsModelItem.priority: -10
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Next Page"
                ShortcutsModelItem.shortcut: "Alt+↓"
                ShortcutsModelItem.enabled: scritedView.canScrollDown
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/sync_with_screenplay.png"
                ToolTip.text: "Use video time as current scene time offset.\t(> or .)"
                enabled: scritedView.screenplaySplitsCount > 0 && scritedView.mediaIsLoaded
                onClicked: scritedView.syncVideoTimeWithScreenplayOffsets()

                ShortcutsModelItem.priority: -11
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Sync Time"
                ShortcutsModelItem.shortcut: "> ."
                ShortcutsModelItem.enabled: enabled
            }

            QtObject {
                ShortcutsModelItem.priority: -12
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Adjust Offsets"
                ShortcutsModelItem.shortcut: "Ctrl+>"
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/reset_screenplay_offsets.png"
                ToolTip.text: "Reset time offset of all scenes."
                enabled: scritedView.screenplaySplitsCount > 0
                onClicked: scritedView.resetScreenplayOffsets()
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/time_column.png"
                ToolTip.text: "Toggle time column."
                enabled: scritedView.screenplaySplitsCount > 0
                down: scritedView.timeOffsetVisible
                onClicked: scritedView.toggleTimeOffsetDisplay()
            }

            CheckBox2 {
                anchors.verticalCenter: parent.verticalCenter
                checked: scritedView.playbackScreenplaySync
                onToggled: scritedView.playbackScreenplaySync = checked
                hoverEnabled: true
                ToolTip.text: "Check this to keep media playback and screenplay in sync."
                ToolTip.visible: hovered
                text: "Auto Scroll"
                enabled: scritedView.mediaIsLoaded
                focusPolicy: Qt.NoFocus

                ShortcutsModelItem.priority: -13
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Auto Scroll " + (checked ? "OFF" : "ON")
                ShortcutsModelItem.shortcut: "+"
                ShortcutsModelItem.enabled: enabled
            }
        }
    }
}
