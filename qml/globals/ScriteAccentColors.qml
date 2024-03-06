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

pragma Singleton

import QtQuick 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

QtObject {
    readonly property int key: Material.BlueGrey
    readonly property color windowColor: c300.background
    readonly property color borderColor: c400.background
    readonly property color separatorColor: c400.background
    readonly property var highlight: c400
    readonly property var button: c200

    readonly property QtObject regular: QtObject {
        readonly property color background: Material.color(key)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c10: QtObject {
        readonly property color background: Qt.rgba(1,1,1,0)
        readonly property color text: "black"
    }

    readonly property QtObject c50: QtObject {
        readonly property color background: Material.color(key, Material.Shade50)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c100: QtObject {
        readonly property color background: Material.color(key, Material.Shade100)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c200: QtObject {
        readonly property color background: Material.color(key, Material.Shade200)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c300: QtObject {
        readonly property color background: Material.color(key, Material.Shade300)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c400: QtObject {
        readonly property color background: Material.color(key, Material.Shade400)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c500: QtObject {
        readonly property color background: Material.color(key, Material.Shade500)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c600: QtObject {
        readonly property color background: Material.color(key, Material.Shade600)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c700: QtObject {
        readonly property color background: Material.color(key, Material.Shade700)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c800: QtObject {
        readonly property color background: Material.color(key, Material.Shade800)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject c900: QtObject {
        readonly property color background: Material.color(key, Material.Shade900)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject a100: QtObject {
        readonly property color background: Material.color(key, Material.ShadeA100)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject a200: QtObject {
        readonly property color background: Material.color(key, Material.ShadeA200)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject a400: QtObject {
        readonly property color background: Material.color(key, Material.ShadeA400)
        readonly property color text: Scrite.app.textColorFor(background)
    }

    readonly property QtObject a700: QtObject {
        readonly property color background: Material.color(key, Material.ShadeA700)
        readonly property color text: Scrite.app.textColorFor(background)
    }
}
