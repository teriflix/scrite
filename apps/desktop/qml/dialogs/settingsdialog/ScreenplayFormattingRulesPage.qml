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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../helpers"
import "../../controls"

Item {
    id: root

    clip: true

    ColumnLayout {
        id: _layout

        anchors.fill: parent
        anchors.margins: 10
        anchors.leftMargin: 0

        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            spacing: 5

            VclComboBox {
                id: _paragraphTypeComboBox

                Layout.fillWidth: true

                visible: _layout.width >= 800

                model: GenericArrayModel {
                    array: [
                        { "key": "Heading", "value": SceneElement.Heading },
                        { "key": "Action", "value": SceneElement.Action },
                        { "key": "Character", "value": SceneElement.Character },
                        { "key": "Dialogue", "value": SceneElement.Dialogue },
                        { "key": "Parenthetical", "value": SceneElement.Parenthetical },
                        { "key": "Shot", "value": SceneElement.Shot },
                        { "key": "Transition", "value": SceneElement.Transition }
                    ]
                    objectMembers: ["key", "value"]

                    function longestKeyWidth() {
                        var ret = 0
                        for(var i=0; i<count; i++) {
                            const item = at(i)
                            ret = Math.max(ret, Runtime.idealFontMetrics.boundingRect(item.key).width)
                        }
                        return Math.ceil(ret)
                    }
                }
                textRole: "key"
                valueRole: "value"
            }

            VclComboBox {
                id: _languageComboBox

                Layout.preferredWidth: _private.lanugageModel.longestKeyWidth

                model: _private.lanugageModel
                enabled: LanguageEngine.handleLanguageSwitch

                textRole: "languageName"
                valueRole: "languageCode"
                onActivated: (index) => {
                                 _private.printElementFormat.defaultLanguageCode = currentValue
                                 _private.displayElementFormat.defaultLanguageCode = currentValue
                             }
            }

            VclComboBox {
                id: _fontSizesComboBox

                readonly property var systemFonts: Scrite.app.systemFontInfo()

                model: systemFonts.standardSizes

                onActivated: (index) => {
                                 const fontSize = model[index]
                                 _private.printElementFormat.fontPointSize = fontSize
                                 _private.displayElementFormat.fontPointSize = fontSize
                             }
            }

            RowLayout {
                spacing: 0

                SimpleToolButton {
                    id: _boldButton
                    down: checked
                    iconSource: "qrc:/icons/editor/format_bold.png"
                    onClicked: {
                        checked = !checked
                        _private.printElementFormat.fontBold = checked ? SceneElementFormat.Set : SceneElementFormat.Unset
                        _private.displayElementFormat.fontBold = checked ? SceneElementFormat.Set : SceneElementFormat.Unset
                    }
                }

                SimpleToolButton {
                    id: _italicsButton
                    down: checked
                    iconSource: "qrc:/icons/editor/format_italics.png"
                    onClicked: {
                        checked = !checked
                        _private.printElementFormat.fontItalics = checked ? SceneElementFormat.Set : SceneElementFormat.Unset
                        _private.displayElementFormat.fontItalics = checked ? SceneElementFormat.Set : SceneElementFormat.Unset
                    }
                }

                SimpleToolButton {
                    id: _underlineButton
                    down: checked
                    iconSource: "qrc:/icons/editor/format_underline.png"
                    onClicked: {
                        checked = !checked
                        _private.printElementFormat.fontUnderline = checked ? SceneElementFormat.Set : SceneElementFormat.Unset
                        _private.displayElementFormat.fontUnderline = checked ? SceneElementFormat.Set : SceneElementFormat.Unset
                    }
                }
            }

            SimpleToolButton {
                id: _textAlignment
                property int value: Qt.AlignLeft
                iconSource: {
                    switch(value) {
                    case Qt.AlignLeft:
                        return "qrc:/icons/editor/format_align_left.png"
                    case Qt.AlignHCenter:
                        return "qrc:/icons/editor/format_align_center.png"
                    case Qt.AlignRight:
                        return "qrc:/icons/editor/format_align_right.png"
                    case Qt.AlignJustify:
                        return "qrc:/icons/editor/format_align_justify.png"
                    }
                    return "qrc:/icons/editor/format_align_left.png"
                }
                hoverEnabled: true

                ToolTip.text: "Paragraph Indentation"
                ToolTip.visible: containsMouse

                onClicked: _textAlignmentMenu.open()

                Item {
                    width: parent.width
                    anchors.bottom: parent.bottom

                    VclMenu {
                        id: _textAlignmentMenu

                        VclMenuItem {
                            text: "Left"
                            checkable: true
                            checked: _textAlignment.value === Qt.AlignLeft
                            icon.source: "qrc:/icons/editor/format_align_left.png"
                            font.bold: _textAlignment.value === Qt.AlignLeft
                            onClicked: {
                                _textAlignment.value = Qt.AlignLeft
                                _private.printElementFormat._textAlignment = _textAlignment.value
                                _private.displayElementFormat._textAlignment = _textAlignment.value
                            }
                        }

                        VclMenuItem {
                            text: "Center"
                            checkable: true
                            checked: _textAlignment.value === Qt.AlignHCenter
                            icon.source: "qrc:/icons/editor/format_align_center.png"
                            font.bold: _textAlignment.value === Qt.AlignHCenter
                            onClicked: {
                                _textAlignment.value = Qt.AlignHCenter
                                _private.printElementFormat._textAlignment = _textAlignment.value
                                _private.displayElementFormat._textAlignment = _textAlignment.value
                            }
                        }

                        VclMenuItem {
                            text: "Right"
                            checkable: true
                            checked: _textAlignment.value === Qt.AlignRight
                            icon.source: "qrc:/icons/editor/format_align_right.png"
                            font.bold: _textAlignment.value === Qt.AlignRight
                            onClicked: {
                                _textAlignment.value = Qt.AlignRight
                                _private.printElementFormat._textAlignment = _textAlignment.value
                                _private.displayElementFormat._textAlignment = _textAlignment.value
                            }
                        }

                        VclMenuItem {
                            text: "Justify"
                            checkable: true
                            checked: _textAlignment.value === Qt.AlignJustify
                            icon.source: "qrc:/icons/editor/format_align_justify.png"
                            font.bold: _textAlignment.value === Qt.AlignJustify
                            onClicked: {
                                _textAlignment.value = Qt.AlignJustify
                                _private.printElementFormat._textAlignment = _textAlignment.value
                                _private.displayElementFormat._textAlignment = _textAlignment.value
                            }
                        }
                    }
                }
            }

            ColorToolButton {
                id: _textForeground

                ToolTip.text: "Text Color"
                ToolTip.visible: containsMouse

                hoverEnabled: true
                onColorPicked: (newColor) => {
                                   _textForeground.selectedColor = newColor
                                   _private.printElementFormat.textColor = newColor
                                   _private.displayElementFormat.textColor = newColor
                               }

                Rectangle {
                    anchors.centerIn: parent

                    width: Math.min(parent.width,parent.height) * 0.8
                    height: width
                    color: "white"
                    border.width: parent.colorsMenuVisible ? 2 : 0

                    VclText {
                        anchors.centerIn: parent

                        color: _textForeground.selectedColor === Runtime.colors.transparent ? "black" : _textForeground.selectedColor

                        font.bold: true
                        font.pixelSize: parent.height * 0.70
                        font.underline: true

                        text: "A"
                    }
                }
            }

            // FIXME: Make this visible once we figure out a way to show background colors
            ColorToolButton {
                id: _textBackground

                ToolTip.text: "Background Color"
                ToolTip.visible: containsMouse

                visible: false
                hoverEnabled: true
                onColorPicked: (newColor) => {
                                   _textBackground.selectedColor = newColor
                                   _private.printElementFormat.backgroundColor = newColor
                                   _private.displayElementFormat.backgroundColor = newColor
                               }

                Rectangle {
                    anchors.centerIn: parent

                    width: Math.min(parent.width,parent.height) * 0.8
                    height: width

                    border.width: parent.colorsMenuVisible ? 2 : 1
                    border.color: "black"
                    color: _textBackground.selectedColor === Runtime.colors.transparent ? "white" : _textBackground.selectedColor

                    VclText {
                        anchors.centerIn: parent

                        color: _textForeground.selectedColor === Runtime.colors.transparent ? "black" : _textForeground.selectedColor

                        font.bold: true
                        font.pixelSize: parent.height * 0.70

                        text: "A"
                    }
                }
            }

            SimpleToolButton {
                id: _textLineHeight
                property real value: 0.85
                down: _textLineHeightEditor.visible
                iconSource: "qrc:/icons/editor/format_line_spacing.png"
                hoverEnabled: true

                onClicked: _textLineHeightEditor.open()

                ToolTip.text: "Line Spacing"
                ToolTip.visible: containsMouse

                Item {
                    width: parent.width
                    anchors.bottom: parent.bottom

                    Popup {
                        id: _textLineHeightEditor
                        closePolicy: Popup.CloseOnEscape|Popup.CloseOnPressOutside
                        contentItem: SpinBox {
                            from: 25
                            to: 300
                            stepSize: 5
                            editable: true
                            value: _textLineHeight.value * 100
                            onValueModified: {
                                const v = value/100
                                _textLineHeight.value = v
                                _private.printElementFormat.lineHeight = v
                                _private.displayElementFormat.lineHeight = v
                            }
                        }
                    }
                }
            }

            SimpleToolButton {
                id: _firstLineIndent
                property real value: 0

                down: _firstLineIndentEditor.visible
                iconSource: "qrc:/icons/editor/format_first_line_indent.png"
                hoverEnabled: true

                onClicked: _firstLineIndentEditor.open()

                ToolTip.text: "First Line Indent"
                ToolTip.visible: containsMouse

                Item {
                    width: parent.width
                    anchors.bottom: parent.bottom

                    Popup {
                        id: _firstLineIndentEditor
                        closePolicy: Popup.CloseOnEscape|Popup.CloseOnPressOutside
                        contentItem: SpinBox {
                            from: 0
                            to: 100
                            stepSize: 5
                            editable: true
                            value: _firstLineIndent.value
                            onValueModified: {
                                const v = value
                                _firstLineIndent.value = v
                                _private.printElementFormat.textIndent = v
                                _private.displayElementFormat.textIndent = v
                            }
                        }
                    }
                }
            }

            SimpleToolButton {
                ToolTipPopup {
                    container: parent
                    visible: parent.containsMouse
                    text: "Copy one or more attributes of " + _paragraphTypeComboBox.currentText + " to other paragraph types."
                }

                hoverEnabled: true
                iconSource: "qrc:/icons/action/done_all.png"
                onClicked: _copyAttribsDialog.open()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: _previewText

                scale: (parent.width-100)/width
                width: Scrite.document.printFormat.pageLayout.contentWidth
                anchors.centerIn: parent

                font: Scrite.document.printFormat.defaultFont
                readOnly: true
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                background: Item { }

                SceneDocumentBinder {
                    id: _previewTextBinder
                    screenplayFormat: Scrite.document.printFormat
                    applyLanguageFonts: true
                    applyFormattingEvenInTransaction: true
                    scene: Scene {
                        undoRedoEnabled: false

                        elements: [
                            SceneElement {
                                type: SceneElement.Heading
                                text: "INT. CLUB - DAY"
                            },
                            SceneElement {
                                type: SceneElement.Action
                                text: "Dr. Rajkumar enters the club house like a boss. He makes eye contact with everybody in the room."
                            },
                            SceneElement {
                                type: SceneElement.Character
                                text: "Dr. Rajkumar"
                            },
                            SceneElement {
                                type: SceneElement.Parenthetical
                                text: "(singing)"
                            },
                            SceneElement {
                                type: SceneElement.Dialogue
                                text: "If you come today, its too early. If you come tomorrow, its too late."
                            },
                            SceneElement {
                                type: SceneElement.Shot
                                text: "EXTREME CLOSEUP on Dr. Rajkumar's smiling face."
                            },
                            SceneElement {
                                type: SceneElement.Transition
                                text: "CUT TO"
                            }
                        ]
                    }
                    textWidth: _previewText.width
                    textDocument: _previewText.textDocument
                    cursorPosition: -1
                    forceSyncDocument: true
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: (mouse) => {
                                   var cpos = _previewText.positionAt(mouse.x, mouse.y)
                                   var sceneElement = _previewTextBinder.sceneElementAt(cpos)
                                   if(sceneElement)
                                   _paragraphTypeComboBox.currentIndex = _paragraphTypeComboBox.model.firstIndexOf("value", sceneElement.type)
                               }

                    // This rectangle follows actively selected paragraph type in the dialog box
                    SceneElementIndicator {
                        id: _activeElementIndicator

                        opacity: 0.2
                        backgroundColor: Color.translucent(Runtime.colors.accent.c600.background, 0.5)

                        function updateSceneElement() {
                            for(var i=0; i<_previewTextBinder.scene.elementCount; i++) {
                                var _sceneElement = _previewTextBinder.scene.elementAt(i)
                                if(_sceneElement.type === _paragraphTypeComboBox.currentValue) {
                                    sceneElement = _sceneElement
                                    return
                                }
                            }
                        }

                        Component.onCompleted: {
                            updateSceneElement()
                            _paragraphTypeComboBox.currentValueChanged.connect(updateSceneElement)
                        }
                    }

                    // This rectangle follows paragraph under the mouse cursor
                    SceneElementIndicator {
                        id: _hoveredElementIndicator

                        property int cursorPosition: parent.containsMouse ? _previewText.positionAt(parent.mouseX, parent.mouseY) : -1
                        sceneElement: _previewTextBinder.sceneElementAt(cursorPosition)

                        opacity: _activeElementIndicator.sceneElement === sceneElement ? 0 : 0.5
                        backgroundColor: Color.translucent(Runtime.colors.primary.c600.background, 0.5)

                        ToolTip.text: sceneElement ? sceneElement.typeAsString : ""
                        ToolTip.visible: opacity > 0 && sceneElement
                    }

                    component SceneElementIndicator : Item {
                        id: _sceneElementIndicator
                        property SceneElement sceneElement
                        property rect sceneElementRect

                        property color backgroundColor: Color.translucent(Runtime.colors.accent.c600.background, 0.5)

                        property bool valid: sceneElementRect.width > 0 && sceneElementRect.height > 0
                        visible: valid
                        x: valid ? _previewText.leftInset + _previewText.leftPadding + sceneElementRect.x : 0
                        y: valid ? _previewText.topInset + _previewText.topPadding + sceneElementRect.y : 0
                        width: valid ? sceneElementRect.width : 0
                        height: valid ? sceneElementRect.height : 0

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -5
                            color: parent.backgroundColor
                            radius: 5
                        }

                        Connections {
                            target: _private.printElementFormat
                            function onElementFormatChanged() {
                                Qt.callLater(_sceneElementIndicator.determineSceneElementRect, true)
                            }
                        }

                        onSceneElementChanged: Qt.callLater(determineSceneElementRect)

                        function determineSceneElementRect(reload) {
                            if(reload === true)
                                _previewTextBinder.reload()
                            sceneElementRect = _previewTextBinder.sceneElementBoundingRect(sceneElement)
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            spacing: 20

            VclButton {
                text: "Factory Reset"
                toolTipText: "Restores formatting options to defaults for current document only."

                onClicked: {
                    Scrite.document.formatting.resetToFactoryDefaults()
                    Scrite.document.printFormat.resetToFactoryDefaults()
                }
            }

            VclButton {
                text: "Make Default"
                toolTipText: "Saves current formatting options as default for all current and future documents."

                onClicked: Scrite.document.formatting.saveAsUserDefaults()
            }
        }

        VclText {
            Layout.fillWidth: true
            Layout.topMargin: 20

            readonly property Action action: ActionHub.languageOptions.find("handleLanguageSwitch")

            visible: !LanguageEngine.handleLanguageSwitch
            wrapMode: Text.WordWrap
            text: "NOTE: Paragraph language option cannot be configured when <b>" + action.text + "</b> option is unchecked in Language settings."
        }
    }

    // Private implementation
    VclDialog {
        id: _copyAttribsDialog

        title: "Copy Attributes"
        width: 640
        height: Math.min(480, root.height*0.8)

        content: Item {
            implicitHeight: _copyAttribsDialogLayout.height + 40

            ColumnLayout {
                id: _copyAttribsDialogLayout
                width: parent.width-40
                anchors.centerIn: parent
                spacing: 20

                VclLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Select attributes of <b>" + _paragraphTypeComboBox.currentText + "</b> you want to copy to all other paragraph types."
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 5

                    VclCheckBox {
                        id: _copyFontFamilyAttrib
                        text: "Font Family: " + _private.displayElementFormat.font.family
                        padding: 0
                        checked: false
                    }

                    VclCheckBox {
                        id: _copyFontSizeAttrib
                        text: "Font Size: " + _private.displayElementFormat.font.pointSize + " pt"
                        padding: 0
                        checked: false
                    }

                    VclCheckBox {
                        id: _copyFontStyleAttrib
                        text: "Font Style: (" + fontStyle + ")"
                        padding: 0
                        checked: false
                        visible: fontStyle !== ""

                        property string fontStyle: {
                            var styles = []
                            if(_private.displayElementFormat.font.bold)
                                styles.push("<b>B</b>")
                            if(_private.displayElementFormat.font.italic)
                                styles.push("<i>I</i>")
                            if(_private.displayElementFormat.font.underline)
                                styles.push("<u>U</u>")
                            return styles.join(", ")
                        }
                    }

                    VclCheckBox {
                        id: _copyTextAlignmentAttrib
                        text: "Alignment: " + alignment
                        padding: 0
                        checked: false

                        property string alignment: {
                            if(_private.displayElementFormat._textAlignment === Qt.AlignLeft)
                                return "Left Align"
                            if(_private.displayElementFormat._textAlignment === Qt.AlignRight)
                                return "Right Align"
                            if(_private.displayElementFormat._textAlignment === Qt.AlignHCenter)
                                return "Center Align"
                            if(_private.displayElementFormat._textAlignment === Qt.AlignJustify)
                                return "Justify"
                            return "Left Align"
                        }
                    }

                    VclCheckBox {
                        id: _copyLineHeightAttrib
                        text: "Line Height: " + Math.round(_private.displayElementFormat.lineHeight*100) + "%"
                        padding: 0
                        checked: false
                    }

                    VclCheckBox {
                        id: _copyTextIndentAttrib
                        text: "First Line Indent: " + Math.round(_private.displayElementFormat.textIndent) + "pt"
                        padding: 0
                        checked: false
                    }

                    RowLayout {
                        spacing: 10

                        VclCheckBox {
                            id: _copyColorAttribs
                            text: "Colors"
                            padding: 0
                            checked: false
                        }

                        Rectangle {
                            implicitWidth: 30
                            implicitHeight: _copyColorAttribs.height
                            color: _private.displayElementFormat.backgroundColor
                            border.width: 1
                            border.color: Runtime.colors.primary.borderColor

                            VclText {
                                font.pixelSize: parent.height * 0.7
                                color: _private.displayElementFormat.textColor
                                anchors.centerIn: parent
                                text: "A"
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20

                    VclButton {
                        text: "Select All"
                        onClicked: {
                            _copyFontFamilyAttrib.checked = true
                            _copyFontSizeAttrib.checked = true
                            _copyFontStyleAttrib.checked = true
                            _copyLineHeightAttrib.checked = true
                            _copyTextAlignmentAttrib.checked = true
                            _copyColorAttribs.checked = true
                            _copyTextIndentAttrib.checked = true
                        }
                    }

                    VclButton {
                        text: "Unselect All"
                        onClicked: {
                            _copyFontFamilyAttrib.checked = false
                            _copyFontSizeAttrib.checked = false
                            _copyFontStyleAttrib.checked = false
                            _copyLineHeightAttrib.checked = false
                            _copyTextAlignmentAttrib.checked = false
                            _copyColorAttribs.checked = false
                            _copyTextIndentAttrib.checked = false
                        }
                    }

                    VclButton {
                        text: "Apply"
                        onClicked: {
                            var applyToAll = (props) => {
                                _private.displayElementFormat.applyToAll(props)
                                _private.printElementFormat.applyToAll(props)
                            }

                            if(_copyFontFamilyAttrib.checked)
                                applyToAll(SceneElementFormat.FontFamily)
                            if(_copyFontSizeAttrib.checked)
                                applyToAll(SceneElementFormat.FontSize)
                            if(_copyFontStyleAttrib.checked)
                                applyToAll(SceneElementFormat.FontStyle)
                            if(_copyLineHeightAttrib.checked)
                                applyToAll(SceneElementFormat.LineHeight)
                            if(_copyTextAlignmentAttrib.checked)
                                applyToAll(SceneElementFormat.TextAlignment)
                            if(_copyColorAttribs.checked)
                                applyToAll(SceneElementFormat.TextAndBackgroundColors)
                            if(_copyTextIndentAttrib.checked)
                                applyToAll(SceneElementFormat.TextIndent)

                            _copyAttribsDialog.close()
                        }
                    }
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property ListModel lanugageModel: ListModel {
            property int longestKeyWidth: -1

            Component.onCompleted: {
                const supportedLanguages = Runtime.language.supported
                let keyWidth = 0;

                append({"languageCode": -1, "languageName": "Default"})
                keyWidth = Runtime.idealFontMetrics.boundingRect("Default").width

                for(let i=0; i<supportedLanguages.count; i++) {
                    const language = supportedLanguages.languageAt(i)
                    append({"languageCode": language.code, "languageName": language.name})
                    keyWidth = Math.max(keyWidth, Runtime.idealFontMetrics.boundingRect(language.name).width)
                }

                longestKeyWidth = keyWidth + 50
            }
        }

        property SceneElementFormat printElementFormat: Scrite.document.printFormat.elementFormat(_paragraphTypeComboBox.currentValue)
        property SceneElementFormat displayElementFormat: Scrite.document.formatting.elementFormat(_paragraphTypeComboBox.currentValue)

        onPrintElementFormatChanged: {
            _languageComboBox.currentIndex = Runtime.language.supported.indexOfLanguage(printElementFormat.defaultLanguageCode) + 1

            const font = printElementFormat.font
            _fontSizesComboBox.currentIndex = _fontSizesComboBox.model.indexOf(font.pointSize)
            _boldButton.checked = font.bold
            _italicsButton.checked = font.italic
            _underlineButton.checked = font.underline

            _textForeground.selectedColor = printElementFormat.textColor
            _textBackground.selectedColor = Color.translucent(printElementFormat.backgroundColor, 4)
            _textAlignment.value = printElementFormat._textAlignment
            _textLineHeight.value = printElementFormat.lineHeight
            _firstLineIndent.value = printElementFormat.textIndent
        }
    }

    Component.onCompleted: {
        Scrite.document.formatting.beginTransaction();
        Scrite.document.printFormat.beginTransaction();
    }

    Component.onDestruction: {
        Scrite.document.formatting.commitTransaction();
        Scrite.document.printFormat.commitTransaction();
    }
}

