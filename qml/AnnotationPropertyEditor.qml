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

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import io.scrite.components 1.0

// For use from within StructureView.qml only!

Item {
    property Annotation annotation

    Flickable {
        id: propertyEditorView
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: scrollBarVisible ? 0 : 10
        contentWidth: propertyEditorItems.width
        contentHeight: propertyEditorItems.height
        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

        property bool scrollBarVisible: contentHeight > height
        ScrollBar.vertical: ScrollBar2 { flickable: propertyEditorView }

        Column {
            id: propertyEditorItems
            width: propertyEditorView.width - (propertyEditorView.scrollBarVisible ? 20 : 0)
            spacing: 20

            Column {
                width: parent.width
                spacing: parent.spacing/4

                Text {
                    width: parent.width
                    font.pointSize: Scrite.app.idealFontPointSize + 2
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    padding: 10
                    text: annotation.type.toUpperCase()
                }

                Text {
                    width: parent.width
                    font.pointSize: Scrite.app.idealFontPointSize
                    horizontalAlignment: Text.AlignHCenter
                    text: "<b>Position:</b> " + Math.round(annotation.geometry.x-canvasItemsBoundingBox.left) + ", " + Math.round(annotation.geometry.y-canvasItemsBoundingBox.top) + ". <b>Size:</b> " + Math.round(annotation.geometry.width) + " x " + Math.round(annotation.geometry.height)
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            Repeater {
                model: annotation ? annotation.metaData : 0

                Column {
                    property var propertyInfo: annotation.metaData[index]
                    spacing: 3
                    width: propertyEditorView.width - (propertyEditorView.scrollBarVisible ? 20 : 0)
                    visible: propertyInfo.visible === true

                    Text {
                        width: parent.width
                        text: propertyInfo.title
                        font.pointSize: Scrite.app.idealFontPointSize
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
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                Button2 {
                    text: "Bring To Front"
                    onClicked: {
                        var a = annotationGripLoader.annotation
                        annotationGripLoader.reset()
                        Scrite.document.structure.bringToFront(a)
                    }
                }

                Button2 {
                    text: "Send To Back"
                    onClicked: {
                        var a = annotationGripLoader.annotation
                        annotationGripLoader.reset()
                        Scrite.document.structure.sendToBack(a)
                    }
                }
            }

            Button2 {
                text: "Delete Annotation"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    var a = annotationGripLoader.annotation
                    annotationGripLoader.reset()
                    Scrite.document.structure.removeAnnotation(a)
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
                                var newColor = Scrite.app.pickColor(propertyValue)
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
                font.pointSize: Scrite.app.idealFontPointSize
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
            font.pointSize: Scrite.app.idealFontPointSize
            height: Math.max(80, contentHeight) + topPadding + bottomPadding
            padding: 7.5
            onTextChanged: Qt.callLater(commitTextChanges)
            function commitTextChanges() {
                changePropertyValue(text)
            }
            selectByKeyboard: true
            selectByMouse: true
            wrapMode: Text.WordWrap
            placeholderText: propertyInfo.placeHolderText
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
            Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
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

            Text {
                width: parent.width
                font.pointSize: Scrite.app.idealFontPointSize-1
                visible: propertyValue != urlField.text
                text: "Press " + (Scrite.app.isMacOSPlatform ? "Return" : "Enter") + " key to set."
            }
        }
    }

    Component {
        id: fontFamilyEditor

        Column {
            id: fontFamilyEditorItem

            Text {
                rightPadding: changeFontButton.width + 5
                font.pointSize: Scrite.app.idealFontPointSize
                text: propertyValue
                height: 42
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                width: parent.width

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: fontListViewArea.visible = !fontListViewArea.visible
                }

                ToolButton3 {
                    id: changeFontButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height
                    down: fontListViewArea.visible
                    iconSource: fontListViewArea.visible ? "../icons/action/keyboard_arrow_up.png" : "../icons/action/keyboard_arrow_down.png"
                    onClicked: fontListViewArea.visible = !fontListViewArea.visible
                }
            }

            Rectangle {
                id: fontListViewArea
                color: primaryColors.c50.background
                width: parent.width - 10
                border.width: 1
                border.color: primaryColors.borderColor
                height: 200 + fontSearchBar.height
                visible: false
                anchors.right: parent.right
                onVisibleChanged: {
                    if(visible && fontListView.systemFontInfo === undefined)
                        fontListView.systemFontInfo = Scrite.app.systemFontInfo()
                    if(visible)
                        Scrite.app.execLater(fontListViewArea, 100, adjustScroll)
                }

                function adjustScroll() {
                    var pt = fontFamilyEditorItem.mapToItem(propertyEditorItems, 0, 0)
                    if(pt.y < propertyEditorView.contentY)
                        propertyEditorView.contentY = Math.max(pt.y-10, 0)
                    else if(pt.y + fontFamilyEditorItem.height > propertyEditorView.contentY + propertyEditorView.height)
                        propertyEditorView.contentY = (pt.y + fontFamilyEditorItem.height + 10 - propertyEditorView.height)
                }

                TextField {
                    id: fontSearchBar
                    width: parent.width
                    placeholderText: "search for a font"
                    anchors.top: parent.top
                    font.pointSize: Scrite.app.idealFontPointSize
                    onTextEdited: Qt.callLater(highlightFont)
                    function highlightFont() {
                        var utext = text.toUpperCase()
                        var checkFn = function(arg) {
                            return arg.toUpperCase().indexOf(utext) === 0
                        }
                        var families = fontListView.systemFontInfo.families
                        var index = families.findIndex(checkFn)
                        if(index >= 0) {
                            fontListView.currentIndex = index
                            changePropertyValue(families[index])
                        }
                    }
                }

                ListView {
                    id: fontListView
                    property var systemFontInfo
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    anchors.top: fontSearchBar.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    model: systemFontInfo ? systemFontInfo.families : 0
                    highlight: Rectangle {
                        color: Scrite.app.palette.highlight
                    }
                    highlightMoveDuration: 0
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    keyNavigationEnabled: false
                    delegate: Text {
                        font.family: modelData
                        font.pointSize: Scrite.app.idealFontPointSize
                        text: modelData
                        width: fontListView.width-20
                        color: fontListView.currentIndex === index ? Scrite.app.palette.highlightedText : "black"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: changePropertyValue(modelData)
                        }
                        padding: 4
                    }
                    ScrollBar.vertical: ScrollBar2 { flickable: propertyEditorView }
                    currentIndex: systemFontInfo ? systemFontInfo.families.indexOf(propertyValue) : -1
                }
            }
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

                Text {
                    text: propertyValue == "" ? "Set" : "Change"
                    color: "blue"
                    font.underline: true
                    font.pointSize: Scrite.app.idealFontPointSize

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
                    font.pointSize: Scrite.app.idealFontPointSize

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
