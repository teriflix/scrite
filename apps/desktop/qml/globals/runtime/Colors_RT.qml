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
import QtQuick.Controls.Material

import io.scrite.components

import "../"

Item {
    id: root

    required property ApplicationSettings_RT applicationSettings

    readonly property int   defaultAccentColor: Material.DeepPurple
    readonly property int   defaultPrimaryColor: Material.Grey
    readonly property var   forDocument: ["#e60000", "#ff9900", "#ffff00", "#008a00", "#0066cc", "#9933ff", "#ffffff", "#facccc", "#ffebcc", "#ffffcc", "#cce8cc", "#cce0f5", "#ebd6ff", "#bbbbbb", "#f06666", "#ffc266", "#ffff66", "#66b966", "#66a3e0", "#c285ff", "#888888", "#a10000", "#b26b00", "#b2b200", "#006100", "#0047b2", "#6b24b2", "#444444", "#5c0000", "#663d00", "#666600", "#003700", "#002966", "#3d1466"]
    readonly property var   forScene: SceneColors.palette
    readonly property alias theme: _private.theme
    readonly property alias scheme: _private.scheme

    property real sceneControlTint: applicationSettings.colorIntensity*0.4
    property real sceneHeadingTint: applicationSettings.colorIntensity*0.4
    property real currentNoteTint: applicationSettings.colorIntensity*0.4
    property real currentLineHightlightTint: applicationSettings.colorIntensity*0.2
    property real screenplayTracksTint: Runtime.bounded(0.4, applicationSettings.colorIntensity, 1)
    property color buttonDownIconColor: _private.theme === Material.Dark ? accent.c800.background : accent.c200.background
    property color selectedSceneControlTint: Color.translucent(theme === Material.Light ? primary.c100.background : primary.c800.background, Runtime.bounded(0.2,1-applicationSettings.colorIntensity,0.8))
    property color selectedSceneHeadingTint:  Color.translucent(theme === Material.Light ? primary.c100.background : primary.c800.background, Runtime.bounded(0.2,1-applicationSettings.colorIntensity,0.8))

    readonly property color transparent: "transparent"

    readonly property ColorTheme_RT primary: ColorTheme_RT {
        ObjectRegister.name: "primaryColors"

        key: Material.Grey // applicationSettings.primaryColor
        theme: root.theme
    }

    readonly property ColorTheme_RT accent: ColorTheme_RT {
        ObjectRegister.name: "accentColors"

        key: root.applicationSettings.accentColor
        theme: root.theme
    }

    function tint(a, b) {
        return Color.stacked( Color.tint(a, b), palette.window )
    }

    function tintTx(a, b) {
        return Color.transform( Color.tint(a, b), palette.window, scheme )
    }

    function tx(c, backdrop) {
        return Color.transform(c, backdrop !== undefined ? backdrop : palette.window, scheme)
    }

    ObjectRegister.name: "runtimeColors"

    QtObject {
        id: _private

        property int   theme: {
            if (root.applicationSettings.colorMode === "Light")
                return Material.Light
            if (root.applicationSettings.colorMode === "Dark")
                return Material.Dark
            // "System" follow OS preference
            return Scrite.app.styleHints.colorScheme === Qt.ColorScheme.Dark ? Material.Dark : Material.Light
        }
        property int  scheme: theme === Material.Dark ? Qt.ColorScheme.Dark : Qt.ColorScheme.Light

        onThemeChanged: Scrite.app.styleHints.colorScheme = scheme
    }
}
