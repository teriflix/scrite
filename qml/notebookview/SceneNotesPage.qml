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

AbstractNotebookPage {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    property alias currentTab: _tabBar.currentTab

    signal deleteNoteRequest(Note note)
    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    Rectangle {
        id: _background

        anchors.fill: parent

        color: Qt.tint(_private.scene.color, "#e7ffffff")
    }

    TextTabBar {
        id: _tabBar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8

        name: root.pageData ? root.pageData.notebookItemTitle.substr(0, root.pageData.notebookItemTitle.indexOf(']')+1) : "Scene"
        tabs: ["Synopsis", "Relationships", "Notes", "Comments"]
        currentTab: 0
    }

    StackLayout {
        id: _contentArea

        anchors.top: _tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 10

        clip: true
        currentIndex: _tabBar.currentTab

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true

            active: visible

            source: SceneSynopsisTab {
                scene: _private.scene
                maxTextAreaSize: root.maxTextAreaSize
                minTextAreaSize: root.minTextAreaSize
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true

            active: visible

            sourceComponent: SceneCharacterRelationshipsTab {
                scene: _private.scene

                onSwitchRequest: (item) => { root.switchRequest(item) }
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

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true

            active: visible

            source: SceneCommentsTab {
                scene: _private.scene
                maxTextAreaSize: root.maxTextAreaSize
                minTextAreaSize: root.minTextAreaSize
            }
        }
    }

    QtObject {
        id: _private

        property Notes notes: root.pageData ? root.pageData.notebookItemObject : null
        property Scene scene: notes ? notes.scene : null
    }
}
