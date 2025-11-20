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

    required property ListView screenplayElementList

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object,event,result) => {
        EventFilter.forwardEventTo(screenplayElementList)
        result.filter = true
        result.accepted = true
    }

    height: contentHeight

    clip: true
    contentX: screenplayElementList.contentX - screenplayElementList.originX
    interactive: false
    contentWidth: _screenplayTracksFlickContent.width
    contentHeight: _screenplayTracksFlickContent.height

    Item {
        id: _screenplayTracksFlickContent

        width: screenplayElementList.contentWidth
        height: Runtime.screenplayTracks.trackCount * (Runtime.minimumFontMetrics.height + 10)

        Repeater {
            model: Runtime.screenplayTracks

            Rectangle {
                id: _track

                required property int index
                required property var track // of type ScreenplayTrack, a Q_GADGET declared in screenplay.h
                                            // struct ScreenplayTrack { QString name; QList<ScreenplayTrackItem> items; }

                property var items: track.items
                property string name: track.name

                y: index * (Runtime.minimumFontMetrics.height + 10)

                width: _screenplayTracksFlickContent.width
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

                        property var extents: screenplayElementList.extents(startIndex, endIndex)

                        property int startIndex: modelData.startIndex
                        property int endIndex: modelData.endIndex

                        property string name: modelData.name

                        x: extents.from

                        width: extents.to - extents.from
                        height: parent.height-4

                        color: parent.border.color
                        border.color: Color.translucent(Color.textColorFor(color), 0.25)
                        border.width: 0.5

                        anchors.verticalCenter: parent.verticalCenter

                        VclLabel {
                            anchors.centerIn: parent

                            width: parent.width-10

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

                y: -height - 15

                container: _toolTip
            }
        }
    }
}
