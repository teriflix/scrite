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
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    required property Annotation annotation

    required property BoundingBoxEvaluator canvasItemsBoundingBox

    Flickable {
        id: propertyEditorView
        clip: true
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: scrollBarVisible ? 0 : 10
        contentWidth: propertyEditorItems.width
        contentHeight: propertyEditorItems.height
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        property bool scrollBarVisible: contentHeight > height
        ScrollBar.vertical: VclScrollBar { flickable: propertyEditorView }

        Column {
            id: propertyEditorItems
            width: propertyEditorView.width - (propertyEditorView.scrollBarVisible ? 20 : 0)
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
                    text: annotation.type.toUpperCase()
                }

                VclLabel {
                    width: parent.width
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    horizontalAlignment: Text.AlignHCenter
                    text: "<b>Position:</b> " + Math.round(annotation.geometry.x-root.canvasItemsBoundingBox.left) + ", " + Math.round(annotation.geometry.y-root.canvasItemsBoundingBox.top) + ". <b>Size:</b> " + Math.round(annotation.geometry.width) + " x " + Math.round(annotation.geometry.height)
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            Repeater {
                model: annotation ? annotation.metaData : 0

                Column {
                    property var propertyInfo: annotation.metaData[index]
                    spacing: 3
                    width: propertyEditorView.width - (propertyEditorView.scrollBarVisible ? 20 : 0)
                    visible: propertyInfo.visible === true

                    VclLabel {
                        width: parent.width
                        text: propertyInfo.title
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        font.bold: true
                    }

                    Loader {
                        id: editorLoader
                        width: parent.width - 20
                        anchors.right: parent.right
                        enabled: !Scrite.document.readOnly

                        property var propertyInfo: parent.propertyInfo
                        property var propertyValue: annotation.attributes[ propertyInfo.name ]

                        function changePropertyValue(newValue) {
                            var attrs = annotation.attributes
                            attrs[propertyInfo.name] = newValue
                            annotation.attributes = attrs
                            annotation.saveAttributesAsDefault()
                        }

                        active: propertyInfo.visible === true
                        sourceComponent: {
                            switch(propertyInfo.type) {
                            case "color": return colorEditor
                            case "number": return numberEditor
                            case "boolean": return booleanEditor
                            case "text": return textEditor
                            case "url": return urlEditor
                            case "fontFamily": return fontFamilyEditor
                            case "fontStyle": return fontStyleEditor
                            case "hAlign": return hAlignEditor
                            case "vAlign": return vAlignEditor
                            case "image": return imageEditor
                            }
                            return unknownEditor
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
                        // var a = annotationGripLoader.annotation
                        // annotationGripLoader.reset()
                        Scrite.document.structure.bringToFront(annotation)
                    }
                }

                VclButton {
                    text: "Send To Back"
                    onClicked: {
                        // var a = annotationGripLoader.annotation
                        // annotationGripLoader.reset()
                        Scrite.document.structure.sendToBack(annotation)
                    }
                }
            }

            VclButton {
                text: "Delete Annotation"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    // var a = annotationGripLoader.annotation
                    // annotationGripLoader.reset()
                    Scrite.document.structure.removeAnnotation(annotation)
                }
            }

            Item {
                width: parent.width
                height: 10
            }
        }
    }

    Component {
        id: colorEditor

        Row {
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
                                var newColor = Scrite.app.pickColor(propertyValue)
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
        id: numberEditor

        Row {
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
        id: booleanEditor

        VclCheckBox {
            text: propertyInfo.text
            checked: propertyValue
            checkable: true
            onToggled: changePropertyValue(checked)
        }
    }

    Component {
        id: textEditor

        TextArea {
            id: _textArea

            function commitTextChanges() {
                changePropertyValue(text)
            }

            SyntaxHighlighter.delegates: [
                LanguageFontSyntaxHighlighterDelegate {
                    enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                    defaultFont: _textArea.font
                }
            ]
            SyntaxHighlighter.textDocument: textDocument

            LanguageTransliterator.popup: LanguageTransliteratorPopup {
                editorFont: _textArea.font
            }
            LanguageTransliterator.option: Runtime.language.activeTransliterationOption
            LanguageTransliterator.enabled: !readOnly

            text: propertyValue
            height: Math.max(80, contentHeight) + topPadding + bottomPadding
            padding: 7.5

            wrapMode: Text.WordWrap
            selectByMouse: true
            placeholderText: propertyInfo.placeHolderText
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
                    var pt = mapToItem(propertyEditorItems, cursorRectangle.x, cursorRectangle.y)
                    if(pt.y < propertyEditorView.contentY)
                        propertyEditorView.contentY = Math.max(pt.y-10, 0)
                    else if(pt.y + cursorRectangle.height > propertyEditorView.contentY + propertyEditorView.height)
                        propertyEditorView.contentY = (pt.y + cursorRectangle.height + 10 - propertyEditorView.height)
                }
            }
        }
    }

    Component {
        id: urlEditor

        Column {
            TextField {
                id: urlField
                text: propertyValue
                onAccepted: changePropertyValue(text)
                placeholderText: "Enter URL and press " + (Scrite.app.isMacOSPlatform ? "Return" : "Enter") + " key to set."
                width: parent.width
            }

            VclLabel {
                width: parent.width
                font.pointSize: Runtime.idealFontMetrics.font.pointSize-1
                visible: propertyValue != urlField.text
                text: "Press " + (Scrite.app.isMacOSPlatform ? "Return" : "Enter") + " key to set."
            }
        }
    }

    Component {
        id: fontFamilyEditor

        VclButton {
            id: fontFamilyButton

            Layout.fillWidth: true

            text: propertyValue
            font.family: propertyValue
            font.pointSize: Scrite.app.idealFontPointSize

            contentItem: VclLabel {
                text: fontFamilyButton.text
                font: fontFamilyButton.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            onClicked: FontSelectionDialog.launchWithTitle("Select a font for annotation", (family) => {
                                                               if(family !== "")
                                                                   changePropertyValue(family)
                                                           }, propertyValue)
        }
    }

    Component {
        id: fontStyleEditor

        Row {
            spacing: 5

            Repeater {
                model: ['bold', 'italic', 'underline']

                VclCheckBox {
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
        id: hAlignEditor

        Row {
            spacing: 5

            Repeater {
                model: ['left', 'center', 'right']

                VclRadioButton {
                    text: modelData
                    font.capitalization: Font.Capitalize
                    checked: modelData === propertyValue
                    onToggled: changePropertyValue(modelData)
                }
            }
        }
    }

    Component {
        id: vAlignEditor

        Row {
            spacing: 5

            Repeater {
                model: ['top', 'center', 'bottom']

                VclRadioButton {
                    text: modelData
                    font.capitalization: Font.Capitalize
                    checked: modelData === propertyValue
                    onToggled: changePropertyValue(modelData)
                }
            }
        }
    }

    Component {
        id: imageEditor

        Rectangle {
            height: (width/16)*9
            color: Runtime.colors.primary.c100.background
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            VclFileDialog {
                id: fileDialog
                nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                selectFolder: false
                selectMultiple: false
                sidebarVisible: true
                selectExisting: true
                 // The default Ctrl+U interfers with underline
                onAccepted: {
                    if(fileUrl != "") {
                        if(propertyValue != "")
                            annotation.removeImage(propertyValue)
                        var newImageName = annotation.addImage(Scrite.app.urlToLocalFile(fileUrl))
                        changePropertyValue(newImageName)
                    }
                }
            }

            Image {
                id: image
                anchors.fill: parent
                anchors.margins: 1
                fillMode: Image.PreserveAspectFit
                source: annotation.imageUrl(propertyValue)
                asynchronous: true
            }

            BusyIcon {
                anchors.centerIn: parent
                running: parent.status === Image.Loading
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: image.opacity = containsMouse ? 0.25 : 1
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
                        onClicked: fileDialog.open()
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
                                annotation.removeImage(propertyValue)
                            changePropertyValue("")
                        }
                    }
                }
            }
        }
    }

    Component {
        id: unknownEditor

        Item { }
    }
}
