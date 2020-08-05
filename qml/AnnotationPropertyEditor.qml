/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

// For use from within StructureView.qml only!

Item {
    property Annotation annotation

    ListView {
        id: propertyEditorView
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: scrollBarVisible ? 0 : 10
        model: annotation ? annotation.metaData : 0
        spacing: 20
        property bool scrollBarVisible: contentHeight > height
        header: Item {
            width: propertyEditorView.width - (propertyEditorView.scrollBarVisible ? 20 : 0)
            height: 80

            Button2 {
                text: "Delete Annotation"
                anchors.centerIn: parent
                onClicked: {
                    var a = annotationGripLoader.annotation
                    annotationGripLoader.reset()
                    scriteDocument.structure.removeAnnotation(a)
                }
            }
        }
        footer: Item {
            width: propertyEditorView.width - 20
            height: 80

            Row {
                spacing: 10
                anchors.centerIn: parent

                Button2 {
                    text: "Bring To Front"
                    onClicked: {
                        var a = annotationGripLoader.annotation
                        annotationGripLoader.reset()
                        scriteDocument.structure.bringToFront(a)
                    }
                }

                Button2 {
                    text: "Send To Back"
                    onClicked: {
                        var a = annotationGripLoader.annotation
                        annotationGripLoader.reset()
                        scriteDocument.structure.sendToBack(a)
                    }
                }
            }
        }
        ScrollBar.vertical: ScrollBar {
            policy: propertyEditorView.scrollBarVisible ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.25)
                dark: Qt.rgba(0,0,0,0.75)
            }
            opacity: active ? 1 : 0.2
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
        }
        delegate: Column {
            property var propertyInfo: annotation.metaData[index]
            spacing: 3
            width: propertyEditorView.width - (propertyEditorView.scrollBarVisible ? 20 : 0)

            Text {
                width: parent.width
                text: propertyInfo.title
                font.pointSize: app.idealFontPointSize
                font.bold: true
            }

            Loader {
                id: editorLoader
                width: parent.width - 20
                anchors.right: parent.right
                enabled: !scriteDocument.readOnly

                property var propertyInfo: parent.propertyInfo
                property var propertyValue: annotation.attributes[ propertyInfo.name ]

                function changePropertyValue(newValue) {
                    var attrs = annotation.attributes
                    attrs[propertyInfo.name] = newValue
                    annotation.attributes = attrs
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

                    menu: Menu2 {
                        ColorMenu {
                            title: "Standard Colors"
                            onMenuItemClicked: {
                                colorMenu.close()
                                changePropertyValue(color)
                            }
                        }

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Custom Color"
                            onClicked: {
                                var newColor = app.pickColor(propertyValue)
                                changePropertyValue( "" + newColor )
                                colorMenu.close()
                            }
                        }
                    }
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: propertyValue
                font.capitalization: Font.AllUppercase
                font.pointSize: app.idealFontPointSize
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
                onValueModified: changePropertyValue(value)
            }
        }
    }

    Component {
        id: booleanEditor

        CheckBox2 {
            text: propertyInfo.text
            checked: propertyValue
            checkable: true
            onToggled: changePropertyValue(checked)
        }
    }

    Component {
        id: textEditor

        TextArea {
            background: Rectangle {
                color: primaryColors.c50.background
                border.width: 1
                border.color: primaryColors.borderColor
            }
            text: propertyValue
            font.pointSize: app.idealFontPointSize
            height: 150
            padding: 7.5
            onTextChanged: Qt.callLater(commitTextChanges)
            function commitTextChanges() {
                changePropertyValue(text)
            }
            selectByKeyboard: true
            selectByMouse: true
            wrapMode: Text.WordWrap
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
        }
    }

    Component {
        id: urlEditor

        Column {
            TextField {
                id: urlField
                text: propertyValue
                onAccepted: changePropertyValue(text)
                placeholderText: "Enter URL and press " + (app.isMacOSPlatform ? "Return" : "Enter") + " key to set."
                width: parent.width
            }

            Text {
                width: parent.width
                font.pointSize: app.idealFontPointSize-1
                visible: propertyValue != urlField.text
                text: "Press " + (app.isMacOSPlatform ? "Return" : "Enter") + " key to set."
            }
        }
    }

    Component {
        id: fontFamilyEditor

        ComboBox2 {
            readonly property var systemFontInfo: app.systemFontInfo()
            model: systemFontInfo.families
            editable: true
            currentIndex: systemFontInfo.families.indexOf(propertyValue)
            onActivated: changePropertyValue(currentText)
        }
    }

    Component {
        id: fontStyleEditor

        Row {
            spacing: 5

            Repeater {
                model: ['bold', 'italic', 'underline']

                CheckBox2 {
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

                RadioButton2 {
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

                RadioButton2 {
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
            color: primaryColors.c100.background
            border.width: 1
            border.color: primaryColors.borderColor

            FileDialog {
                id: fileDialog
                nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                selectFolder: false
                selectMultiple: false
                sidebarVisible: true
                selectExisting: true
                onAccepted: {
                    if(fileUrl != "") {
                        if(propertyValue != "")
                            annotation.removeImage(propertyValue)
                        var newImageName = annotation.addImage(app.urlToLocalFile(fileUrl))
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

            BusyIndicator {
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

                Text {
                    text: propertyValue == "" ? "Set" : "Change"
                    color: "blue"
                    font.underline: true
                    font.pointSize: app.idealFontPointSize

                    MouseArea {
                        anchors.fill: parent
                        onClicked: fileDialog.open()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                Text {
                    text: "Remove"
                    color: "blue"
                    font.underline: true
                    visible: propertyValue != ""
                    font.pointSize: app.idealFontPointSize

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
