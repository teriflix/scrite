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

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Image {
    id: root

    property int sceneType: Scene.Standard
    property bool showTooltip: true
    property bool lightBackground: true

    signal clicked()

    ToolTipPopup {
        text: {
            switch(sceneType) {
            case Scene.Song: return "This is a Song scene."
            case Scene.Action: return "This is a Action scene."
            case Scene.Montage: return "This is a Montage scene."
            default: break
            }
            return ""
        }
        visible: _mouseArea.containsMouse && showTooltip
    }

    width: 32
    height: 32

    mipmap: true
    fillMode: Image.PreserveAspectFit

    source: {
        switch(sceneType) {
        case Scene.Song: return lightBackground ? "qrc:/icons/content/music.png" : "qrc:/icons/content/music_inverted.png"
        case Scene.Action: return lightBackground ? "qrc:/icons/content/fight.png" : "qrc:/icons/content/fight_inverted.png"
        case Scene.Montage: return lightBackground ? "qrc:/icons/content/montage.png" : "qrc:/icons/content/montage_inverted.png"
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
