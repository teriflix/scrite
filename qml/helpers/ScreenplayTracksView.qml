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

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object,event,result) => {
        EventFilter.forwardEventTo(listView)
        result.filter = true
        result.accepted = true
    }

    height: contentHeight

    clip: true
    contentX: listView.contentX - listView.originX
    interactive: false
    contentWidth: _content.width
    contentHeight: _content.height

    Item {
        id: _content

        width: listView.orientation === Qt.Horizontal ? listView.contentWidth : Runtime.screenplayTracks.trackCount * (Runtime.minimumFontMetrics.height + 10)
        height: listView.orientation === Qt.Horizontal ? Runtime.screenplayTracks.trackCount * (Runtime.minimumFontMetrics.height + 10) : listView.contentHeight

        Repeater {
            model: Runtime.screenplayTracks

            Rectangle {
                id: _track

                required property int index
                required property var track // of type ScreenplayTrack, a Q_GADGET declared in screenplay.h
                                            // struct ScreenplayTrack { QString name; QList<ScreenplayTrackItem> items; }

                property var items: track.items
                property string name: track.name

                property real offset: index * (Runtime.minimumFontMetrics.height + 10)

                x: listView.orientation == Qt.Vertical ? offset : 0
                y: listView.orientation == Qt.Horizontal ? offset : 0

                width: _content.width
                height: Runtime.minimumFontMetrics.height + 8

                color: Color.translucent( border.color, 0.1 )
                border.color: Runtime.colors.accent.c900.background
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
                                                        // struct ScreenplayTrackItem { int startIndex, endIndex; QString name; }

                        property var extents: listView.extents(startIndex, endIndex)

                        property int startIndex: modelData.startIndex
                        property int endIndex: modelData.endIndex

                        property string name: modelData.name

                        x: listView.orientation === Qt.Horizontal ? extents.from : 2
                        y: listView.orientation === Qt.Vertical ? extents.from : 2

                        width: listView.orientation === Qt.Horizontal ? extents.to - extents.from : parent.width-4
                        height: listView.orientation === Qt.Horizontal ? parent.height-4 : extents.to - extents.from

                        color: parent.border.color
                        border.color: Color.translucent(Color.textColorFor(color), 0.25)
                        border.width: 0.5

                        VclLabel {
                            anchors.centerIn: parent

                            width: (listView.orientation === Qt.Vertical ? parent.height : parent.width) - 10

                            rotation: listView.orientation === Qt.Vertical ? 90 : 0
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
                                    _toolTip.set(mouseX + _trackItem.x, mouseY, toolTipText(), _trackItemMouseArea)
                                } else if(_toolTip.source === _trackItemMouseArea) {
                                    _toolTip.unset(_trackItemMouseArea)
                                }
                            }

                            function toolTipText() {
                                let ret = "<b>" + _track.name + " &gt; " + _trackItem.name + "</b>, "
                                if(_trackItem.endIndex === _trackItem.startIndex)
                                    ret += "1 Scene"
                                else
                                    ret += (1 + _trackItem.endIndex - _trackItem.startIndex) + " Scenes"
                                if(!Runtime.paginator.paused) {
                                    let from = Scrite.document.screenplay.elementWithIndex(_trackItem.startIndex)
                                    let to = Scrite.document.screenplay.elementWithIndex(_trackItem.endIndex)
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

            function set(_x, _y, _text, _source) {
                if((_text === "" || _text === undefined) && source === _source) {
                    unset(_source)
                    return
                }

                x = _x
                y = _y
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

                x: listView.orientation === Qt.Vertical ? 15 : 0
                y: listView.orientation === Qt.Horizontal ? -height - 15 : 0

                container: _toolTip
            }
        }
    }
}
