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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Shapes 1.5
import QtQuick.Controls 2.15

import io.scrite.components 1.0



import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Rectangle {
    id: root

    readonly property real preferredHeight: _screenplayTools.height
    property alias canZoomIn: _zoomIn.enabled
    property alias canZoomOut: _zoomOut.enabled

    signal zoomInRequest()
    signal zoomOutRequest()
    signal clearRequest()

    z: 1
    implicitWidth: _screenplayToolsLayout.width+4

    color: Runtime.colors.accent.c100.background

    Flow {
        id: _screenplayToolsLayout

        anchors.horizontalCenter: parent.horizontalCenter

        height: parent.height-5

        spacing: 1
        flow: Flow.TopToBottom
        layoutDirection: Qt.RightToLeft

        FlatToolButton {
            enabled: !Scrite.document.readOnly
            iconSource: "qrc:/icons/content/clear_all.png"
            toolTipText: "Clear the screenplay, while retaining the scenes."

            onClicked: root.clearRequest()
        }

        FlatToolButton {
            id: _zoomIn
            autoRepeat: true
            iconSource: "qrc:/icons/navigation/zoom_in.png"
            toolTipText: "Increase size of blocks in this view."

            onClicked: root.zoomInRequest()

            ActionHandler {
                action: ActionHub.timelineOperations.find("zoomIn")

                onTriggered: root.zoomInRequest()
            }
        }

        FlatToolButton {
            id: _zoomOut
            autoRepeat: true
            iconSource: "qrc:/icons/navigation/zoom_out.png"
            toolTipText: "Decrease size of blocks in this view."

            onClicked: root.zoomOutRequest()

            ActionHandler {
                action: ActionHub.timelineOperations.find("zoomOut")

                onTriggered: root.zoomOutRequest()
            }
        }
    }

    Rectangle {
        anchors.right: parent.right

        width: 1
        height: parent.height

        color: Runtime.colors.accent.borderColor
    }
}
