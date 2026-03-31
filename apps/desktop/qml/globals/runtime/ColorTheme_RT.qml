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

QtObject {
    id: root

    required property int key
    required property int theme

    // These semantic aliases always reference the light-mode shade names; mirroredShade()
    // inside each cN already handles the flip, so c200/c300/c400 resolve correctly in
    // both light and dark mode without any additional conditional here.
    property ColorPair_RT button:    c200
    property ColorPair_RT highlight: c400

    property color borderColor:    c400.background
    property color separatorColor: c400.background
    property color windowColor:    c300.background

    // Returns the mirror shade for dark mode so that light-mode shade references
    // automatically produce appropriately dark (or light) colours when the theme flips.
    // c500, c10, and "regular" are intentionally left unchanged as mid-point / special values.
    function mirroredShade(shade) {
        if (theme !== Material.Dark)
            return shade
        switch (shade) {
        case Material.Shade50:   return Material.Shade900
        case Material.Shade100:  return Material.Shade900
        case Material.Shade200:  return Material.Shade800
        case Material.Shade300:  return Material.Shade700
        case Material.Shade400:  return Material.Shade600
        case Material.Shade600:  return Material.Shade400
        case Material.Shade700:  return Material.Shade300
        case Material.Shade800:  return Material.Shade200
        case Material.Shade900:  return Material.Shade100
        case Material.ShadeA100: return Material.ShadeA700
        case Material.ShadeA200: return Material.ShadeA400
        case Material.ShadeA400: return Material.ShadeA200
        case Material.ShadeA700: return Material.ShadeA100
        default:                 return shade  // Shade500 — mid-point, unchanged
        }
    }

    property ColorPair_RT c10: ColorPair_RT {
        background: Qt.rgba(1,1,1,0)
    }

    property ColorPair_RT c50: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade50))
    }

    property ColorPair_RT c100: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade100))
    }

    property ColorPair_RT c200: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade200))
    }

    property ColorPair_RT c300: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade300))
    }

    property ColorPair_RT c400: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade400))
    }

    property ColorPair_RT c500: ColorPair_RT {
        background: Material.color(root.key, Material.Shade500)
    }

    property ColorPair_RT c600: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade600))
    }

    property ColorPair_RT c700: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade700))
    }

    property ColorPair_RT c800: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade800))
    }

    property ColorPair_RT c900: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.Shade900))
    }

    property ColorPair_RT a100: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.ShadeA100))
    }

    property ColorPair_RT a200: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.ShadeA200))
    }

    property ColorPair_RT a400: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.ShadeA400))
    }

    property ColorPair_RT a700: ColorPair_RT {
        background: Material.color(root.key, root.mirroredShade(Material.ShadeA700))
    }

    property ColorPair_RT editor: ColorPair_RT {
        background: root.theme === Material.Light ? "white" : "black"
        text: root.theme === Material.Light ? "black" : "white"
    }
}
