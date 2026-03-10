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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../dialogs"
import "../../helpers"
import "../../controls"

Item {
    id: root

    required property Annotation annotation
    required property BoundingBoxEvaluator canvasItemsBoundingBox

    Flickable {
        id: _propertyEditorView

        clip: true
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: scrollBarVisible ? 0 : 10
        contentWidth: _propertyEditorItems.width
        contentHeight: _propertyEditorItems.height
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        property bool scrollBarVisible: contentHeight > height
        ScrollBar.vertical: VclScrollBar { flickable: _propertyEditorView }

        Column {
            id: _propertyEditorItems

            width: _propertyEditorView.width - (_propertyEditorView.scrollBarVisible ? 20 : 0)
            spacing: 20

            Column {
                width: parent.width
                spacing: parent.spacing/4

                VclLabel {
                    width: parent.width
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    padding: 10
                    text: root.annotation ? root.annotation.type.toUpperCase() : ""
                }

                VclLabel {
                    width: parent.width
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        if(root.annotation)
                            return "<b>Position:</b> " + Math.round(root.annotation.geometry.x-root.canvasItemsBoundingBox.left) + ", " +
                                                         Math.round(root.annotation.geometry.y-root.canvasItemsBoundingBox.top) + ". <b>Size:</b> " +
                                                         Math.round(root.annotation.geometry.width) + " x " + Math.round(root.annotation.geometry.height)
                        return ""
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            Repeater {
                model: root.annotation ? root.annotation.metaData : 0

                delegate: Column {
                    id: _editorDelegate

                    required property int index
                    required property var modelData

                    property var propertyInfo: modelData
                    property var propertyValue: root.annotation.attributes[ propertyInfo.name ]

                    spacing: 3
                    width: _propertyEditorView.width - (_propertyEditorView.scrollBarVisible ? 20 : 0)
                    visible: propertyInfo.visible === true

                    VclLabel {
                        width: parent.width
                        text: propertyInfo.title
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        font.bold: true
                    }

                    Loader {
                        id: _editorLoader

                        property alias propertyInfo: _editorDelegate.propertyInfo
                        property alias propertyValue: _editorDelegate.propertyValue

                        function changePropertyValue(newValue) {
                            var attrs = root.annotation.attributes
                            attrs[propertyInfo.name] = newValue
                            root.annotation.attributes = attrs
                            root.annotation.saveAttributesAsDefault()
                        }

                        width: parent.width - 20
                        anchors.right: parent.right
                        enabled: !Scrite.document.readOnly

                        active: propertyInfo.visible === true
                        sourceComponent: {
                            switch(propertyInfo.type) {
                            case "color": return _colorEditorComponent
                            case "number": return _numberEditorComponent
                            case "boolean": return _booleanEditorComponent
                            case "text": return _textEditorComponent
                            case "url": return _urlEditorComponent
                            case "fontFamily": return _fontFamilyEditorComponent
                            case "fontStyle": return _fontStyleEditorComponent
                            case "hAlign": return _hAlignEditorComponent
                            case "vAlign": return _vAlignEditorComponent
                            case "_imageEditorImage": return _imageEditorComponent
                            }
                            return _unknownEditorComponent
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                VclButton {
                    text: "Bring To Front"
                    onClicked: {
                        // var a = annotationGripLoader.root.annotation
                        // annotationGripLoader.reset()
                        Scrite.document.structure.bringToFront(root.annotation)
                    }
                }

                VclButton {
                    text: "Send To Back"
                    onClicked: {
                        // var a = annotationGripLoader.root.annotation
                        // annotationGripLoader.reset()
                        Scrite.document.structure.sendToBack(root.annotation)
                    }
                }
            }

            VclButton {
                text: "Delete Annotation"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    // var a = annotationGripLoader.root.annotation
                    // annotationGripLoader.reset()
                    Scrite.document.structure.removeAnnotation(root.annotation)
                }
            }

            Item {
                width: parent.width
                height: 10
            }
        }
    }

    Component {
        id: _colorEditorComponent

        Row {
            id: _colorEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            height: 40
            spacing: 10

            Rectangle {
                width: 30; height: 30
                anchors.verticalCenter: parent.verticalCenter
                color: propertyValue
                border { width: 1; color: "black" }

                MouseArea {
                    anchors.fill: parent
                    onClicked: colorMenu.show()
                }

                MenuLoader {
                    id: colorMenu
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    menu: VclMenu {
                        ColorMenu {
                            title: "Standard Colors"
                            onMenuItemClicked: {
                                colorMenu.close()
                                changePropertyValue(color)
                            }
                        }

                        MenuSeparator { }

                        VclMenuItem {
                            text: "Custom Color"
                            onClicked: {
                                var newColor = Color.pick(propertyValue)
                                changePropertyValue( "" + newColor )
                                colorMenu.close()
                            }
                        }
                    }
                }
            }

            VclLabel {
                anchors.verticalCenter: parent.verticalCenter
                text: propertyValue
                font.capitalization: Font.AllUppercase
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }
        }
    }

    Component {
        id: _numberEditorComponent

        Row {
            id: _numberEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            SpinBox {
                value: propertyValue
                from: propertyInfo.min
                to: propertyInfo.max
                stepSize: propertyInfo.step
                editable: true
                onValueModified: changePropertyValue(value)
            }
        }
    }

    Component {
        id: _booleanEditorComponent

        VclCheckBox {
            id: _booleanEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            text: propertyInfo.text
            checked: propertyValue
            checkable: true
            onToggled: changePropertyValue(checked)
        }
    }

    Component {
        id: _textEditorComponent

        TextArea {
            id: _textEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            function commitTextChanges() {
                changePropertyValue(text)
            }

            SyntaxHighlighter.delegates: [
                LanguageFontSyntaxHighlighterDelegate {
                    enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                    defaultFont: _textEditor.font
                }
            ]
            SyntaxHighlighter.textDocument: textDocument

            DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

            LanguageTransliterator.popup: LanguageTransliteratorPopup {
                editorFont: _textEditor.font
            }
            LanguageTransliterator.option: Runtime.language.activeTransliterationOption
            LanguageTransliterator.enabled: !readOnly

            text: propertyValue
            height: Math.max(80, contentHeight) + topPadding + bottomPadding
            padding: 7.5

            wrapMode: Text.WordWrap
            selectByMouse: true
            placeholderText: typeof propertyInfo.placeHolderText === "string" ? propertyInfo.placeHolderText : ""
            selectByKeyboard: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize

            background: Rectangle {
                color: Runtime.colors.primary.c50.background
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
            }

            onTextChanged: Qt.callLater(commitTextChanges)

            onCursorRectangleChanged: {
                if(activeFocus) {
                    var pt = mapToItem(_propertyEditorItems, cursorRectangle.x, cursorRectangle.y)
                    if(pt.y < _propertyEditorView.contentY)
                        _propertyEditorView.contentY = Math.max(pt.y-10, 0)
                    else if(pt.y + cursorRectangle.height > _propertyEditorView.contentY + _propertyEditorView.height)
                        _propertyEditorView.contentY = (pt.y + cursorRectangle.height + 10 - _propertyEditorView.height)
                }
            }
        }
    }

    Component {
        id: _urlEditorComponent

        Column {
            id: _urlEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            TextField {
                id: urlField
                text: propertyValue
                onAccepted: changePropertyValue(text)
                placeholderText: "Enter URL and press " + (Platform.isMacOSDesktop ? "Return" : "Enter") + " key to set."
                width: parent.width
            }

            VclLabel {
                width: parent.width
                font.pointSize: Runtime.idealFontMetrics.font.pointSize-1
                visible: propertyValue != urlField.text
                text: "Press " + (Platform.isMacOSDesktop ? "Return" : "Enter") + " key to set."
            }
        }
    }

    Component {
        id: _fontFamilyEditorComponent

        VclButton {
            id: _fontFamilyEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            Layout.fillWidth: true

            text: propertyValue
            font.family: propertyValue
            font.pointSize: Runtime.idealFontMetrics.font.pointSize

            contentItem: VclLabel {
                text: _fontFamilyEditor.text
                font: _fontFamilyEditor.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            onClicked: FontSelectionDialog.launchWithTitle("Select a font for root.annotation", (family) => {
                                                               if(family !== "")
                                                                   changePropertyValue(family)
                                                           }, propertyValue)
        }
    }

    Component {
        id: _fontStyleEditorComponent

        Row {
            id: _fontStyleEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            spacing: 5

            Repeater {
                model: ['bold', 'italic', 'underline']

                delegate: VclCheckBox {
                    required property int index
                    required property string modelData

                    text: modelData
                    font.capitalization: Font.Capitalize
                    font.bold: index === 0
                    font.italic: index === 1
                    font.underline: index === 2
                    checked: propertyValue.indexOf(modelData) >= 0
                    onToggled: {
                        var pv = Array.isArray(propertyValue) ? propertyValue : []
                        if(checked)
                            pv.push(modelData)
                        else
                            pv.splice(propertyValue.indexOf(modelData), 1)
                        changePropertyValue(pv)
                    }
                }
            }
        }
    }

    Component {
        id: _hAlignEditorComponent

        Row {
            id: _hAlignEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            spacing: 5

            Repeater {
                model: ['left', 'center', 'right']

                delegate: VclRadioButton {
                    required property int index
                    required property string modelData

                    text: modelData
                    font.capitalization: Font.Capitalize
                    checked: modelData === propertyValue
                    onToggled: changePropertyValue(modelData)
                }
            }
        }
    }

    Component {
        id: _vAlignEditorComponent

        Row {
            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            spacing: 5

            Repeater {
                model: ['top', 'center', 'bottom']

                delegate: VclRadioButton {
                    required property int index
                    required property string modelData

                    text: modelData
                    font.capitalization: Font.Capitalize
                    checked: modelData === propertyValue
                    onToggled: changePropertyValue(modelData)
                }
            }
        }
    }

    Component {
        id: _imageEditorComponent

        Rectangle {
            id: _imageEditor

            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }

            height: (width/16)*9
            color: Runtime.colors.primary.c100.background
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            VclFileDialog {
                id: _imageEditorFileDialog

                nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]

                onAccepted: {
                    if(selectedFile !== "") {
                        if(propertyValue != "")
                            root.annotation.removeImage(propertyValue)
                        var newImageName = root.annotation.addImage(Url.toPath(selectedFile))
                        changePropertyValue(newImageName)
                    }
                }
            }

            Image {
                id: _imageEditorImage
                anchors.fill: parent
                anchors.margins: 1
                fillMode: Image.PreserveAspectFit
                source: root.annotation.imageUrl(propertyValue)
                asynchronous: true
            }

            BusyIcon {
                anchors.centerIn: parent
                running: parent.status === Image.Loading
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: _imageEditorImage.opacity = containsMouse ? 0.25 : 1
            }

            Row {
                anchors.centerIn: parent
                spacing: 20

                VclLabel {
                    text: propertyValue == "" ? "Set" : "Change"
                    color: "blue"
                    font.underline: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    MouseArea {
                        anchors.fill: parent
                        onClicked: _imageEditorFileDialog.open()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                VclLabel {
                    text: "Remove"
                    color: "blue"
                    font.underline: true
                    visible: propertyValue != ""
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if(propertyValue != "")
                                root.annotation.removeImage(propertyValue)
                            changePropertyValue("")
                        }
                    }
                }
            }
        }
    }

    Component {
        id: _unknownEditorComponent

        Item {
            property var propertyInfo: parent.propertyInfo
            property var propertyValue: parent.propertyValue
            function changePropertyValue(newValue) { parent.changePropertyValue(newValue) }
        }
    }
}
