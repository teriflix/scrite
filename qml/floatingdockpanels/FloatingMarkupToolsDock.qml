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
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

FloatingDock {
    id: root

    property SceneDocumentBinder sceneDocumentBinder

    x: adjustedX(Runtime.markupToolsSettings.contentX)
    y: adjustedY(Runtime.markupToolsSettings.contentY)
    width: 462
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
        enabled: _private.textFormat && Runtime.allowAppUsage
        ShortcutsModelItem.title: "Bold"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: _private.textFormat.toggleBold()
    }

    Shortcut {
        sequence: "Ctrl+I"
        context: Qt.ApplicationShortcut
        enabled: _private.textFormat && Runtime.allowAppUsage
        ShortcutsModelItem.title: "Italics"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: _private.textFormat.toggleItalics()
    }

    Shortcut {
        sequence: "Ctrl+U"
        context: Qt.ApplicationShortcut
        enabled: _private.textFormat && Runtime.allowAppUsage
        ShortcutsModelItem.title: "Underline"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: _private.textFormat.toggleUnderline()
    }

    Shortcut {
        sequence: "Shift+F3"
        context: Qt.ApplicationShortcut
        enabled: sceneDocumentBinder && Runtime.allowAppUsage
        ShortcutsModelItem.title: "All CAPS"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.group: "Markup Tools"
        ShortcutsModelItem.enabled: enabled
        onActivated: sceneDocumentBinder.changeCase(SceneDocumentBinder.UpperCase)
    }

    Shortcut {
        sequence: "Ctrl+Shift+F3"
        context: Qt.ApplicationShortcut
        enabled: sceneDocumentBinder && Runtime.allowAppUsage
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

            SimpleToolButton {
                ToolTip.text: "Strikeout\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+K")
                ToolTip.visible: containsMouse

                checked: _private.textFormat ? _private.textFormat.strikeout : false
                enabled: _private.textFormat
                iconSource: "qrc:/icons/editor/format_strikethrough.png"
                hoverEnabled: true

                onClicked: if(_private.textFormat) _private.textFormat.toggleStrikeout()
            }

            ColorToolButton {
                id: textColorToolButton

                ToolTip.text: "Text Color"
                ToolTip.visible: containsMouse

                enabled: _private.textFormat
                hoverEnabled: true
                selectedColor: _private.textFormat ? _private.textFormat.textColor : Runtime.colors.transparent

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

                        color: textColorToolButton.selectedColor === Runtime.colors.transparent ? "black" : textColorToolButton.selectedColor

                        font.bold: true
                        font.pixelSize: parent.height * 0.70
                        font.underline: true

                        text: "A"
                    }
                }
            }

            ColorToolButton {
                id: bgColorToolButton

                ToolTip.text: "Background Color"
                ToolTip.visible: containsMouse

                enabled: _private.textFormat
                hoverEnabled: true
                selectedColor: _private.textFormat ? _private.textFormat.backgroundColor : Runtime.colors.transparent

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
                    color: bgColorToolButton.selectedColor === Runtime.colors.transparent ? "white" : bgColorToolButton.selectedColor

                    VclText {
                        anchors.centerIn: parent

                        color: textColorToolButton.selectedColor === Runtime.colors.transparent ? "black" : textColorToolButton.selectedColor

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

    QtObject {
        id: _private

        property TextFormat textFormat: sceneDocumentBinder ? sceneDocumentBinder.textFormat : null
        property SceneElement sceneElement: sceneDocumentBinder ? sceneDocumentBinder.currentElement : null

        property bool initialized: false
    }
}
