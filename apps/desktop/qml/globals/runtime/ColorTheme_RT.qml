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

    property var button: c200
    property var highlight: c400

    property color borderColor: c400.background
    property color separatorColor: c400.background
    property color windowColor: c300.background

    property ColorPair_RT regular: ColorPair_RT {
        background: Material.color(root.key)
    }

    property ColorPair_RT c10: ColorPair_RT {
        background: Qt.rgba(1,1,1,0)
    }

    property ColorPair_RT c50: ColorPair_RT {
        background: Material.color(root.key, Material.Shade50)
    }

    property ColorPair_RT c100: ColorPair_RT {
        background: Material.color(root.key, Material.Shade100)
    }

    property ColorPair_RT c200: ColorPair_RT {
        background: Material.color(root.key, Material.Shade200)
    }

    property ColorPair_RT c300: ColorPair_RT {
        background: Material.color(root.key, Material.Shade300)
    }

    property ColorPair_RT c400: ColorPair_RT {
        background: Material.color(root.key, Material.Shade400)
    }

    property ColorPair_RT c500: ColorPair_RT {
        background: Material.color(root.key, Material.Shade500)
    }

    property ColorPair_RT c600: ColorPair_RT {
        background: Material.color(root.key, Material.Shade600)
    }

    property ColorPair_RT c700: ColorPair_RT {
        background: Material.color(root.key, Material.Shade700)
    }

    property ColorPair_RT c800: ColorPair_RT {
        background: Material.color(root.key, Material.Shade800)
    }

    property ColorPair_RT c900: ColorPair_RT {
        background: Material.color(root.key, Material.Shade900)
    }

    property ColorPair_RT a100: ColorPair_RT {
        background: Material.color(root.key, Material.ShadeA100)
    }

    property ColorPair_RT a200: ColorPair_RT {
        background: Material.color(root.key, Material.ShadeA200)
    }

    property ColorPair_RT a400: ColorPair_RT {
        background: Material.color(root.key, Material.ShadeA400)
    }

    property ColorPair_RT a700: ColorPair_RT {
        background: Material.color(root.key, Material.ShadeA700)
    }
}
