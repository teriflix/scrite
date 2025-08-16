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

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

Item {
    id: root

    required property StructureElementStack elementStack

    required property rect canvasScrollViewportRect

    required property BoundingBoxEvaluator canvasItemsBoundingBox

    BoundingBoxItem.evaluator: canvasItemsBoundingBox
    BoundingBoxItem.livePreview: false
    BoundingBoxItem.viewportItem: root
    BoundingBoxItem.viewportRect: canvasScrollViewportRect
    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0)
    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0)

    x: elementStack.geometry.x
    y: elementStack.geometry.y
    width: elementStack.geometry.width
    height: elementStack.geometry.height

    Flickable {
        id: _tabBarItemFlick

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.top
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.bottomMargin: -_tabBarItem.activeTabBorderWidth-0.5

        clip: interactive
        height: contentHeight
        interactive: contentWidth > width
        contentWidth: _tabBarItem.width
        contentHeight: _tabBarItem.height

        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        SimpleTabBarItem {
            id: _tabBarItem

            tabCount: elementStack.objectCount
            tabLabelStyle: SimpleTabBarItem.Alphabets
            activeTabBorderWidth: (elementStack.hasCurrentElement ? 2 : 1)

            activeTabIndex: elementStack.topmostElementIndex
            activeTabColor: Qt.tint(elementStack.topmostElement.scene.color, (elementStack.hasCurrentElement ? "#C0FFFFFF" : "#F0FFFFFF"))
            activeTabFont.bold: true
            activeTabTextColor: Scrite.app.textColorFor(activeTabColor)
            activeTabBorderColor: Scrite.app.isLightColor(elementStack.topmostElement.scene.color) ? "black" : elementStack.topmostElement.scene.color
            activeTabFont.pointSize: Runtime.idealFontMetrics.font.pointSize

            inactiveTabFont.pointSize: Runtime.idealFontMetrics.font.pointSize-4
            inactiveTabTextColor: Scrite.app.translucent(Scrite.app.textColorFor(inactiveTabColor), 0.75)

            minimumTabWidth: root.width*0.1

            onTabClicked: elementStack.bringElementToTop(index)

            onActiveTabIndexChanged: Qt.callLater(ensureActiveTabIsVisible)

            onTabPathsUpdated: Qt.callLater(ensureActiveTabIsVisible)

            Connections {
                target: elementStack
                ignoreUnknownSignals: true

                function onDataChanged2() { _tabBarItem.updateTabAttributes() }

                function onStackInitialized() { _tabBarItem.updateTabAttributes() } // ???
            }

            onAttributeRequest: {
                if(index === activeTabIndex)
                    return

                const element = elementStack.objectAt(index)
                switch(attr) {
                case SimpleTabBarItem.TabColor:
                    requestedAttributeValue = Qt.tint(element.scene.color, "#D0FFFFFF")
                    break
                case SimpleTabBarItem.TabBorderColor:
                    requestedAttributeValue = Scrite.app.isLightColor(element.scene.color) ? "gray" : element.scene.color
                    break
                default:
                    break
                }
            }

            function ensureActiveTabIsVisible() {
                if(activeTabIndex < 0) {
                    _tabBarItemFlick.contentX = 0
                    return
                }

                const r = tabRect(activeTabIndex)
                if(_tabBarItemFlick.contentX > r.x)
                    _tabBarItemFlick.contentX = r.x
                else if(_tabBarItemFlick.contentX + _tabBarItemFlick.width < r.x + r.width)
                    _tabBarItemFlick.contentX = r.x + r.width - _tabBarItemFlick.width
            }
        }
    }
}
