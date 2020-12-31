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
            spacing: 5

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
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/rewind_10.png"
                ToolTip.text: "Rewind 10 seconds.\t(" + app.polishShortcutTextForDisplay("Ctrl") + " + Left Arrow)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.rewind()
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/fast_rewind.png"
                ToolTip.text: "Rewind 10 seconds.\t(Left Arrow)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.miniRewind()
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/fast_forward.png"
                ToolTip.text: "Rewind 10 seconds.\t(" + app.polishShortcutTextForDisplay("Ctrl") + " + Right Arrow)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.miniForward()
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/forward_10.png"
                ToolTip.text: "Forward 10 seconds.\t(Right Arrow)"
                enabled: scritedView.mediaIsLoaded
                onClicked: scritedView.forward()
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
            }

            ToolButton3 {
                iconSource: "../icons/action/keyboard_arrow_up.png"
                ToolTip.text: "Previous Scene\t(" + app.polishShortcutTextForDisplay("Ctrl") + " + Up Arrow)"
                enabled: scritedView.previousSceneAvailable
                onClicked: scritedView.scrollPreviousScene()
            }

            ToolButton3 {
                iconSource: "../icons/action/keyboard_arrow_down.png"
                ToolTip.text: "Previous Scene\t(" + app.polishShortcutTextForDisplay("Ctrl") + " + Down Arrow)"
                enabled: scritedView.nextSceneAvailable
                onClicked: scritedView.scrollNextScene()
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/sync_with_screenplay.png"
                ToolTip.text: "Use video time as current scene time offset.\t(> or .)"
                enabled: scritedView.screenplaySplitsCount > 0
                onClicked: scritedView.syncVideoTimeWithScreenplayOffsets()
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/reset_screenplay_offsets.png"
                ToolTip.text: "Reset time offset of all scenes."
                enabled: scritedView.screenplaySplitsCount > 0
                onClicked: scritedView.resetScreenplayOffsets()
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.borderColor
            }

            ToolButton3 {
                iconSource: "../icons/mediaplayer/time_column.png"
                ToolTip.text: "Toggle time column."
                enabled: scritedView.screenplaySplitsCount > 0
                down: scritedView.timeOffsetVisible
                onClicked: scritedView.toggleTimeOffsetDisplay()
            }
        }
    }
}
