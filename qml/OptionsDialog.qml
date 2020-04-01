/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    width: 700
    height: 500

    Item {
        anchors.fill: parent
        anchors.margins: 20

        Rectangle {
            anchors.fill: pageList
            anchors.margins: -4
            radius: 5
            border { width: 1; color: "black" }
        }

        ListModel {
            id: pageModel
            ListElement { name: "Title Page"; group: "Screenplay" }
            ListElement { name: "Heading"; group: "Formatting"; elementType: SceneElement.Heading }
            ListElement { name: "Action"; group: "Formatting"; elementType: SceneElement.Action }
            ListElement { name: "Character"; group: "Formatting"; elementType: SceneElement.Character }
            ListElement { name: "Dialogue"; group: "Formatting"; elementType: SceneElement.Dialogue }
            ListElement { name: "Parenthetical"; group: "Formatting"; elementType: SceneElement.Parenthetical }
            ListElement { name: "Shot"; group: "Formatting"; elementType: SceneElement.Shot }
            ListElement { name: "Transition"; group: "Formatting"; elementType: SceneElement.Transition }
            ListElement { name: "Settings"; group: "Application"; }
        }

        ListView {
            id: pageList
            height: parent.height
            width: 170
            model: pageModel
            spacing: 5
            section.property: "group"
            section.criteria: ViewSection.FullString
            section.delegate: Rectangle {
                width: pageList.width
                height: 30
                color: "lightsteelblue"

                Text {
                    anchors.centerIn: parent
                    font.pixelSize: 14
                    font.letterSpacing: 2
                    text: section
                }
            }
            highlight: Rectangle {
                color: "lightgray"
                radius: 5
            }

            delegate: Text {
                width: pageList.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                height: 32
                font.pixelSize: 18
                font.bold: pageList.currentIndex === index
                text: name
                MouseArea {
                    anchors.fill: parent
                    onClicked: pageList.currentIndex = index
                }
            }
        }

        Loader {
            anchors.left: pageList.right
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            active: pageList.currentIndex >= 0
            sourceComponent: {
                if(pageList.currentIndex >= 1 && pageList.currentIndex <= 7)
                    return elementFormatOptionsComponent

                switch(pageList.currentIndex) {
                case 0: return screenplayOptionsComponent
                case 8: return applicationSettingsComponent
                }
            }
        }

    }

    Component {
        id: screenplayOptionsComponent

        Item {
            property real labelWidth: 100

            Column {
                width: parent.width
                spacing: 20

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: "Title Page Settings"
                    font.pixelSize: 24
                }

                Item { width: parent.width; height: 1 }

                // Title field
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Title"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.title
                        onTextEdited: scriteDocument.screenplay.title = text
                        font.pixelSize: 14
                    }
                }

                // Author field
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Author"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.author
                        onTextEdited: scriteDocument.screenplay.author = text
                        font.pixelSize: 14
                    }
                }

                // Contact field
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Contact"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.contact
                        onTextEdited: scriteDocument.screenplay.contact = text
                        font.pixelSize: 14
                    }
                }

                // Version field
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Version"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.version
                        onTextEdited: scriteDocument.screenplay.version = text
                        font.pixelSize: 14
                    }
                }
            }
        }
    }

    property var systemFontInfo: app.systemFontInfo()

    Component {
        id: elementFormatOptionsComponent

        ScrollView {
            id: scrollView
            property real labelWidth: 125
            property var pageData: pageModel.get(pageList.currentIndex)
            property SceneElementFormat format: scriteDocument.formatting.elementFormat(pageData.elementType)

            Column {
                width: scrollView.width
                spacing: 10

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: pageData.name + " - Element Format"
                    font.pixelSize: 24
                }

                // Font Family
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Font Family"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    ComboBox {
                        width: parent.width-parent.spacing-labelWidth
                        model: systemFontInfo.families
                        currentIndex: systemFontInfo.families.indexOf(format.font.family)
                        onCurrentIndexChanged: format.font.family = systemFontInfo.families[currentIndex]
                    }
                }

                // Font Size
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Font Size"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SpinBox {
                        width: parent.width-parent.spacing-labelWidth
                        from: 9
                        to: 62
                        stepSize: 1
                        editable: true
                        value: format.font.pointSize
                        onValueModified: format.font.pointSize = value
                    }
                }

                // Font Style
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Font Style"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        CheckBox {
                            text: "Bold"
                            font.bold: true
                            checkable: true
                            checked: format.font.bold
                        }

                        CheckBox {
                            text: "Italics"
                            font.italic: true
                            checkable: true
                            checked: format.font.italic
                        }

                        CheckBox {
                            text: "Underline"
                            font.underline: true
                            checkable: true
                            checked: format.font.underline
                        }
                    }
                }

                // Colors
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Text Color"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Rectangle {
                        border.width: 1
                        border.color: "black"
                        color: format.textColor
                        width: 30; height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: format.textColor = app.pickColor(format.textColor)
                        }
                    }

                    Text {
                        horizontalAlignment: Text.AlignRight
                        text: "Background Color"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Rectangle {
                        border.width: 1
                        border.color: "black"
                        color: format.backgroundColor
                        width: 30; height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: format.backgroundColor = app.pickColor(format.backgroundColor)
                        }
                    }
                }

                // Text Alignment
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Text Alignment"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        RadioButton {
                            text: "Left"
                            checkable: true
                            checked: format.textAlignment === Qt.AlignLeft
                            onCheckedChanged: {
                                if(checked)
                                    format.textAlignment = Qt.AlignLeft
                            }
                        }

                        RadioButton {
                            text: "Center"
                            checkable: true
                            checked: format.textAlignment === Qt.AlignHCenter
                            onCheckedChanged: {
                                if(checked)
                                    format.textAlignment = Qt.AlignHCenter
                            }
                        }

                        RadioButton {
                            text: "Right"
                            checkable: true
                            checked: format.textAlignment === Qt.AlignRight
                            onCheckedChanged: {
                                if(checked)
                                    format.textAlignment = Qt.AlignRight
                            }
                        }

                        RadioButton {
                            text: "Justify"
                            checkable: true
                            checked: format.textAlignment === Qt.AlignJustify
                            onCheckedChanged: {
                                if(checked)
                                    format.textAlignment = Qt.AlignJustify
                            }
                        }
                    }
                }

                // Block Margin
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Block Width"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SpinBox {
                        width: parent.width-parent.spacing-labelWidth
                        from: 0
                        to: 100
                        stepSize: 1
                        value: format.blockWidth * 100
                        onValueModified: format.blockWidth = value/100
                        textFromValue: function(value,locale) {
                            return value + "%"
                        }
                    }
                }

                // Block Alignment
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Block Alignment"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        RadioButton {
                            text: "Left"
                            checkable: true
                            checked: format.blockAlignment === Qt.AlignLeft
                        }

                        RadioButton {
                            text: "Center"
                            checkable: true
                            checked: format.blockAlignment === Qt.AlignHCenter
                        }

                        RadioButton {
                            text: "Right"
                            checkable: true
                            checked: format.blockAlignment === Qt.AlignRight
                        }
                    }
                }

                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Margins"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        Text {
                            text: "Top"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        TextField {
                            width: 100
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                            text: format.topMargin
                            validator: IntValidator { top: 100; bottom: 0 }
                            onTextChanged: format.topMargin = parseInt(text)
                        }

                        Text {
                            text: "Bottom"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        TextField {
                            width: 100
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                            text: format.bottomMargin
                            validator: IntValidator { top: 100; bottom: 0 }
                            onTextChanged: format.bottomMargin = parseInt(text)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: applicationSettingsComponent

        Item {
            Column {
                width: parent.width * 0.8
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                GroupBox {
                    width: parent.width
                    label: CheckBox {
                        text: "Enable AutoSave"
                        checked: scriteDocument.autoSave
                        onToggled: scriteDocument.autoSave = checked
                    }

                    Column {
                        width: parent.width
                        spacing: 10
                        enabled: scriteDocument.autoSave

                        Text {
                            width: parent.width
                            text: "Auto Save Interval (in seconds)"
                        }

                        TextField {
                            width: parent.width
                            text: scriteDocument.autoSaveDurationInSeconds
                            validator: IntValidator {
                                bottom: 1; top: 3600
                            }
                            onTextEdited: scriteDocument.autoSaveDurationInSeconds = parseInt(text)
                        }
                    }
                }
            }
        }
    }
}
