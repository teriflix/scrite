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
            width: propertyEditorView.width - 20
            height: 10
        }
        footer: Item {
            width: propertyEditorView.width - 20
            height: 10
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

                property var propertyInfo: parent.propertyInfo
                property var propertyValue: annotation.attributes[ propertyInfo.name ]

                function changePropertyValue(newValue) {
                    var attrs = annotation.attributes
                    attrs[propertyInfo.name] = newValue
                    annotation.attributes = attrs
                }

                sourceComponent: {
                    switch(propertyInfo.type) {
                    case "color": return colorEditor
                    case "number": return numberEditor
                    case "boolean": return booleanEditor
                    case "text": return textEditor
                    case "fontFamily": return fontFamilyEditor
                    case "fontStyle": return fontStyleEditor
                    case "hAlign": return hAlignEditor
                    case "vAlign": return vAlignEditor
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
            wrapMode: Text.WordWrap
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
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
        id: unknownEditor

        Item { }
    }
}
