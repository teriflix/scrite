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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Layouts

import Scrite.App

import "../../helpers"
import "../tabs"

/**
  A notebook page whose content is in tabs. The page declares its own tabs as NotebookPageTab children.
  Extensions add theirs with a FeatureProvider whose hostUri is the hostUri of the page. All tabs,
  including the singleton notes of the notes of the page, are sorted by their order.

  The hostUri of a page is "Scrite.App.Notebook.<Page>.Tabs", and is unique to it.
  */
AbstractNotebookPage {
    id: root

    // Without tabs, the page shows just its first tab, and none of the other tabs, not even the ones
    // that extensions offer.
    property bool showTabs: true

    // False when the page has nothing to show tabs of, and shows something else in their place.
    property bool tabsVisible: true

    // The space to the left, right and bottom of the current tab, which the tab bar already has.
    property real contentMargin: 0

    property alias hostUri: _featureHost.uri

    property string tabBarName: "Tabs"

    // The uri of the current tab. Pages bind it to a setting, so as to remember it. If no tab has it,
    // the first tab is current.
    property string currentTabUri

    // The notes whose singleton notes get a tab each.
    property Notes notes

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.
    signal deleteNoteRequest(Note note)

    // Selects the tab of the singleton note. Returns false if this page does not have one for it.
    function showNote(note) {
        if(!root.showTabs || !note)
            return false

        const uri = _private.singletonTabUri(note)
        if(!_private.tabs.some( (tab) => tab.uri === uri ))
            return false

        root.currentTabUri = uri
        return true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: root.showTabs ? 11 : 0

        visible: root.tabsVisible

        TextTabBar {
            id: _tabBar

            Layout.leftMargin: 11
            Layout.rightMargin: 11
            Layout.fillWidth: true

            name: root.tabBarName
            tabs: _private.tabs.map( (tab) => tab.title )
            switchTabHandlerEnabled: root.showTabs && root.tabsVisible
            visible: root.showTabs

            // Only a tab that the user switched to is noted. The Binding below sets the others.
            onCurrentTabChanged: {
                const tab = _private.tabs[currentTab]
                if(tab && currentTab !== _private.indexOfTab(root.currentTabUri))
                    root.currentTabUri = tab.uri
            }
        }

        // Only the current tab is loaded. The others are made when they are switched to.
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: root.contentMargin
            Layout.rightMargin: root.contentMargin
            Layout.bottomMargin: root.contentMargin

            clip: true
            sourceComponent: root.tabsVisible && _private.tabs.length > 0 ? _private.tabs[_private.currentIndex].component : null
        }
    }

    // Keeps the tab bar on the tab with currentTabUri, even after it has been clicked.
    Binding {
        target: _tabBar
        property: "currentTab"
        value: _private.indexOfTab(root.currentTabUri)
        restoreMode: Binding.RestoreNone
    }

    // The tabs that extensions offer.
    FeatureHost {
        id: _featureHost

        onFeatureAdded: (feature) => { _private.addContributedTab(feature) }
        onFeatureRemoved: (feature) => { _private.removeContributedTab(feature) }
    }

    // A tab for each singleton note, after the others unless an extension orders its tabs higher.
    Instantiator {
        id: _singletonTabs

        model: root.showTabs && root.notes ? root.notes.singletonNotes : []

        delegate: NotebookPageTab {
            id: _singletonTab

            required property int index
            required property var modelData

            uri: _private.singletonTabUri(_singletonTab.modelData)
            title: _singletonTab.modelData.title
            order: 1000 + _singletonTab.index

            SingletonNoteTab {
                note: _singletonTab.modelData
                maxTextAreaSize: root.maxTextAreaSize
                minTextAreaSize: root.minTextAreaSize

                onDeleteRequest: (note) => { _private.deleteSingletonNote(note) }
            }
        }

        onObjectAdded: _private.collectSingletonTabs()
        onObjectRemoved: _private.collectSingletonTabs()
    }

    Component.onCompleted: {
        _private.collectBuiltinTabs()
        _private.ready = true
    }

    QtObject {
        id: _private

        // Without tabs, the first one is current, whatever the tab bar says.
        readonly property int currentIndex: root.showTabs ? Math.max(0, Math.min(_tabBar.currentTab, tabs.length - 1)) : 0

        readonly property var tabs: {
            if(!ready)
                return []

            if(!root.showTabs) {
                const first = sortTabs(builtinTabs)
                return first.length > 0 ? [first[0]] : []
            }

            return sortTabs(builtinTabs.concat(contributedTabs, singletonTabs))
        }

        // Until the tabs of the page are known, there are none, so that no tab is chosen early.
        property bool ready: false

        property var builtinTabs: []
        property var contributedTabs: []
        property var singletonTabs: []

        // -1 when there is no tab at all, and 0 when no tab has the uri.
        function indexOfTab(uri) {
            if(tabs.length === 0)
                return -1

            const index = tabs.findIndex( (tab) => tab.uri === uri )
            return index < 0 ? 0 : index
        }

        function sortTabs(list) {
            return list.map( (tab, index) => ({"tab": tab, "index": index}) )
                       .filter( (entry) => entry.tab.visible )
                       .sort( (a, b) => a.tab.order !== b.tab.order ? a.tab.order - b.tab.order : a.index - b.index )
                       .map( (entry) => entry.tab )
        }

        function collectBuiltinTabs() {
            const list = []
            for(let i = 0; i < root.data.length; i++) {
                const child = root.data[i]
                if(child instanceof NotebookPageTab)
                    list.push(child)
            }
            builtinTabs = list
        }

        function collectSingletonTabs() {
            const list = []
            for(let i = 0; i < _singletonTabs.count; i++)
                list.push(_singletonTabs.objectAt(i))
            singletonTabs = list
        }

        function addContributedTab(feature) {
            if(feature instanceof NotebookPageTab)
                contributedTabs = contributedTabs.concat([feature])
        }

        function removeContributedTab(feature) {
            contributedTabs = contributedTabs.filter( (tab) => tab !== feature )
        }

        function singletonTabUri(note) {
            return "Scrite.NotebookTab.SingletonNote." + note.id
        }

        function deleteSingletonNote(note) {
            root.askDeleteConfirmation("Are you sure you want to delete this note?", () => {
                                           Qt.callLater(() => { root.notes.removeNote(note) })
                                       })
        }
    }
}
