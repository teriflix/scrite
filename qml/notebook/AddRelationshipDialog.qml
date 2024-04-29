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
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(character) {
        var dlg = dialogComponent.createObject(root, {"character": character})
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch AddRelationshipDialog")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            property Character character

            width: 750
            height: 600
            title: "Add Relationship"

            contentItem: Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    SearchBar {
                        id: searchBar

                        Layout.fillWidth: true

                        searchEngine.objectName: "Characters Search Engine"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        border.color: Runtime.colors.primary.borderColor
                        color: Runtime.colors.primary.c100.background

                        TabSequenceManager {
                            id: characterListTabManager
                        }

                        ListView {
                            id: charactersListView
                            anchors.fill: parent
                            anchors.margins: 1
                            anchors.leftMargin: 5

                            clip: true
                            cacheBuffer: Number.MAX_SAFE_INTEGER

                            ScrollBar.vertical: VclScrollBar { }

                            highlight: Item { }
                            highlightFollowsCurrentItem: true
                            highlightMoveDuration: 0
                            highlightResizeDuration: 0

                            model: dialog.character.unrelatedCharacterNames()
                            delegate: Rectangle {
                                id: characterRowItem

                                required property string modelData
                                required property int index

                                property string thisCharacterName: Scrite.app.camelCased(character.name)
                                property string otherCharacterName: modelData
                                property bool   checked: relationshipName.length > 0
                                property string relationship: relationshipName.text

                                property bool  highlight: false
                                property color backgroundColor: highlight ? Runtime.colors.accent.c100.background : Runtime.colors.primary.c10.background
                                property color foregroundColor: highlight ? Runtime.colors.accent.c100.text : Runtime.colors.primary.c10.text

                                width: charactersListView.width
                                height: characterRow.height*1.15
                                color: backgroundColor

                                SearchAgent.engine: searchBar.searchEngine
                                SearchAgent.onSearchRequest: {
                                    SearchAgent.searchResultCount = SearchAgent.indexesOf(string, otherCharacterName).length > 0 ? 1 : 0
                                }
                                SearchAgent.onCurrentSearchResultIndexChanged: {
                                    highlight = SearchAgent.currentSearchResultIndex >= 0
                                    charactersListView.currentIndex = index
                                }

                                RowLayout {
                                    id: characterRow
                                    width: parent.width-20
                                    spacing: 10

                                    Image {
                                        Layout.preferredWidth: 24
                                        Layout.preferredHeight: 24

                                        source: "qrc:/icons/navigation/check.png"
                                        opacity: relationshipName.length > 0 ? 1 : 0.05
                                    }

                                    VclLabel {
                                        text: thisCharacterName + ": "
                                        color: foregroundColor
                                    }

                                    VclTextField {
                                        id: relationshipName

                                        Layout.fillWidth: true

                                        Material.background: backgroundColor
                                        Material.foreground: foregroundColor

                                        TabSequenceItem.manager: characterListTabManager
                                        TabSequenceItem.sequence: index

                                        label: ""
                                        color: foregroundColor
                                        maximumLength: 50
                                        placeholderText: "husband of, wife of, friends with, reports to ..."
                                        enableTransliteration: true

                                        onActiveFocusChanged: {
                                            if(activeFocus)
                                                charactersListView.currentIndex = index
                                        }
                                    }

                                    VclLabel {
                                        text: Scrite.app.camelCased(otherCharacterName) + "."
                                        color: foregroundColor
                                    }
                                }
                            }
                        }
                    }

                    VclButton {
                        Layout.alignment: Qt.AlignRight
                        text: "Create Relationships"
                        onClicked: createRelationshipsJob.start()
                    }
                }

                SequentialAnimation {
                    id: createRelationshipsJob

                    running: false

                    ScriptAction {
                        script: {
                            _private.waitDialog = WaitDialog.launch("Creating relationships ...")
                        }
                    }

                    PauseAnimation {
                        duration: 200
                    }

                    ScriptAction {
                        script: {
                            for(var i=0; i<charactersListView.count; i++) {
                                var item = charactersListView.itemAtIndex(i)
                                if(item.checked) {
                                    var otherCharacter = Scrite.document.structure.addCharacter(item.otherCharacterName)
                                    if(otherCharacter) {
                                        character.addRelationship(item.relationship, otherCharacter)
                                        character.characterRelationshipGraph = {}
                                    }
                                }
                            }

                            _private.waitDialog.close()
                            _private.waitDialog = null

                            Qt.callLater(dialog.close)
                        }
                    }
                }

                QtObject {
                    id: _private

                    property VclDialog waitDialog
                }
            }
        }
    }
}
