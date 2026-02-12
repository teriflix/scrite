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

import QtQuick 2.15

import io.scrite.components 1.0

import "qrc:/qml/helpers"

Item {
    id: root

    property var items: []

    property bool hasItems: items.length > 0
    property bool canLayout: items.length >= 2

    property alias active: _tightRect.visible
    property alias contextMenu: _selectionMenu.menu

    readonly property alias rect: _private.rect

    signal moveItem(Item item, real dx, real dy)
    signal placeItem(Item item)

    function createBounds() {
        let bounds = {
            "p1": { x: -1, y: -1 },
            "p2": { x: -1, y: -1 },
            "unite": function(pt) {
                if(this.p1.x < 0 || this.p1.y < 0) {
                    this.p1.x = pt.x
                    this.p1.y = pt.y
                } else {
                    this.p1.x = Math.min(this.p1.x, pt.x)
                    this.p1.y = Math.min(this.p1.y, pt.y)
                }
                if(this.p2.x < 0 || this.p2.y < 0) {
                    this.p2.x = pt.x
                    this.p2.y = pt.y
                } else {
                    this.p2.x = Math.max(this.p2.x, pt.x)
                    this.p2.y = Math.max(this.p2.y, pt.y)
                }

                this.p1.x = Math.round(this.p1.x)
                this.p2.x = Math.round(this.p2.x)
                this.p1.y = Math.round(this.p1.y)
                this.p2.y = Math.round(this.p2.y)
            }
        }

        return bounds
    }

    // For intializing from a repeater and a boundary
    // Here boundary is a JSON like {left: x, top: y, right: r, bottom: b }
    function init(repeater, boundary, includeInvisibleItems) {
        if(repeater === undefined || repeater === null)
            return

        if(includeInvisibleItems === undefined)
            includeInvisibleItems = false

        clear()

        let bounds = createBounds()
        let selectedItems = []
        let count = repeater.count
        for(let i=0; i<count; i++) {
            let item = repeater.itemAt(i)
            if(!item.visible && !includeInvisibleItems)
                continue
            let p1 = Qt.point(item.x, item.y)
            let p2 = Qt.point(item.x+item.width, item.y+item.height)
            let areaContainsPoint = function(p) {
                return boundary.left <= p.x && p.x <= boundary.right &&
                        boundary.top <= p.y && p.y <= boundary.bottom;
            }
            if(areaContainsPoint(p1) || areaContainsPoint(p2)) {
                bounds.unite(p1)
                bounds.unite(p2)
                selectedItems.push(item)
            }
        }

        _tightRect.x = bounds.p1.x - 10
        _tightRect.y = bounds.p1.y - 10
        _tightRect.width = (bounds.p2.x-bounds.p1.x+20)
        _tightRect.height = (bounds.p2.y-bounds.p1.y+20)
        _tightRect.topLeft = Qt.point(_tightRect.x, _tightRect.y)

        selectedItems.sort( function(i1, i2) {
            return (i1.x === i2.x) ? i1.y - i2.y : i1.x - i2.x
        })
        items = selectedItems
    }

    // For initializing from an array of items
    function set(arrayOfItems) {
        if(arrayOfItems === null)
            return

        clear()

        let selectedItems = []
        let bounds = createBounds()
        for(let i=0; i<arrayOfItems.length; i++) {
            let item = arrayOfItems[i]
            let p1 = Qt.point(item.x, item.y)
            let p2 = Qt.point(item.x+item.width, item.y+item.height)
            bounds.unite(p1)
            bounds.unite(p2)
            selectedItems.push(item)
        }

        _tightRect.x = bounds.p1.x - 10
        _tightRect.y = bounds.p1.y - 10
        _tightRect.width = (bounds.p2.x-bounds.p1.x+20)
        _tightRect.height = (bounds.p2.y-bounds.p1.y+20)
        _tightRect.topLeft = Qt.point(_tightRect.x, _tightRect.y)

        selectedItems.sort( function(i1, i2) {
            return (i1.x === i2.x) ? i1.y - i2.y : i1.x - i2.x
        })
        items = selectedItems
    }

    function refit() {
        let count = items.length
        if(count === 0)
            return
        let bounds = createBounds()
        for(let i=0; i<count; i++) {
            let item = items[i]
            let p1 = Qt.point(item.x, item.y)
            let p2 = Qt.point(item.x+item.width, item.y+item.height)
            bounds.unite(p1)
            bounds.unite(p2)
        }

        _tightRect.x = bounds.p1.x - 10
        _tightRect.y = bounds.p1.y - 10
        _tightRect.width = (bounds.p2.x-bounds.p1.x+20)
        _tightRect.height = (bounds.p2.y-bounds.p1.y+20)
        _tightRect.topLeft = Qt.point(_tightRect.x, _tightRect.y)
    }

    function clear() {
        if(interactive) {
            let elements = items
            elements.forEach( function(element) {
                placeItem(element);
            })
        }
        items = []
    }

    MouseArea {
        anchors.fill: parent

        enabled: _tightRect.visible
        acceptedButtons: Qt.LeftButton

        onPressed: {
            root.clear()
            mouse.accepted = false
        }
    }

    Rectangle {
        id: _tightRect

        property point topLeft: Qt.point(0,0)

        color: Color.translucent(Scrite.app.palette.highlight,0.2)
        border { width: 2; color: Scrite.app.palette.highlight }
        visible: parent.items.length > 0

        MouseArea {
            anchors.fill: parent

            drag.target: root.interactive ? parent : null
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.minimumY: 0

            enabled: parent.visible
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: {
                if(_selectionMenu.menu && mouse.button === Qt.RightButton)
                    _selectionMenu.popup()
            }
        }

        function shiftElements() {
            let elements = root.items
            let i, item
            let dx = x - topLeft.x
            let dy = y - topLeft.y
            topLeft = Qt.point(x,y)
            for(i=0; i<elements.length; i++)
                root.moveItem(elements[i], dx, dy)
            Scrite.document.structure.forceBeatBoardLayout = false
        }

        MenuLoader {
            id: _selectionMenu

            anchors.fill: parent
        }

        onXChanged: if(root.items.length > 0) shiftElements()
        onYChanged: if(root.items.length > 0) shiftElements()
    }

    QtObject {
        id: _private

        property rect rect: Qt.rect(_tightRect.x, _tightRect.y, _tightRect.width, _tightRect.height)
    }
}
