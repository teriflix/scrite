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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"

VclFloatingDock {
    id: root

    property SceneDocumentBinder sceneDocumentBinder

    x: adjustedX(Runtime.markupToolsSettings.contentX)
    y: adjustedY(Runtime.markupToolsSettings.contentY)
    width: 426
    height: 36 + 8 + titleBarHeight
    visible: Runtime.screenplayEditorSettings.markupToolsDockVisible && Runtime.screenplayEditor

    title: "Markup Tools"

    function init() { }
    Component.onCompleted: {
        Qt.callLater( () => {
                         saveSettingsTask.enabled = true
                     })
    }

    // Shortcuts Section
    Shortcut {
        sequence: "Ctrl+B"
        context: Qt.ApplicationShortcut
        enabled: _private.textFormat
        ShortcutsModelItem.title: "Bold"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: _private.textFormat.toggleBold()
    }

    Shortcut {
        sequence: "Ctrl+I"
        context: Qt.ApplicationShortcut
        enabled: _private.textFormat
        ShortcutsModelItem.title: "Italics"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: _private.textFormat.toggleItalics()
    }

    Shortcut {
        sequence: "Ctrl+U"
        context: Qt.ApplicationShortcut
        enabled: _private.textFormat
        ShortcutsModelItem.title: "Underline"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: _private.textFormat.toggleUnderline()
    }

    Shortcut {
        sequence: "Shift+F3"
        context: Qt.ApplicationShortcut
        enabled: sceneDocumentBinder
        ShortcutsModelItem.title: "All CAPS"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: sceneDocumentBinder.changeCase(SceneDocumentBinder.UpperCase)
    }

    Shortcut {
        sequence: "Ctrl+Shift+F3"
        context: Qt.ApplicationShortcut
        enabled: sceneDocumentBinder
        ShortcutsModelItem.title: "All small"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: sceneDocumentBinder.changeCase(SceneDocumentBinder.LowerCase)
    }

    content: Item {
        enabled: !Scrite.document.readOnly

        RowLayout {
            anchors.centerIn: parent

            spacing: 2

            SimpleToolButton {
                ToolTip.text: "Bold\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+B")
                ToolTip.visible: containsMouse

                enabled: _private.textFormat
                checked: _private.textFormat ? _private.textFormat.bold : false
                iconSource: "qrc:/icons/editor/format_bold.png"
                hoverEnabled: true

                onClicked: if(_private.textFormat) _private.textFormat.toggleBold()
            }

            SimpleToolButton {
                ToolTip.text: "Italics\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+I")
                ToolTip.visible: containsMouse

                checked: _private.textFormat ? _private.textFormat.italics : false
                enabled: _private.textFormat
                iconSource: "qrc:/icons/editor/format_italics.png"
                hoverEnabled: true

                onClicked: if(_private.textFormat) _private.textFormat.toggleItalics()
            }

            SimpleToolButton {
                ToolTip.text: "Underline\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+U")
                ToolTip.visible: containsMouse

                checked: _private.textFormat ? _private.textFormat.underline : false
                enabled: _private.textFormat
                iconSource: "qrc:/icons/editor/format_underline.png"
                hoverEnabled: true

                onClicked: if(_private.textFormat) _private.textFormat.toggleUnderline()
            }

            ColorButton {
                id: textColorButton

                ToolTip.text: "Text Color"
                ToolTip.visible: containsMouse

                enabled: _private.textFormat
                hoverEnabled: true
                selectedColor: _private.textFormat ? _private.textFormat.textColor : _private.transparentColor

                onColorPicked: (newColor) => {
                                   if(_private.textFormat)
                                        _private.textFormat.textColor = newColor
                               }

                Rectangle {
                    anchors.centerIn: parent

                    width: Math.min(parent.width,parent.height) * 0.8
                    height: width

                    color: "white"

                    VclText {
                        anchors.centerIn: parent

                        color: textColorButton.selectedColor === _private.transparentColor ? "black" : textColorButton.selectedColor

                        font.bold: true
                        font.pixelSize: parent.height * 0.70
                        font.underline: true

                        text: "A"
                    }
                }
            }

            ColorButton {
                id: bgColorButton

                ToolTip.text: "Background Color"
                ToolTip.visible: containsMouse

                enabled: _private.textFormat
                hoverEnabled: true
                selectedColor: _private.textFormat ? _private.textFormat.backgroundColor : _private.transparentColor

                onColorPicked: (newColor) => {
                                   if(_private.textFormat)
                                        _private.textFormat.backgroundColor = newColor
                               }

                Rectangle {
                    anchors.centerIn: parent

                    width: Math.min(parent.width,parent.height) * 0.8
                    height: width

                    border.width: 1
                    border.color: "black"
                    color: bgColorButton.selectedColor === _private.transparentColor ? "white" : bgColorButton.selectedColor

                    VclText {
                        anchors.centerIn: parent

                        color: textColorButton.selectedColor === _private.transparentColor ? "black" : textColorButton.selectedColor

                        font.bold: true
                        font.pixelSize: parent.height * 0.70

                        text: "A"
                    }
                }
            }

            SimpleToolButton {
                ToolTip.text: "Clear formatting"
                ToolTip.visible: containsMouse

                checked: false
                enabled: _private.textFormat
                iconSource: "qrc:/icons/editor/format_clear.png"
                hoverEnabled: true

                onClicked: if(_private.textFormat) _private.textFormat.reset()
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true

                color: Runtime.colors.primary.borderColor
            }

            SimpleToolButton {
                checked: enabled ? _private.sceneElement.alignment === Qt.AlignLeft : false
                enabled: _private.sceneElement && _private.sceneElement.type === SceneElement.Action
                iconSource: "qrc:/icons/editor/format_align_left.png"

                onClicked: _private.sceneElement.alignment = _private.sceneElement.alignment === Qt.AlignLeft ? 0 : Qt.AlignLeft
            }

            SimpleToolButton {
                checked: enabled ? _private.sceneElement.alignment === Qt.AlignHCenter : false
                enabled: _private.sceneElement && _private.sceneElement.type === SceneElement.Action
                iconSource: "qrc:/icons/editor/format_align_center.png"

                onClicked: _private.sceneElement.alignment = _private.sceneElement.alignment === Qt.AlignHCenter ? 0 : Qt.AlignHCenter
            }

            SimpleToolButton {
                checked: enabled ? _private.sceneElement.alignment === Qt.AlignRight : false
                enabled: _private.sceneElement && _private.sceneElement.type === SceneElement.Action
                iconSource: "qrc:/icons/editor/format_align_right.png"

                onClicked: _private.sceneElement.alignment = _private.sceneElement.alignment === Qt.AlignRight ? 0 : Qt.AlignRight
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true

                color: Runtime.colors.primary.borderColor
            }

            SimpleToolButton {
                ToolTip.text: "All CAPS\t" + Scrite.app.polishShortcutTextForDisplay("Shift+F3")
                ToolTip.visible: containsMouse

                enabled: sceneDocumentBinder
                hoverEnabled: true

                onClicked: sceneDocumentBinder.changeCase(SceneDocumentBinder.UpperCase)

                VclText {
                    anchors.centerIn: parent
                    font.pixelSize: parent.height*0.5
                    text: "AB"
                }
            }

            SimpleToolButton {
                ToolTip.visible: containsMouse
                ToolTip.text: "All small\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+Shift+F3")

                enabled: sceneDocumentBinder
                hoverEnabled: true

                onClicked: sceneDocumentBinder.changeCase(SceneDocumentBinder.LowerCase)

                VclText {
                    anchors.centerIn: parent
                    font.pixelSize: parent.height*0.5
                    text: "ab"
                }
            }
        }
    }

    // Private Section

    // This block ensures that everytime the floating dock coordinates change,
    // they are stored in persistent settings
    Connections {
        id: saveSettingsTask

        target: root
        enabled: false

        function onXChanged() {
            Qt.callLater(saveSettingsTask.saveCoordinates)
        }

        function onYChanged() {
            Qt.callLater(saveSettingsTask.saveCoordinates)
        }

        function onCloseRequest() {
            Runtime.screenplayEditorSettings.markupToolsDockVisible = false
        }

        // Private
        function saveCoordinates() {
            Runtime.markupToolsSettings.contentX = Math.round(root.x)
            Runtime.markupToolsSettings.contentY = Math.round(root.y)
        }
    }

    // Reusable components, only accessed from within this dock
    component SimpleToolButton : Rectangle {

        property bool down: false
        property bool checked: false
        property alias pressed: tbMouseArea.pressed
        property alias hoverEnabled: tbMouseArea.hoverEnabled
        property alias containsMouse: tbMouseArea.containsMouse
        property alias iconSource: tbIcon.source

        signal clicked()

        width: implicitWidth
        height: implicitHeight
        implicitWidth: 36
        implicitHeight: 36

        radius: 4
        opacity: enabled ? 1 : 0.5
        color: tbMouseArea.pressed || down ? Runtime.colors.primary.button.background : (checked ? Runtime.colors.primary.highlight.background : Qt.rgba(0,0,0,0))

        Image {
            id: tbIcon
            anchors.fill: parent
            anchors.margins: 4
            mipmap: true
        }

        MouseArea {
            id: tbMouseArea
            anchors.fill: parent
            onClicked: parent.clicked()
        }
    }

    component ColorButton : Item {
        id: colorButton
        property color selectedColor: _private.transparentColor
        property alias hoverEnabled: cbMouseArea.hoverEnabled
        property alias containsMouse: cbMouseArea.containsMouse

        signal colorPicked(color newColor)

        width: implicitWidth
        height: implicitHeight
        implicitWidth: 36
        implicitHeight: 36

        opacity: enabled ? 1 : 0.5

        MouseArea {
            id: cbMouseArea
            anchors.fill: parent
            onClicked: colorsMenuLoader.active = true
        }

        Loader {
            id: colorsMenuLoader
            x: 0; y: parent.height
            active: false
            sourceComponent: Popup {
                id: colorsMenu
                x: 0; y: 0
                width: availableColorsPalette.suggestedWidth
                height: availableColorsPalette.suggestedHeight
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                Component.onCompleted: open()
                onClosed: Qt.callLater(() => { colorsMenuLoader.active = false})

                contentItem: AvailableColorsPalette {
                    id: availableColorsPalette
                    selectedColor: colorButton.selectedColor
                    onColorPicked: (newColor) => {
                                       colorButton.colorPicked(newColor)
                                       colorsMenu.close()
                                   }
                }
            }
        }
    }

    component AvailableColorsPalette : Grid {
        id: colorsGrid
        property int cellSize: width/columns
        readonly property int suggestedWidth: 280
        readonly property int suggestedHeight: 200
        columns: 7
        opacity: enabled ? 1 : 0.25

        property color selectedColor: _private.transparentColor
        signal colorPicked(color newColor)

        Item {
            width: colorsGrid.cellSize
            height: colorsGrid.cellSize

            Image {
                source: "qrc:/icons/navigation/close.png"
                anchors.fill: parent
                anchors.margins: 5
                mipmap: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: colorPicked(_private.transparentColor)
            }
        }

        Repeater {
            model: _private.availableColors

            Item {
                required property color modelData
                required property int index
                width: colorsGrid.cellSize
                height: colorsGrid.cellSize

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 3
                    border.width: colorsGrid.selectedColor === modelData ? 3 : 0.5
                    border.color: "black"
                    color: modelData
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: colorPicked(modelData)
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property color transparentColor: "transparent"
        readonly property var availableColors: ["#e60000", "#ff9900", "#ffff00", "#008a00", "#0066cc", "#9933ff", "#ffffff", "#facccc", "#ffebcc", "#ffffcc", "#cce8cc", "#cce0f5", "#ebd6ff", "#bbbbbb", "#f06666", "#ffc266", "#ffff66", "#66b966", "#66a3e0", "#c285ff", "#888888", "#a10000", "#b26b00", "#b2b200", "#006100", "#0047b2", "#6b24b2", "#444444", "#5c0000", "#663d00", "#666600", "#003700", "#002966", "#3d1466"]

        property TextFormat textFormat: sceneDocumentBinder ? sceneDocumentBinder.textFormat : null
        property SceneElement sceneElement: sceneDocumentBinder ? sceneDocumentBinder.currentElement : null

        property bool initialized: false
    }
}
