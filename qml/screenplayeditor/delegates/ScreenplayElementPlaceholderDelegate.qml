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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Rectangle {
    id: root

    required property real zoomLevel
    required property rect documentMargins
    required property FontMetrics headingFontMetrics
    required property ScreenplayElement screenplayElement

    property bool evaluateSizeHint: true

    readonly property alias scene: _private.scene
    readonly property alias screenplayElementType: _private.screenplayElementType

    implicitHeight: _private.heightEstimate

    color: scene ? Qt.tint(scene.color, "#E7FFFFFF") : Runtime.colors.primary.c300.background
    border.width: 1
    border.color: scene ? scene.color : Runtime.colors.primary.c400.background

    SceneSizeHintItem {
        id: _sizeHint

        width: contentWidth * zoomLevel
        height: contentHeight * zoomLevel + ((screenplayElement.pageBreakBefore ? 1 : 0) + (screenplayElement.pageBreakAfter ? 1 : 0))*40

        scene: _private.scene
        format: Scrite.document.printFormat
        active: evaluateSizeHint && !screenplayElement.omitted
        visible: false
        asynchronous: false
    }

    VclLabel {
        anchors.right: _sceneHeadingText.left
        anchors.rightMargin: 20
        anchors.verticalCenter: _sceneHeadingText.verticalCenter

        width: headingFontMetrics.averageCharacterWidth*5

        text: screenplayElement.resolvedSceneNumber
        font: _sceneHeadingText.font
        color: screenplayElement.hasUserSceneNumber ? "black" : "gray"
    }

    VclLabel {
        id: _sceneHeadingText

        // It's root.top and not parent.top on purpose.
        // The implicit parent property doesn't get initialized while this component is being
        // constructed by using Component.createObject() method, which is the mechanism used
        // for create instances of the QML component from this file.
        anchors.top: root.top
        anchors.left: root.left
        anchors.right: root.right
        anchors.topMargin: 20
        anchors.leftMargin: documentMargins.left
        anchors.rightMargin: documentMargins.right

        font: _private.headingFormat.font2

        color: screenplayElementType === ScreenplayElement.BreakElementType ? "gray" : "black"
        elide: Text.ElideMiddle
        text: {
            if(screenplayElementType === ScreenplayElement.BreakElementType)
                return screenplayElement.breakTitle
            if(screenplayElement.omitted)
                return "[OMITTED]"
            if(scene && scene.heading.enabled)
                return scene.heading.text
            return "NO SCENE HEADING"
        }
    }

    Image {
        anchors.top: _sceneHeadingText.bottom
        anchors.left: root.left
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.topMargin: 20 * zoomLevel
        anchors.bottomMargin: 20 * zoomLevel

        source: "qrc:/images/sample_scene.png"
        opacity: 0.5
        visible: !screenplayElement.omitted
        fillMode: Image.TileVertically
    }

    QtObject {
        id: _private

        property int screenplayElementType: root.screenplayElement ? root.screenplayElement.elementType : -1

        property real heightEstimate: screenplayElement.omitted ? (_sceneHeadingText.height + _sceneHeadingText.anchors.topMargin*2) : _sizeHint.height

        property Scene scene: root.screenplayElement ? root.screenplayElement.scene : null

        property SceneElementFormat headingFormat: Scrite.document.displayFormat.elementFormat(SceneElement.Heading)
    }

    on__FocusIn: () => { }     // TODO
    on__FocusOut: () => { }    // TODO
}
