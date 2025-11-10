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

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Image {
    id: root

    property int sceneType: Scene.Standard
    property bool showTooltip: true
    property bool lightBackground: true

    signal clicked()

    ToolTip.text: {
        switch(sceneType) {
        case Scene.Song: return "This is a Song scene."
        case Scene.Action: return "This is a Action scene."
        case Scene.Montage: return "This is a Montage scene."
        default: break
        }
        return ""
    }
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.visible: _mouseArea.containsMouse && showTooltip

    width: 32
    height: 32

    mipmap: true
    fillMode: Image.PreserveAspectFit

    source: {
        switch(sceneType) {
        case Scene.Song: return lightBackground ? "qrc:/icons/content/queue_mus24px.png" : "qrc:/icons/content/queue_mus24px_inverted.png"
        case Scene.Action: return lightBackground ? "qrc:/icons/content/fight_scene.png" : "qrc:/icons/content/fight_scene_inverted.png"
        case Scene.Montage: return lightBackground ? "qrc:/icons/content/camera_alt.png" : "qrc:/icons/content/camera_alt_inverted.png"
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
