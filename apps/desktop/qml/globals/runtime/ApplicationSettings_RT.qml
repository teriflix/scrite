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
import QtCore

import io.scrite.components

import "../"

Settings {
    property int accentColor: Runtime.colors.defaultAccentColor
    property int primaryColor: Runtime.colors.defaultPrimaryColor
    property int joinDiscordPromptCounter: 0

    property bool useCustomPdfPageColor: true
    property color darkModePdfPageColor: "lightgray"
    property color lightModePdfPageColor: "white"

    property string colorMode: "System"  // "Light", "Dark", "System"

    property real colorIntensity: 0.5

    property bool enableAnimations: true
    property bool notifyMissingRecentFiles: true
    property bool reloadPrompt: true
    property bool useNativeTextRendering: false
    property bool useSoftwareRenderer: false

    property string theme: "Material"

    Component.onCompleted: {
        colorIntensity = Runtime.bounded(0, colorIntensity, 1)
        Qt.callLater( () => {
                         Runtime.currentTheme = theme
                         Runtime.currentUseSoftwareRenderer = useSoftwareRenderer
                     })
        updateColorScheme()
    }

    category: "Application"
    location: Platform.settingsLocation

    onColorModeChanged: Qt.callLater(updateColorScheme)
    function updateColorScheme() {
        let cs = Qt.ColorScheme.Unknown // Follow OS defaults
        if(colorMode === "Light")
            cs = Qt.ColorScheme.Light
        else if(colorMode === "Dark")
            cs = Qt.ColorScheme.Dark
        Scrite.app.styleHints.colorScheme = cs
    }
}
