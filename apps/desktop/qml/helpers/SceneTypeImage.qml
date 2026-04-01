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

import io.scrite.components

Image {
    id: root

    property int sceneType: Scene.Standard
    property bool showTooltip: true
    property bool lightBackground: true

    signal clicked()

    ToolTipPopup {
        text: {
            switch(root.sceneType) {
            case Scene.Song: return "This is a Song scene."
            case Scene.Action: return "This is a Action scene."
            case Scene.Montage: return "This is a Montage scene."
            default: break
            }
            return ""
        }
        visible: _mouseArea.containsMouse && root.showTooltip
    }

    width: 32
    height: 32

    mipmap: true
    fillMode: Image.PreserveAspectFit

    source: {
        switch(sceneType) {
        case Scene.Song: return lightBackground ? Runtime.themedIcon("qrc:/icons/content/music.png") : "image://icon/dark/qrc:/icons/content/music.png"
        case Scene.Action: return lightBackground ? Runtime.themedIcon("qrc:/icons/content/fight.png") : "image://icon/dark/qrc:/icons/content/fight.png"
        case Scene.Montage: return lightBackground ? Runtime.themedIcon("qrc:/icons/content/montage.png") : "image://icon/dark/qrc:/icons/content/montage.png"
        default: break
        }
        return ""
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        enabled: parent.sceneType !== Scene.Standard
        hoverEnabled: root.showTooltip
        propagateComposedEvents: true

        onClicked: root.clicked()
    }
}
