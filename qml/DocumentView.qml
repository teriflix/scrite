/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import Scrite 1.0

Flickable {
    id: documentView
    flickableDirection: Flickable.VerticalFlick
    boundsBehavior: Flickable.StopAtBounds
    boundsMovement: Flickable.StopAtBounds
    contentWidth: documentContent.width
    contentHeight: documentContent.height

    property alias model: repeater.model
    property alias spacing: documentContent.spacing
    property alias header: headerLoader.sourceComponent
    property alias footer: footerLoader.sourceComponent
    property alias count: repeater.count

    property Component sizeHintDelegate
    property Component itemDelegate

    function itemAt(index) {
        return repeater.itemAt(index)
    }

    property int currentIndex: -1

    Column {
        id: documentContent
        width: documentView.width
        property rect viewportRectForVisibilityConsideration: Qt.rect(0, contentY-height/2, width, height*2)

        Loader {
            id: headerLoader
            width: parent.width
        }

        Repeater {
            id: repeater

            Item {
                id: repeaterDelegate
                width: documentContent.width
                height: Math.max(sizeHintLoader.height, itemLoader.height)
                property rect delegateRect: Qt.rect(0, y, width, height)

                property alias sizeHint: sizeHintLoader.item
                property alias item: itemLoader.item

                Loader {
                    id: sizeHintLoader
                    width: parent.width
                    readonly property int itemIndex: rowNumber
                    readonly property var itemData: modelData
                    sourceComponent: sizeHintDelegate
                }

                Loader {
                    id: itemLoader
                    width: parent.width
                    readonly property int itemIndex: rowNumber
                    readonly property var itemData: modelData
                    sourceComponent: itemDelegate
                    active: visibleToUser.get
                }

                DelayedPropertyBinder {
                    id: visibleToUser
                    initial: false
                    set: app.doRectanglesIntersect(repeaterDelegate.delegateRect, documentContent.viewportRectForVisibilityConsideration)
                }
            }
        }

        Loader {
            id: footerLoader
            width: parent.width
        }
    }

    onCurrentIndexChanged: {
        // TODO
    }

    function positionViewAtIndex2(index, mode) {
        // TODO
    }

    function ensureVisible(item, rect) {
        if(item === null)
            return

        var pt = item.mapToItem(documentContent, rect.x, rect.y)
        var startY = documentView.contentY
        var endY = documentView.contentY + documentView.height - rect.height
        if( startY < pt.y && pt.y < endY )
            return

        if( pt.y < startY )
            documentView.contentY = pt.y
        else if( pt.y > endY )
            documentView.contentY = (pt.y + 2*rect.height) - documentView.height
    }
}
