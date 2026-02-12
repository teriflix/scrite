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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch(character) { return doLaunch({"character": character}) }

    name: "AddRelationshipDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
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
                        reuseItems: false

                        ScrollBar.vertical: VclScrollBar { }

                        highlight: Item { }
                        highlightFollowsCurrentItem: true
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        model: dialog.character.unrelatedCharacterNames()
                        delegate: Rectangle {
                            id: characterRowItem

                            required property int index
                            required property string modelData

                            property string thisCharacterName: SMath.titleCased(character.name)
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
                                    text: SMath.titleCased(otherCharacterName) + "."
                                    color: foregroundColor
                                }
                            }
                        }

                        function initializeCacheBuffer() {
                            let firstDelegate = itemAtIndex(0)
                            const heightEstimate = firstDelegate ? Math.ceil(firstDelegate.height*1.1) : 75
                            cacheBuffer = heightEstimate * count
                        }

                        Component.onCompleted: Qt.callLater(initializeCacheBuffer)
                    }
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Create Relationships"

                    onClicked: createRelationshipsJob.start()

                    ActionHandler {
                        action: dialog.acceptAction

                        onTriggered: parent.clicked()
                    }
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
                        let nrRelationshipsAdded = 0
                        for(let i=0; i<charactersListView.count; i++) {
                            charactersListView.positionViewAtIndex(i, ListView.Visible)
                            let item = charactersListView.itemAtIndex(i)
                            if(item.checked) {
                                let otherCharacter = Scrite.document.structure.addCharacter(item.otherCharacterName)
                                if(otherCharacter) {
                                    const rel = character.addRelationship(item.relationship, otherCharacter)
                                    if(rel)
                                        ++nrRelationshipsAdded
                                }
                            }
                        }

                        if(nrRelationshipsAdded > 0)
                            character.characterRelationshipGraph = {}

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
