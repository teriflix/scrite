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
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "./qml"
import "./qml/globals"
import "./qml/init"
import "./qml/notifications"

ApplicationWindow {
    id: root

    property bool closeButtonVisible: true

    AppWindow.closeButtonVisible: closeButtonVisible
    AppWindow.onInitialize: _initSM.start()

    width: AppWindow.minimumWindowWidth
    height: AppWindow.minimumWindowHeight
    visible: true
    visibility: ApplicationWindow.Maximized

    color: Runtime.colors.primary.windowColor

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    palette.window:          Runtime.colors.palette.window
    palette.windowText:      Runtime.colors.palette.windowText
    palette.base:            Runtime.colors.palette.base
    palette.text:            Runtime.colors.palette.text
    palette.button:          Runtime.colors.palette.button
    palette.buttonText:      Runtime.colors.palette.buttonText
    palette.highlight:       Runtime.colors.palette.highlight
    palette.highlightedText: Runtime.colors.palette.highlightedText
    palette.light:           Runtime.colors.palette.light
    palette.midlight:        Runtime.colors.palette.midlight
    palette.mid:             Runtime.colors.palette.mid
    palette.dark:            Runtime.colors.palette.dark
    palette.shadow:          Runtime.colors.palette.shadow
    palette.alternateBase:   Runtime.colors.palette.alternateBase
    palette.toolTipBase:     Runtime.colors.palette.toolTipBase
    palette.toolTipText:     Runtime.colors.palette.toolTipText
    palette.placeholderText: Runtime.colors.palette.placeholderText
    palette.brightText:      Runtime.colors.palette.brightText
    palette.link:            Runtime.colors.palette.link
    palette.linkVisited:     Runtime.colors.palette.linkVisited

    Loader {
        id: _contentLoader

        anchors.fill: parent

        active: false
        sourceComponent: ScriteMainWindowContent {
            enabled: !NotificationsView.visible && Runtime.allowAppUsage
        }
    }

    AppInitStateMachine {
        id: _initSM
        contentLoader: _contentLoader
    }
}

