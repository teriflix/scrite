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

Item {
    property Item scritedView

    height: 45

    visible: scritedView !== null

    Loader {
        height: parent.height
        active: scritedView
        anchors.centerIn: parent
        sourceComponent: Row {
            spacing: 1

            FlatToolButton {
                iconSource: "qrc:/icons/mediaplayer/movie.png"
                ToolTip.text: "Load a video file for this screenplay."
                enabled: scritedView.screenplaySplitsCount > 0
                onClicked: scritedView.loadMedia()
            }

            FlatToolButton {
                ToolTip.text: "Toggle media playback.\t(Space)"

                ShortcutsModelItem.priority: 0
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: scritedView.mediaIsPlaying ? "Pause" : "Play"
                ShortcutsModelItem.shortcut: "Space"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.mediaIsLoaded
                iconSource: scritedView.mediaIsPlaying ? "qrc:/icons/mediaplayer/pause.png" : "qrc:/icons/mediaplayer/play_arrow.png"

                onClicked: click()

                function click() {
                    scritedView.togglePlayback()
                }
            }

            FlatToolButton {
                ToolTip.text: "Rewind 10 seconds.\t(" + Scrite.app.polishShortcutTextForDisplay("Ctrl") + " + ←)"

                ShortcutsModelItem.priority: -1
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Rewind 10s"
                ShortcutsModelItem.shortcut: "Ctrl+←"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.mediaIsLoaded
                iconSource: "qrc:/icons/mediaplayer/rewind_10.png"

                onClicked: click()

                function click() {
                    scritedView.rewind()
                }
            }

            FlatToolButton {
                ToolTip.text: "Rewind half second.\t(←)"

                ShortcutsModelItem.priority: -2
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Rewind 1s"
                ShortcutsModelItem.shortcut: "←"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.mediaIsLoaded
                iconSource: "qrc:/icons/mediaplayer/fast_rewind.png"

                onClicked: click()

                function click() {
                    scritedView.miniRewind()
                }
            }

            FlatToolButton {
                ToolTip.text: "Forward half second.\t(" + Scrite.app.polishShortcutTextForDisplay("Ctrl") + "+→)"

                ShortcutsModelItem.priority: -3
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Forward 1s"
                ShortcutsModelItem.shortcut: "→"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.mediaIsLoaded
                iconSource: "qrc:/icons/mediaplayer/fast_forward.png"

                onClicked: click()

                function click() {
                    scritedView.miniForward()
                }
            }

            FlatToolButton {
                ToolTip.text: "Forward 10 seconds.\t(→)"

                ShortcutsModelItem.priority: -4
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Forward 10s"
                ShortcutsModelItem.shortcut: "Ctrl+→"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.mediaIsLoaded
                iconSource: "qrc:/icons/mediaplayer/forward_10.png"

                onClicked: click()

                function click() {
                    scritedView.forward()
                }
            }

            FlatToolButton {
                ToolTip.text: "Previous Scene\t(" + Scrite.app.polishShortcutTextForDisplay("Ctrl") + "+↑)"

                ShortcutsModelItem.priority: -5
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Previous Scene"
                ShortcutsModelItem.shortcut: "Ctrl+↑"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.previousSceneAvailable
                iconSource: "qrc:/icons/action/keyboard_arrow_up.png"

                onClicked: click()

                function click() {
                    scritedView.scrollPreviousScene()
                }
            }

            FlatToolButton {
                ToolTip.text: "Next Scene\t(" + Scrite.app.polishShortcutTextForDisplay("Ctrl") + "+↓)"

                ShortcutsModelItem.priority: -6
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Next Scene"
                ShortcutsModelItem.shortcut: "Ctrl+↓"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.nextSceneAvailable
                iconSource: "qrc:/icons/action/keyboard_arrow_down.png"

                onClicked: click()

                function click() {
                    scritedView.scrollNextScene()
                }
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

            FlatToolButton {
                iconSource: "qrc:/icons/mediaplayer/sync_with_screenplay.png"
                ToolTip.text: "Use video time as current scene time offset.\t(> or .)"

                ShortcutsModelItem.priority: -11
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Sync Time"
                ShortcutsModelItem.shortcut: "> ."
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: click()

                enabled: scritedView.screenplaySplitsCount > 0 && scritedView.mediaIsLoaded
                onClicked: click()

                function click() {
                    scritedView.syncVideoTimeWithScreenplayOffsets(true)
                }
            }

            QtObject {
                ShortcutsModelItem.priority: -12
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Adjust Offsets"
                ShortcutsModelItem.shortcut: "Ctrl+>"
                ShortcutsModelItem.enabled: enabled
            }

            FlatToolButton {
                iconSource: "qrc:/icons/mediaplayer/reset_screenplay_offsets.png"
                ToolTip.text: "Reset time offset of all scenes."
                enabled: scritedView.screenplaySplitsCount > 0
                onClicked: scritedView.resetScreenplayOffsets()
            }

            FlatToolButton {
                iconSource: "qrc:/icons/mediaplayer/time_column.png"
                ToolTip.text: "Toggle time column."
                enabled: scritedView.screenplaySplitsCount > 0
                down: scritedView.timeOffsetVisible
                onClicked: scritedView.toggleTimeOffsetDisplay()
            }

            VclCheckBox {
                ToolTip.text: "Check this to keep media playback and screenplay in sync."
                ToolTip.visible: hovered

                ShortcutsModelItem.priority: -13
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Auto Scroll " + (checked ? "OFF" : "ON")
                ShortcutsModelItem.shortcut: "+"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.canActivate: true
                ShortcutsModelItem.onActivated: scritedView.playbackScreenplaySync = checked

                anchors.verticalCenter: parent.verticalCenter

                text: "Auto Scroll"
                checked: scritedView.playbackScreenplaySync
                enabled: scritedView.mediaIsLoaded
                focusPolicy: Qt.NoFocus
                hoverEnabled: true

                onToggled: scritedView.playbackScreenplaySync = checked
            }

            QtObject {
                ShortcutsModelItem.priority: -14
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Decrease Video Height"
                ShortcutsModelItem.shortcut: "("
                ShortcutsModelItem.enabled: true
            }

            QtObject {
                ShortcutsModelItem.priority: -14
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Increase Video Height"
                ShortcutsModelItem.shortcut: ")"
                ShortcutsModelItem.enabled: true
            }

            QtObject {
                ShortcutsModelItem.priority: -15
                ShortcutsModelItem.group: "Scrited"
                ShortcutsModelItem.title: "Default Video Height"
                ShortcutsModelItem.shortcut: "*"
                ShortcutsModelItem.enabled: true
            }
        }
    }
}
