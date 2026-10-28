/****************************************************************************
**
** Copyright (C) 2026 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../helpers"
import "../../controls"
import "../../dialogs"

Item {
    id: root

    required property Notes notes

    required property real maxTextAreaSize
    required property real minTextAreaSize

    readonly property NotesKVStore kvStore: notes.kvStore

    // The category that is selected in the list of categories.
    readonly property string category: _categoriesView.currentItem ? _categoriesView.currentItem.key : ""

    readonly property real keyColumnWidth: Math.max(120, _pairsView.width * 0.3)

    readonly property real contentWidth: Math.max(root.minTextAreaSize, Math.min(root.width - 40, root.maxTextAreaSize))

    SctLabel {
        id: _description

        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        width: root.contentWidth

        opacity: 0.75
        text: _private.description
        wrapMode: Text.WordWrap

        font.pointSize: Runtime.notebookFontPointSize
    }

    SplitView {
        anchors.top: _description.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: 12
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        width: root.contentWidth

        enabled: !Scrite.document.readOnly
        orientation: Qt.Horizontal

        handle: Item {
            implicitWidth: 10
        }

        Rectangle {
            SplitView.minimumWidth: 100
            SplitView.preferredWidth: Runtime.notebookSettings.attributesCategoriesWidth

            color: Color.translucent(Runtime.colors.primary.c200.background, 0.5)
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            // Remembered, so that the split is the same for the attributes of everything else.
            onWidthChanged: {
                if(SplitView.view.resizing)
                    Runtime.notebookSettings.attributesCategoriesWidth = width
            }

            ListView {
                id: _categoriesView

                anchors.fill: parent
                anchors.margins: 1

                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                ScrollBar.vertical: SctScrollBar { }

                clip: true
                currentIndex: 0
                model: root.kvStore

                delegate: Rectangle {
                    id: _categoryDelegate

                    required property int index
                    required property string key
                    required property int pairCount

                    // True while the name of the category is being edited in the list
                    property bool editing: false

                    width: ListView.view.width
                    height: _categoryLabel.implicitHeight + 16

                    color: ListView.isCurrentItem ? Qt.rgba(0.5, 0.5, 0.5, 0.2) : "transparent"

                    Rectangle {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left

                        width: 3

                        color: Runtime.colors.accent.c900.background
                        visible: _categoryDelegate.ListView.isCurrentItem
                    }

                    SctLabel {
                        id: _categoryLabel

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: _countLabel.left
                        anchors.leftMargin: 12
                        anchors.rightMargin: 8

                        elide: Text.ElideRight
                        text: _categoryDelegate.key
                        visible: !_categoryDelegate.editing

                        font.bold: _categoryDelegate.ListView.isCurrentItem
                        font.pointSize: Runtime.notebookFontPointSize
                    }

                    SctTextField {
                        id: _renameField

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: _countLabel.left
                        anchors.leftMargin: 12
                        anchors.rightMargin: 8

                        text: _categoryDelegate.key
                        visible: _categoryDelegate.editing

                        font.bold: true
                        font.pointSize: Runtime.notebookFontPointSize

                        Keys.onEscapePressed: _categoryDelegate.editing = false

                        onVisibleChanged: {
                            if(visible) {
                                forceActiveFocus()
                                selectAll()
                            }
                        }

                        // A name that is not allowed, such as one that another category has, leaves the
                        // name as it was.
                        onEditingFinished: {
                            if(!_categoryDelegate.editing)
                                return

                            _categoryDelegate.editing = false
                            if(text !== _categoryDelegate.key)
                                _private.rename(_categoryDelegate.key, text)
                        }
                    }

                    SctLabel {
                        id: _countLabel

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 12

                        opacity: 0.6
                        text: _categoryDelegate.pairCount
                        visible: !_categoryHover.hovered || _categoryDelegate.index === 0 || _categoryDelegate.editing

                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }

                    HoverHandler {
                        id: _categoryHover
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: _private.select(_categoryDelegate.index, _categoryDelegate.key)
                        onDoubleClicked: {
                            if(_categoryDelegate.index > 0)
                                _categoryDelegate.editing = true
                        }
                    }

                    // Shows where the count is, when the pointer is over a category that can be removed.
                    FlatToolButton {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 4

                        suggestedWidth: 28

                        iconSource: Runtime.themedIcon("qrc:/icons/action/delete.png")
                        toolTipText: "Remove this category"
                        visible: _categoryHover.hovered && _categoryDelegate.index > 0 && !_categoryDelegate.editing

                        onClicked: _private.removeCategory(_categoryDelegate.key, _categoryDelegate.pairCount)
                    }
                }

                footer: SctTextField {
                    id: _newCategoryField

                    property bool invalid: false

                    width: _categoriesView.width

                    color: invalid ? _private.invalidColor : Material.foreground
                    leftPadding: 12
                    placeholderText: text === "" ? "Add a category" : ""

                    font.pointSize: Runtime.notebookFontPointSize

                    onTextEdited: invalid = false
                    onReturnPressed: _private.addCategory(_newCategoryField)
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 200

            color: Color.translucent(Runtime.colors.primary.c100.background, 0.5)
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12

                spacing: 8

                ListView {
                    id: _pairsView

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                    ScrollBar.vertical: SctScrollBar { }

                    clip: true
                    model: _pairsModel
                    spacing: 2

                    footer: ColumnLayout {
                        width: _pairsView.width

                        spacing: 24

                        RowLayout {
                            id: _newPairRow

                            Layout.fillWidth: true

                            opacity: _newKeyField.text === "" && _newValueField.text === "" && !_newKeyField.activeFocus && !_newValueField.activeFocus ? 0.5 : 1
                            spacing: 8

                            SctTextField {
                                id: _newKeyField

                                property bool invalid: false

                                Layout.preferredWidth: root.keyColumnWidth

                                color: invalid ? _private.invalidColor : Material.foreground
                                placeholderText: text === "" ? "Key" : ""
                                tabItem: _newValueField

                                font.bold: true
                                font.pointSize: Runtime.notebookFontPointSize

                                onTextEdited: invalid = false
                            }

                            SctTextField {
                                id: _newValueField

                                Layout.fillWidth: true

                                backTabItem: _newKeyField
                                placeholderText: text === "" ? "Value" : ""
                                tabItem: _newKeyField
                                wrapMode: Text.WordWrap

                                font.pointSize: Runtime.notebookFontPointSize

                                onReturnPressed: _private.addPair(_newKeyField, _newValueField)
                            }
                        }

                        SctLabel {
                            Layout.fillWidth: true

                            horizontalAlignment: Text.AlignHCenter
                            opacity: 0.6
                            text: "No attributes yet. Type a key and a value in the row above to add the first one."
                            visible: _pairsView.count === 0
                            wrapMode: Text.WordWrap

                            font.pointSize: Runtime.notebookFontPointSize
                        }
                    }
                }
            }
        }
    }

    // The pairs of the selected category: the children of its row in the store, so that only the
    // rows that the list shows have delegates.
    DelegateModel {
        id: _pairsModel

        model: root.kvStore
        rootIndex: root.kvStore.index(Math.max(0, _categoriesView.currentIndex), 0)

        delegate: RowLayout {
            id: _pair

            required property string key
            required property var value

            width: ListView.view.width

            spacing: 8

            HoverHandler {
                id: _hoverHandler
            }

            SctTextField {
                id: _keyField

                property bool invalid: false

                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: root.keyColumnWidth

                color: invalid ? _private.invalidColor : Material.foreground
                tabItem: _valueField
                text: _pair.key

                font.bold: true
                font.pointSize: Runtime.notebookFontPointSize
                background.visible: activeFocus

                onTextEdited: invalid = false
                onEditingFinished: {
                    if(text !== _pair.key)
                        invalid = !root.kvStore.renameKey(root.category, _pair.key, text)
                }
            }

            SctTextField {
                id: _valueField

                // True if the value was not text when the user began to edit it. What is typed is
                // then read as JSON, so that a number stays a number.
                property bool typed: false

                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true

                backTabItem: _keyField
                wrapMode: Text.WordWrap

                font.pointSize: Runtime.notebookFontPointSize
                background.visible: activeFocus

                onActiveFocusChanged: {
                    if(activeFocus)
                        typed = _private.isTyped(_pair.value)
                }

                onTextEdited: root.kvStore.setValue(root.category, _pair.key, typed ? _private.valueFrom(text) : text)
            }

            // Not while the user is typing, so that what is typed is not rewritten.
            Binding {
                target: _valueField
                property: "text"
                value: _private.textOf(_pair.value)
                when: !_valueField.activeFocus
                restoreMode: Binding.RestoreNone
            }

            FlatToolButton {
                Layout.alignment: Qt.AlignTop

                suggestedWidth: 28

                iconSource: Runtime.themedIcon("qrc:/icons/action/close.png")
                opacity: _hoverHandler.hovered ? 1 : 0
                toolTipText: "Remove this pair"

                onClicked: root.kvStore.removeValue(root.category, _pair.key)
            }
        }
    }

    Component.onCompleted: _private.restoreCategory()

    onKvStoreChanged: Qt.callLater(_private.restoreCategory)

    QtObject {
        id: _private

        readonly property color invalidColor: Qt.rgba(0.8, 0.1, 0.1, 1)

        // What the tab is about, said for what the attributes are of.
        readonly property string description: {
            let subject = "this"
            switch(root.notes.ownerType) {
            case Notes.SceneOwner: subject = "this scene"; break
            case Notes.CharacterOwner: subject = "this character"; break
            case Notes.StructureOwner: subject = "the story"; break
            case Notes.BreakOwner: subject = "this episode or act"; break
            }

            return "Details about " + subject + " that you want to track, such as lighting or shot division. "
                    + "Add a category for each kind of detail, and a key and a value for each one."
        }

        // A value is text, or a number, a boolean, a list or an object. The text of a value is
        // the value itself if it is text, and its JSON otherwise.
        function isTyped(value) {
            return value !== null && value !== undefined && typeof value !== "string"
        }

        function textOf(value) {
            if(value === null || value === undefined)
                return ""

            return typeof value === "string" ? value : JSON.stringify(value)
        }

        // The value that text is: what it says if that is a number, a boolean, a list or an object
        // in JSON, and the text itself otherwise, so that 35 is a number, "35" and 35mm are text.
        function valueFrom(text) {
            try {
                const parsed = JSON.parse(text.trim())
                return parsed === null ? text : parsed
            } catch(e) {
                return text
            }
        }

        // Adds the pair that is typed in the fields, to the selected category.
        function addPair(keyField, valueField) {
            const key = keyField.text.trim()
            if(key === "")
                return

            if(root.kvStore.hasKey(root.category, key)) {
                keyField.invalid = true
                return
            }

            root.kvStore.setValue(root.category, key, valueFrom(valueField.text))
            keyField.text = ""
            valueField.text = ""
            keyField.forceActiveFocus()
        }

        // Adds the category that is typed in the field, and selects it.
        function addCategory(field) {
            if(field.text.trim() === "")
                return

            const name = field.text.trim()
            field.invalid = !root.kvStore.addCategory(name)
            if(field.invalid)
                return

            field.text = ""
            Qt.callLater(() => { select(_categoriesView.count - 1, name) })
        }

        // Selects a category of the list, which is what the Attributes tab then selects in the notes
        // of everything else that has a category of that name.
        function select(index, name) {
            _categoriesView.currentIndex = index
            Runtime.notebookSettings.attributesCategory = name
        }

        // Selects the category that was selected last, if the notes shown have it, and Root otherwise.
        function restoreCategory() {
            const name = Runtime.notebookSettings.attributesCategory.toLowerCase()
            const index = root.kvStore.categoryNames().findIndex(item => item.toLowerCase() === name)
            _categoriesView.currentIndex = Math.max(0, index)
        }

        // Renames a category, which is remembered by its new name if it was the one selected last.
        function rename(name, newName) {
            if(!root.kvStore.renameCategory(name, newName))
                return

            if(Runtime.notebookSettings.attributesCategory.toLowerCase() === name.toLowerCase())
                Runtime.notebookSettings.attributesCategory = newName.trim()
        }

        function removeCategory(category, pairCount) {
            if(pairCount === 0) {
                root.kvStore.removeCategory(category)
                return
            }

            MessageBox.question("Remove Category",
                                "Remove the category <b>" + category + "</b> and its " + pairCount + " pair(s)?",
                                ["Remove", "Cancel"], (answer) => {
                                    if(answer === "Remove")
                                        root.kvStore.removeCategory(category)
                                })
        }
    }
}
