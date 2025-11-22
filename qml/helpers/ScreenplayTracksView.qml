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
import QtQuick.Shapes 1.5
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Flickable {
    id: root

    required property ListView listView
    required property Screenplay screenplay

    function reload() { _private.reload() }

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object,event,result) => {
        EventFilter.forwardEventTo(listView)
        result.filter = true
        result.accepted = true
    }

    implicitWidth: _private.model ? (listView.orientation === Qt.Vertical ? contentWidth : 0) : 0
    implicitHeight: _private.model ? (listView.orientation === Qt.Horizontal ? contentHeight : 0) : 0

    clip: true
    contentX: listView.orientation === Qt.Horizontal ? listView.contentX : 0
    contentY: listView.orientation === Qt.Vertical ? listView.contentY : 0
    interactive: false
    contentWidth: _content.width
    contentHeight: _content.height

    Item {
        id: _content

        width: listView.orientation === Qt.Horizontal ? listView.contentWidth : Runtime.screenplayTracks.trackCount * (Runtime.minimumFontMetrics.height + 10)
        height: listView.orientation === Qt.Horizontal ? Runtime.screenplayTracks.trackCount * (Runtime.minimumFontMetrics.height + 10) : listView.contentHeight

        Repeater {
            model: _private.model

            Rectangle {
                id: _track

                required property int index
                required property var track // of type ScreenplayTrack, a Q_GADGET declared in screenplay.h
                                            // struct ScreenplayTrack { QString name; QList<ScreenplayTrackItem> items; }

                property var items: track.items
                property bool keywordsTrack: track.name === ""
                property bool stackTrack: track.name === _private.model.stackTrackName
                property string name: track.name

                property real offset: index * (Runtime.minimumFontMetrics.height + 10)

                x: listView.orientation === Qt.Vertical ? offset : 0
                y: listView.orientation === Qt.Horizontal ? offset : 0

                width: listView.orientation === Qt.Horizontal ? _content.width : Runtime.minimumFontMetrics.lineSpacing + 8
                height: listView.orientation === Qt.Horizontal ? Runtime.minimumFontMetrics.lineSpacing + 8 : _content.height

                color: Color.translucent( border.color, 0.1 )
                border.color: track.color
                border.width: 0.5

                MouseArea {
                    id: _trackMouseArea

                    anchors.fill: parent

                    hoverEnabled: true

                    onMouseXChanged: maybeTooltip()
                    onMouseYChanged: maybeTooltip()
                    onContainsMouseChanged: maybeTooltip()

                    function maybeTooltip() {
                        if(containsMouse) {
                            _toolTip.set(mouseX, mouseY, _track.name, _trackMouseArea)
                        } else if(_toolTip.source === _trackMouseArea) {
                            _toolTip.unset(_trackMouseArea)
                        }
                    }
                }

                Repeater {
                    model: _track.items

                    Rectangle {
                        id: _trackItem

                        required property int index
                        required property var modelData // of type ScreenplayTrackItem, a Q_GADGET declared in screenplay.h
                                                        // struct ScreenplayTrackItem { int startIndex, endIndex; QString name; QColor color; }

                        property QtObject extents: QtObject {
                            property real from: {
                                if(_trackItem.startItem === null || _trackItem.endItem === null)
                                    return 0

                                const pos = listView.contentItem.mapFromItem(_trackItem.startItem, 0, 0)
                                return listView.orientation === Qt.Horizontal ? pos.x : pos.y
                            }
                            property real to: {
                                if(_trackItem.startItem === null || _trackItem.endItem === null)
                                    return 0

                                const pos = listView.contentItem.mapFromItem(_trackItem.endItem, 0, 0)
                                return (listView.orientation === Qt.Horizontal ? pos.x + _trackItem.endItem.width : pos.y + _trackItem.endItem.height) + _private.trackMargin
                            }
                        }

                        property int startIndex: root.screenplay.indexOfElement(root.screenplay.elementWithIndex(modelData.startIndex))
                        property int endIndex: modelData.startIndex === modelData.endIndex ? startIndex : root.screenplay.indexOfElement(root.screenplay.elementWithIndex(modelData.endIndex))

                        property Item startItem: listView.itemAtIndex(startIndex)
                        property Item endItem: listView.itemAtIndex(endIndex)

                        property string name: modelData.name

                        property rect itemRect: Qt.rect(x, y, width, height)

                        function lookupItems() {
                            if(!startItem)
                                startItem = listView.itemAtIndex(startIndex)
                            if(!endItem)
                                endItem = listView.itemAtIndex(endIndex)
                            if(!startItem || !endItem)
                                Qt.callLater(lookupItems)
                        }

                        Component.onCompleted: Qt.callLater(lookupItems)

                        x: listView.orientation === Qt.Horizontal ? extents.from : 2
                        y: listView.orientation === Qt.Vertical ? extents.from : 2

                        width: listView.orientation === Qt.Horizontal ? extents.to - extents.from : parent.width-4
                        height: listView.orientation === Qt.Horizontal ? parent.height-4 : extents.to - extents.from

                        color: modelData.color
                        visible: GMath.doRectanglesIntersect(itemRect, _private.viewportRect)

                        border.color: Color.translucent(Color.textColorFor(color), 0.25)
                        border.width: 0.5

                        VclLabel {
                            anchors.centerIn: parent

                            width: (listView.orientation === Qt.Vertical ? parent.height : parent.width) - 10

                            rotation: listView.orientation === Qt.Vertical ? -90 : 0
                            transformOrigin: Item.Center

                            font: Runtime.minimumFontMetrics.font
                            elide: Text.ElideMiddle
                            color: Color.textColorFor(parent.color)
                            horizontalAlignment: Text.AlignHCenter

                            text: _trackItem.name
                        }

                        MouseArea {
                            id: _trackItemMouseArea

                            anchors.fill: parent

                            hoverEnabled: true

                            onMouseXChanged: maybeTooltip()
                            onMouseYChanged: maybeTooltip()
                            onContainsMouseChanged: maybeTooltip()

                            function maybeTooltip() {
                                if(containsMouse) {
                                    _toolTip.set(mouseX + _trackItem.x, mouseY + _trackItem.y, toolTipText(), _trackItemMouseArea)
                                } else if(_toolTip.source === _trackItemMouseArea) {
                                    _toolTip.unset(_trackItemMouseArea)
                                }
                            }

                            function toolTipText() {
                                let ret = "<b>" + (_track.keywordsTrack ? "" : _track.name + " &gt; ") + _trackItem.name + "</b>, "
                                if(_trackItem.modelData.endIndex === _trackItem.modelData.startIndex)
                                    ret += "1 Scene"
                                else
                                    ret += (1 + _trackItem.modelData.endIndex - _trackItem.modelData.startIndex) + " Scenes"
                                if(!Runtime.paginator.paused && Runtime.paginator.screenplay === root.screenplay) {
                                    let from = root.screenplay.elementAt(_trackItem.startIndex)
                                    let to = root.screenplay.elementAt(_trackItem.endIndex)
                                    ret += ", Duration: " + TMath.timeLengthString(Runtime.paginator.timeLength(from, to))
                                }
                                return ret
                            }
                        }
                    }
                }
            }
        }

        Item {
            id: _toolTip

            property MouseArea source: null

            width: root.listView.orientation === Qt.Vertical ? root.contentWidth : 1
            height: root.listView.orientation === Qt.Horizontal ? 1 : root.contentHeight

            function set(_x, _y, _text, _source) {
                if((_text === "" || _text === undefined) && source === _source) {
                    unset(_source)
                    return
                }

                x = root.listView.orientation === Qt.Horizontal ? _x : root.width
                y = root.listView.orientation === Qt.Vertical ? _y : 0
                source = _source
                _tooltipPopup.text = _text
                _tooltipPopup.visible = true
            }

            function unset(_source) {
                if(source === _source) {
                    _tooltipPopup.visible = false
                    source = null
                }
            }

            ToolTipPopup {
                id: _tooltipPopup

                x: listView.orientation === Qt.Vertical ? Runtime.minimumFontMetrics.lineSpacing : 0
                y: listView.orientation === Qt.Horizontal ? -height - 15 : 0

                container: _toolTip
            }
        }
    }

    Connections {
        target: root.listView

        ignoreUnknownSignals: true

        function onHeaderItemChanged() {
            Qt.callLater(_private.reload)
        }

        function onDelegateCountChanged() {
            Qt.callLater(_private.reload)
        }

        function onCountChanged() {
            Qt.callLater(_private.reload)
        }

        function onCacheBufferChanged() {
            Qt.callLater(_private.reload)
        }
    }

    QtObject {
        id: _private

        property real trackMargin: 0.5
        property real headerSize: root.listView.headerItem ? (root.listView.orientation === Qt.Horizontal ? root.listView.headerItem.width : root.listView.headerItem.height) : 0
        property bool displayTracks: true

        property rect viewportRect: Qt.rect( visibleArea.xPosition * contentWidth,
                                            visibleArea.yPosition * contentHeight,
                                            visibleArea.widthRatio * contentWidth,
                                            visibleArea.heightRatio * contentHeight )

        property ScreenplayTracks model: root.enabled && displayTracks ? Runtime.screenplayTracks : null

        readonly property SequentialAnimation reloadTask: SequentialAnimation {
            alwaysRunToEnd: false
            ScriptAction { script: _private.displayTracks = false }
            PauseAnimation { duration: 0  }
            ScriptAction { script: _private.displayTracks = true }
        }

        function reload() {
            reloadTask.start()
        }
    }
}
