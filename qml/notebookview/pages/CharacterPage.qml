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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/notebookview/tabs"

AbstractNotebookPage {
    id: root

    property alias currentTab: _tabBar.currentTab

    signal deleteNoteRequest(Note note)
    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    Rectangle {
        id: _background

        anchors.fill: parent

        color: Runtime.colors.primary.c100.background
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        TextTabBar {
            id: _tabBar

            Layout.fillWidth: true

            name: "Characters"
            tabs: ["Information", "Relationships", "Notes"]
            currentTab: 0
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: _tabBar.currentTab

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: CharacterInformationTab {
                    character: _private.character
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: CharacterRelationshipsTab {
                    character: _private.character
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: NotesTab {
                    notes: _private.notes

                    onSwitchRequest: (item) => { root.switchRequest(item) }
                    onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
                }
            }
        }
    }

    QtObject {
        id: _private

        property Notes notes: character ? character.notes : null
        property Character character: root.pageData ? root.pageData.notebookItemObject.character : null
    }
}
