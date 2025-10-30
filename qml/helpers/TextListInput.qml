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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Flow {
    id: root

    required property var textList
    required property var completionStrings
    required property string labelText
    required property string labelIconSource
    required property string addTextButtonTooltip

    property var highlightedTextColors: Runtime.colors.accent.c900
    property var textColors: Runtime.colors.accent.c10
    property bool readOnly: false
    property font font: Runtime.idealFontMetrics.font
    property real textBorderWidth: 1
    property real zoomLevel: 1.0

    property alias header: _headerLoader.sourceComponent

    readonly property alias label: _label

    signal ensureVisible(Item item, rect area)
    signal textClicked(string text, Item source)
    signal textCloseRequest(string text, Item source)
    signal configureTextRequest(string text, TagText tag)
    signal newTextRequest(string text)

    function configureTextsLater() {
        Qt.callLater(configureTexts)
    }

    function configureTexts() {
        const nrTexts = _texts.count
        for(let i=0; i<nrTexts; i++) {
            let text = _texts.itemAt(i)
            text.configure()
        }
    }

    function acceptNewText() {
        _newInputLoader.active = true
    }

    flow: Flow.LeftToRight
    spacing: 5

    Loader {
        id: _headerLoader
        active: sourceComponent !== null
    }

    FlatToolButton {
        ToolTip.text: root.labelText

        suggestedWidth: _label.height
        suggestedHeight: _label.height

        iconSource: root.labelIconSource

        onClicked: _newInputLoader.active = true
    }

    VclLabel {
        id: _label

        text: root.labelText + ": "

        topPadding: 5
        bottomPadding: 5

        font.bold: true
        font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)
    }

    Repeater {
        id: _texts

        model: root.textList

        TagText {
            id: _text

            required property string modelData

            property var colors: containsMouse ? root.highlightedTextColors : root.textColors

            Component.onCompleted: configure()

            border.color: colors.text
            border.width: root.textBorderWidth

            text: modelData
            color: colors.background
            enabled: !root.readOnly
            textColor: colors.text
            topPadding: Math.max(5, 5 * root.zoomLevel)
            leftPadding: Math.max(10, 10 * root.zoomLevel)
            rightPadding: leftPadding
            bottomPadding: topPadding

            font.family: root.font.family
            font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)
            font.capitalization: root.font.capitalization

            onClicked: root.textClicked(modelData, _text)

            onCloseRequest: {
                if(!root.readOnly)
                    root.textCloseRequest(modelData, _text)
            }

            function configure() {
                root.configureTextRequest(modelData, _text)
            }
        }
    }

    Loader {
        id: _newInputLoader

        active: false
        visible: active

        sourceComponent: VclTextField {
            Component.onCompleted: {
                forceActiveFocus()
                root.ensureVisible(_newInputLoader, Qt.rect(0,0,width,height))
            }

            Keys.onEscapePressed: {
                text = ""
                _newInputLoader.active = false
            }

            readOnly: false
            completionStrings: root.completionStrings

            font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)
            font.capitalization: root.font.capitalization

            onEditingComplete: {
                if(text.length > 0) {
                    root.newTextRequest(text)
                }
                _newInputLoader.active = false
            }
        }

        onStatusChanged: {
            if(status === Loader.Null) {
                Object.resetProperty(_newInputLoader, "width")
                Object.resetProperty(_newInputLoader, "height")
            }
        }
    }

    Image {
        source: "qrc:/icons/content/add_box.png"

        width: _label.height
        height: width

        opacity: enabled ? 1 : 0.5
        visible: enabled
        enabled: !root.readOnly

        MouseArea {
            ToolTip.text: root.addTextButtonTooltip
            ToolTip.delay: 1000
            ToolTip.visible: containsMouse

            anchors.fill: parent

            hoverEnabled: true

            onClicked: _newInputLoader.active = true
            onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
        }
    }
}
