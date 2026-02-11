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

Item {
    id: root

    required property DropArea mainDropArea
    required property ListView screenplayElementList
    required property color dropAreaHighlightColor

    signal dropSceneAtRequest(QtObject source, int index)

    width: 100
    height: root.screenplayElementList.height

    Rectangle {
        width: 5
        height: parent.height

        color: _private.highlightAsDropArea ? root.dropAreaHighlightColor : Qt.rgba(0,0,0,0)
    }

    Rectangle {
        anchors.fill: parent
        anchors.bottomMargin: root.screenplayElementList.scrollBarRequired ? 20 : 0

        color: Runtime.colors.primary.button.background
        border.color: Runtime.colors.primary.borderColor
        border.width: 1

        opacity: _private.highlightAsDropArea ? 0.75 : 0.5
        visible: Scrite.document.structure.elementCount > 0 && enableDragDrop

        VclLabel {
            anchors.fill: parent
            anchors.margins: 5

            wrapMode: Text.WordWrap
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            text: root.screenplayElementList.count === 0 ? "Drop the first scene here." : "Drop the last scene here."
        }
    }

    DropArea {
        id: _footerDropArea

        anchors.fill: parent

        keys: [Runtime.timelineViewSettings.dropAreaKey]
        enabled: enableDragDrop

        onEntered: (drag) => {
                       root.screenplayElementList.forceActiveFocus()
                       drag.acceptProposedAction()
                   }

        onDropped: (drop) => {
                       _private.highlightAsDropArea = false
                       dropSceneAtRequest(drop.source, Scrite.document.screenplay.elementCount)
                       drop.acceptProposedAction()
                   }
    }

    QtObject {
        id: _private

        property bool highlightAsDropArea: _footerDropArea.containsDrag || root.mainDropArea.containsDrag
    }
}

