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
        anchors.margins: 5
        model: annotation ? annotation.metaData : 0
        spacing: 20
        delegate: Column {
            property var propertyInfo: annotation.metaData[index]
            spacing: 3
            width: propertyEditorView.width

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

        Item { }
    }

    Component {
        id: textEditor

        Item { }
    }

    Component {
        id: unknownEditor

        Item { }
    }
}
