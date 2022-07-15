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

import io.scrite.components 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15

Image {
    property bool showTooltip: true
    property int sceneType: Scene.Standard
    property bool lightBackground: true

    width: 32; height: 32
    fillMode: Image.PreserveAspectFit
    mipmap: true
    source: {
        switch(sceneType) {
        case Scene.Song: return lightBackground ? "../icons/content/queue_mus24px.png" : "../icons/content/queue_mus24px_inverted.png"
        case Scene.Action: return lightBackground ? "../icons/content/fight_scene.png" : "../icons/content/fight_scene_inverted.png"
        case Scene.Montage: return lightBackground ? "../icons/content/camera_alt.png" : "../icons/content/camera_alt_inverted.png"
        default: break
        }
        return ""
    }

    ToolTip.delay: 1000
    ToolTip.text: {
        switch(sceneType) {
        case Scene.Song: return "This is a Song scene."
        case Scene.Action: return "This is a Action scene."
        case Scene.Montage: return "This is a Montage scene."
        default: break
        }
        return ""
    }
    ToolTip.visible: sceneTypeMouseArea.containsMouse

    MouseArea {
        id: sceneTypeMouseArea
        enabled: parent.showTooltip && parent.sceneType !== Scene.Standard
        anchors.fill: parent
        propagateComposedEvents: true
    }
}
