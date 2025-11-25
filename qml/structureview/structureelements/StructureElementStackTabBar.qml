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

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

Item {
    id: root

    required property StructureElementStack elementStack

    required property rect canvasScrollViewportRect

    required property Item canvasScrollViewport
    required property BoundingBoxEvaluator canvasItemsBoundingBox

    BoundingBoxItem.evaluator: canvasItemsBoundingBox
    BoundingBoxItem.livePreview: false
    BoundingBoxItem.viewportItem: canvasScrollViewport
    BoundingBoxItem.viewportRect: canvasScrollViewportRect
    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0)
    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0)

    x: _private.geometry.x
    y: _private.geometry.y
    width: _private.geometry.width
    height: _private.geometry.height

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.top
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.bottomMargin: -1.5

        active: root.elementStack.objectCount > 1

        sourceComponent: Flickable {
            id: _tabBarItemFlick

            clip: interactive
            height: contentHeight
            interactive: contentWidth > width
            contentWidth: _tabBarItem.width
            contentHeight: _tabBarItem.height

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            SimpleTabBarItem {
                id: _tabBarItem

                tabCount: root.elementStack.objectCount
                tabLabelStyle: SimpleTabBarItem.Alphabets
                activeTabBorderWidth: (root.elementStack.hasCurrentElement ? 2 : 1)

                activeTabIndex: root.elementStack.topmostElementIndex
                activeTabColor: Runtime.colors.tint(root.elementStack.topmostElement.scene.color, (root.elementStack.hasCurrentElement ? Runtime.colors.sceneControlTint : Runtime.colors.selectedSceneControlTint))
                activeTabFont.bold: true
                activeTabTextColor: Color.textColorFor(activeTabColor)
                activeTabBorderColor: Color.isLight(root.elementStack.topmostElement.scene.color) ? "black" : root.elementStack.topmostElement.scene.color
                activeTabFont.pointSize: Runtime.idealFontMetrics.font.pointSize

                inactiveTabFont.pointSize: Runtime.idealFontMetrics.font.pointSize-4
                inactiveTabTextColor: Color.translucent(Color.textColorFor(inactiveTabColor), 0.75)

                minimumTabWidth: root.width*0.1

                onTabClicked: root.elementStack.bringElementToTop(index)

                onActiveTabIndexChanged: Qt.callLater(ensureActiveTabIsVisible)

                onTabPathsUpdated: Qt.callLater(ensureActiveTabIsVisible)

                Connections {
                    target: root.elementStack
                    ignoreUnknownSignals: true

                    function onDataChanged2() { _tabBarItem.updateTabAttributes() }

                    function onStackInitialized() { _tabBarItem.updateTabAttributes() } // ???
                }

                onAttributeRequest: {
                    if(index === activeTabIndex)
                        return

                    const element = root.elementStack.objectAt(index)
                    switch(attr) {
                    case SimpleTabBarItem.TabColor:
                        requestedAttributeValue = Runtime.colors.tint(element.scene.color, Runtime.colors.selectedSceneControlTint)
                        break
                    case SimpleTabBarItem.TabBorderColor:
                        requestedAttributeValue = Color.isLight(element.scene.color) ? Runtime.colors.primary.borderColor : element.scene.color
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

    QtObject {
        id: _private

        property rect geometry: root.elementStack.geometry
    }
}
