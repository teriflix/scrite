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
                id: _outerTrackDelegate

                readonly property var trackData: modelData

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
                            _toolTipItem.x = mouseX
                            _toolTipItem.y = mouseY
                            _toolTipItem.ToolTip.text = "'" + _outerTrackDelegate.trackData.category + "' Track"
                            _toolTipItem.ToolTip.visible = true
                            _toolTipItem.source = _trackMouseArea
                        } else if(_toolTipItem.source === _trackMouseArea) {
                            _toolTipItem.ToolTip.visible = false
                            _toolTipItem.source = null
                        }
                    }
                }

                Repeater {
                    model: _outerTrackDelegate.trackData.tracks

                    Rectangle {
                        id: _innerTrackDelegate

                        readonly property var groupData: _outerTrackDelegate.trackData.tracks[index]
                        readonly property var groupExtents: screenplayElementList.extents(_innerTrackDelegate.groupData.startIndex, _innerTrackDelegate.groupData.endIndex)

                        x: groupExtents.from

                        width: groupExtents.to - groupExtents.from
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

                            text: _innerTrackDelegate.groupData.group
                        }

                        MouseArea {
                            id: _groupMouseArea

                            anchors.fill: parent

                            hoverEnabled: true

                            onMouseXChanged: maybeTooltip()
                            onMouseYChanged: maybeTooltip()
                            onContainsMouseChanged: maybeTooltip()

                            function maybeTooltip() {
                                if(containsMouse) {
                                    var ttText = "<b>" + _outerTrackDelegate.trackData.category + " &gt; " + _innerTrackDelegate.groupData.group + "</b>, "
                                    if(_innerTrackDelegate.groupData.endIndex === _innerTrackDelegate.groupData.startIndex)
                                        ttText += "1 Scene"
                                    else
                                        ttText += (1 + _innerTrackDelegate.groupData.endIndex - _innerTrackDelegate.groupData.startIndex) + " Scenes"
                                    if(!Runtime.screenplayTextDocument.paused) {
                                        var from = Scrite.document.screenplay.elementWithIndex(_innerTrackDelegate.groupData.startIndex)
                                        var to = Scrite.document.screenplay.elementWithIndex(_innerTrackDelegate.groupData.endIndex)
                                        ttText += ", Length: " + Runtime.screenplayTextDocument.lengthInTimeAsString(from, to)
                                    }

                                    _toolTipItem.x = mouseX + parent.x
                                    _toolTipItem.y = mouseY
                                    _toolTipItem.ToolTip.text = ttText
                                    _toolTipItem.ToolTip.visible = true
                                    _toolTipItem.source = _groupMouseArea
                                } else if(_toolTipItem.source === _groupMouseArea) {
                                    _toolTipItem.ToolTip.visible = false
                                    _toolTipItem.source = null
                                }
                            }
                        }
                    }
                }
            }
        }

        Item {
            id: _toolTipItem
            property MouseArea source: null
            ToolTip.delay: 1000
        }
    }
}
