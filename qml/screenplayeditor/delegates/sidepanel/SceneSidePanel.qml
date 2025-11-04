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

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/"

AbstractScenePartEditor {
    id: root

    required property ListView listView

    property alias label: _sidePanel.label
    property alias maxPanelWidth: _sidePanel.maxPanelWidth

    property alias expanded: _sidePanel.expanded

    readonly property alias currentTab: _private.currentTab

    width: _sidePanel.width

    SidePanel {
        id: _sidePanel

        height: parent.height

        clip: true
        buttonColor: Qt.tint(scene.color, expanded ? Runtime.colors.highlightedSceneControlTint : Runtime.colors.sceneControlTint)
        borderColor: Runtime.colors.primary.c400.background
        borderWidth: 1
        backgroundColor: buttonColor

        expanded: Runtime.screenplayEditorSettings.sceneSidePanelOpen
        cornerComponent: expanded ? _private.expandedCorner : _private.collapsedCorner

        content: TrapeziumTabView {
            tabColor: _private.indicatorColor
            tabBarVisible: false
            currentTabIndex: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab
            currentTabContent: _private.tabComponentsArray[currentTabIndex % _private.tabComponentsArray.length]
        }

        onExpandedChanged: Runtime.screenplayEditorSettings.sceneSidePanelOpen = expanded
    }

    QtObject {
        id: _private

        property int currentTab: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab

        property var tabComponentsArray: [_private.commentsTab,_private.featuredImageTab,_private.indexCardFieldsTab,_private.sceneMetaDataTab]

        property color indicatorColor: Color.isLight(root.scene.color) ? Runtime.colors.primary.c500.background : root.scene.color

        readonly property Component collapsedCorner: CollapsedCorner {
            scene: root.scene
        }

        readonly property Component expandedCorner: ExpandedCorner {
            scene: root.scene
            currentTab: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab
            downIndicatorColor: _private.indicatorColor

            onCurrentTabChanged: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab = currentTab
        }

        readonly property Component commentsTab: CommentsTab {
            index: root.index
            sceneID: root.sceneID
            screenplayElement: root.screenplayElement
            screenplayElementDelegateHasFocus: root.hasFocus

            partName: "CommentsTab"
            zoomLevel: root.zoomLevel
            fontMetrics: root.fontMetrics
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
        }

        readonly property Component featuredImageTab: FeaturedImageTab {
            index: root.index
            sceneID: root.sceneID
            screenplayElement: root.screenplayElement
            screenplayElementDelegateHasFocus: root.hasFocus

            partName: "FeaturedImageTab"
            zoomLevel: root.zoomLevel
            fontMetrics: root.fontMetrics
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            mipmap: !root.listView.moving

            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
        }

        readonly property Component indexCardFieldsTab: IndexCardFieldsTab {
            index: root.index
            sceneID: root.sceneID
            screenplayElement: root.screenplayElement
            screenplayElementDelegateHasFocus: root.hasFocus

            partName: "IndexCardFieldsTab"
            zoomLevel: root.zoomLevel
            fontMetrics: root.fontMetrics
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
        }

        readonly property Component sceneMetaDataTab: SceneMetaDataTab {
            index: root.index
            sceneID: root.sceneID
            screenplayElement: root.screenplayElement
            screenplayElementDelegateHasFocus: root.hasFocus

            partName: "SceneMetaDataTab"
            zoomLevel: root.zoomLevel
            fontMetrics: root.fontMetrics
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
        }
    }
}
