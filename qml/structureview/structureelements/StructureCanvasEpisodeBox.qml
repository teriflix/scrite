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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

Rectangle {
    id: root

    /**
      episodeBox is a JSON object of the form
      {
        "name": ".....",
        "sceneCount": ....,
        "geometry": { x: .., y: .. , width: .., height: .. }
      }
      */
    required property var episodeBox
    required property int episodeBoxIndex
    required property int episodeBoxCount

    required property rect canvasScrollViewportRect

    required property BoundingBoxEvaluator canvasItemsBoundingBox

    BoundingBoxItem.evaluator: canvasItemsBoundingBox
    BoundingBoxItem.stackOrder: 1.0 + (episodeBoxIndex/episodeBoxCount)
    BoundingBoxItem.livePreview: false
    BoundingBoxItem.viewportRect: canvasScrollViewportRect
    BoundingBoxItem.viewportItem: root
    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0.05)
    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0.5)

    x: episodeBox.geometry.x - 40
    y: episodeBox.geometry.y - 120 - _private.topMarginForStacks
    width: episodeBox.geometry.width + 80
    height: episodeBox.geometry.height + 120 + _private.topMarginForStacks + 40

    color: Scrite.app.translucent(Runtime.colors.accent.c100.background, Scrite.document.structure.forceBeatBoardLayout ? 0.3 : 0.1)
    border.width: 2
    border.color: Runtime.colors.accent.c600.background

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: _episodeNameText.bottom
        anchors.bottomMargin: -8

        color: Runtime.colors.accent.c200.background
    }

    VclLabel {
        id: _episodeNameText

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 8

        text: "<b>" + episodeBox.name + "</b><font size=\"-2\">: " + episodeBox.sceneCount + (episodeBox.sceneCount === 1 ? " Scene": " Scenes") + "</font>"
        color: Runtime.colors.accent.c200.text

        font.bold: true
        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 8
    }

    QtObject {
        id: _private

        property real topMarginForStacks: Scrite.document.structure.elementStacks.objectCount > 0 ? 15 : 0
    }
}
