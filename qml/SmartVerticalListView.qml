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

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

Flickable {
    id: listViewFlick

    property alias spacing: listViewContentLayout.spacing
    property alias header: headerLoader.delegate
    property alias footer: footerLoader.delegate
    property alias model: listViewContentRepeater.model
    property Component delegate
    property var rowHeightEvaluator: function(row) { return 0; }

    contentWidth: listViewContent.width
    contentHeight: listViewContent.height
    property bool requiresScrolling: contentHeight > height
    ScrollBar.vertical: ScrollBar {
        policy: listViewFlick.requiresScrolling ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        minimumSize: 0.1
    }

    property int firstVisibleRow: -1
    property int lastVisibleRow: -1
    onContentYChanged: Qt.callLater(updateFirstAndLastVisibleRow)

    function updateFirstAndLastVisibleRow() {
        updateFirstAndLastVisibleRowTimer.start()
    }

    function itemAt(x, y) {
        var item = listViewContentLayout.childAt(x, y)
        if(item === null)
            return -1
        return indexOf(item) >= 0 ? item : null
    }

    function indexAt(x, y) {
        var item = listViewContentLayout.childAt(x, y)
        if(item === null)
            return -1
        return indexOf(item)
    }

    function itemAtIndex(index) {
        return listViewContentRepeater.itemAt(index)
    }

    function positionViewAtBeginning() {
        contentY = 0
    }

    function positionViewAtEnd() {
        contentY = contentHeight-height
    }

    function positionViewAtIndex(index, mode) {
        var item = listViewContentRepeater.itemAt(index)
        if(item === null)
            return

        var newContentY = contentY
        switch(mode) {
        case ListView.Beginning:
            newContentY = item.y
            break
        case ListView.Center:
            newContentY = item.y - height/2
            break
        case ListView.End:
            newContentY = item.y + item.height - height
            break
        case ListView.Contain:
            newContentY = item.y
            break
        }

        contentY = newContentY
    }

    function indexOf(item) {
        if(item === headerLoader.itemAt(0) || item === footerLoader.itemAt(0))
            return -1
        for(var i=0; i<listViewContentRepeater.count; i++) {
            var loader = listViewContentRepeater.itemAt(i)
            if(loader === item || loader.item === item)
                return i
        }
        return -1;
    }

    Timer {
        id: updateFirstAndLastVisibleRowTimer
        interval: 10
        repeat: false
        onTriggered: {
            if(listViewContentRepeater.count == 0) {
                firstVisibleRow = -1
                lastVisibleRow = -1
                return
            }

            var y1 = Math.round(mapToItem(listViewContentLayout, 1, 0).y)
            var y2 = Math.round(mapToItem(listViewContentLayout, 1, height-1).y)
            if(y1 > y2) {
                firstVisibleRow = -1
                lastVisibleRow = -1
                Qt.callLater(updateFirstAndLastVisibleRow)
                return
            }

            var y = y1;
            var item = null
            while(y < y2) {
                item = listViewContentLayout.childAt(1, y)
                if(item === null || item === headerLoader.itemAt(0)) {
                    y = y+1
                    continue
                }
                firstVisibleRow = indexOf(item)
                break
            }

            y = y2;
            while(y > y1) {
                item = listViewContentLayout.childAt(1, y)
                if(item === null || item === footerLoader.itemAt(0)) {
                    y = y-1
                    continue
                }
                lastVisibleRow = indexOf(item)
                break
            }
        }
    }

    Item {
        id: listViewContent
        width: listViewFlick.width - (listViewFlick.requiresScrolling ? listViewFlick.ScrollBar.vertical.width : 0)
        height: listViewContentLayout.height

        Column {
            id: listViewContentLayout
            width: parent.width

            Repeater {
                id: headerLoader
                model: 1
                onCountChanged: Qt.callLater(updateFirstAndLastVisibleRow)
            }

            Repeater {
                id: listViewContentRepeater
                onCountChanged: Qt.callLater(updateFirstAndLastVisibleRow)
                delegate: SmartLoader {
                    id: delegateLoader
                    property int row: index
                    property var rowData: modelData
                    width: parent.width
                    height: rowHeightEvaluator(row)
                    sourceComponent: delegate
                    active: delegateLoader.row >= listViewFlick.firstVisibleRow && delegateLoader.row <=listViewFlick.lastVisibleRow
                }
            }

            Repeater {
                id: footerLoader
                model: 1
                onCountChanged: Qt.callLater(updateFirstAndLastVisibleRow)
            }
        }
    }
}

