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
        buttonColor: _private.indicatorColor
        borderColor: buttonColor
        borderWidth: 0
        backgroundColor: buttonColor

        expanded: Runtime.screenplayEditorSettings.sceneSidePanelOpen
        cornerComponent: expanded ? _private.expandedCorner : _private.collapsedCorner

        content: TrapeziumTabView {
            tabColor: _private.indicatorColor
            tabBarVisible: false
            currentTabIndex: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab
            currentTabContent: _private.tabComponentsArray[currentTabIndex % _private.tabComponentsArray.length]
            tabContentBorderVisible: false
        }

        onExpandedChanged: Runtime.screenplayEditorSettings.sceneSidePanelOpen = expanded
    }

    ActionHandler {
        action: ActionHub.editOptions.find("toggleCommentsPanel")

        enabled: root.isCurrent
        checked: Runtime.screenplayEditorSettings.sceneSidePanelOpen
        onTriggered: (source) => {
                        Runtime.screenplayEditorSettings.sceneSidePanelOpen = !Runtime.screenplayEditorSettings.sceneSidePanelOpen
                     }
    }

    QtObject {
        id: _private

        property int currentTab: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab

        property var tabComponentsArray: [_private.commentsTab,_private.featuredImageTab,_private.indexCardFieldsTab,_private.sceneMetaDataTab]

        property color indicatorColor: {
            const ideally = Runtime.colors.tint(scene.color, root.isCurrent ? Runtime.colors.selectedSceneHeadingTint : Runtime.colors.sceneControlTint)
            return Color.isLight(scene.color) ? (root.isCurrent ? Runtime.colors.primary.c200.background : Runtime.colors.primary.c50.background) : ideally
        }

        readonly property Component collapsedCorner: CollapsedCorner {
            scene: root.scene

            ActionHandler {
                action: ActionHub.editOptions.find("cycleCommentsPanelTab")

                enabled: root.isCurrent
                onTriggered: (source) => { Runtime.screenplayEditorSettings.sceneSidePanelOpen = true }
            }
        }

        readonly property Component expandedCorner: ExpandedCorner {
            scene: root.scene
            currentTab: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab
            downIndicatorColor: Qt.rgba(0,0,0,0.5)

            onCurrentTabChanged: Runtime.screenplayEditorSettings.sceneSidePanelActiveTab = currentTab

            ActionHandler {
                action: ActionHub.editOptions.find("cycleCommentsPanelTab")

                enabled: root.isCurrent
                onTriggered: (source) => { parent.cycleTab() }
            }
        }

        readonly property Component commentsTab: CommentsTab {
            index: root.index
            sceneID: root.sceneID
            screenplayElement: root.screenplayElement
            screenplayElementDelegateHasFocus: root.hasFocus

            partName: "CommentsTab"
            isCurrent: root.isCurrent
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
            isCurrent: root.isCurrent
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
            isCurrent: root.isCurrent
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
            isCurrent: root.isCurrent
            zoomLevel: root.zoomLevel
            fontMetrics: root.fontMetrics
            pageMargins: root.pageMargins
            screenplayAdapter: root.screenplayAdapter

            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
        }
    }
}
