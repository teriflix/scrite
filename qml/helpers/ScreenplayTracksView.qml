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

    readonly property alias model: _private.model
    readonly property alias trackCount: _trackRepeater.count

    function reload() { _private.reload() }

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object,event,result) => {
        EventFilter.forwardEventTo(listView)
        result.filter = true
        result.accepted = true
    }


    clip: true
    contentX: _private.isHorizontalTrack ? listView.contentX - listView.originX: 0
    contentY: _private.isHorizontalTrack ? 0 : listView.contentY - listView.originY
    interactive: false
    contentWidth: _content.width
    contentHeight: _content.height

    Item {
        id: _content

        width: _private.isHorizontalTrack ? listView.contentItem.width : _private.totalTracksSize
        height: _private.isHorizontalTrack ? _private.totalTracksSize : listView.contentItem.height

        Repeater {
            id: _trackRepeater

            model: _private.model

            delegate: Rectangle {
                id: _track

                required property int index
                required property var track // of type ScreenplayTrack, a Q_GADGET declared in screenplay.h
                                            // struct ScreenplayTrack { QString name; QList<ScreenplayTrackItem> items; }

                property var items: track.items
                property string name: track.name

                property int offset: index * _private.trackSize

                x: _private.isHorizontalTrack ? 0 : offset
                y: _private.isHorizontalTrack ? offset : 0

                width: _private.isHorizontalTrack ? _content.width : _private.trackSize
                height: _private.isHorizontalTrack ? _private.trackSize : _content.height

                color: Runtime.colors.primary.c100.background

                border.color: Runtime.colors.primary.borderColor
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
                            _toolTip.set(mouseX, mouseY, _track.name + " Track", _trackMouseArea)
                        } else if(_toolTip.source === _trackMouseArea) {
                            _toolTip.unset(_trackMouseArea)
                        }
                    }
                }

                Repeater {
                    model: _track.items

                    delegate: Rectangle {
                        id: _trackItem

                        required property int index
                        required property var modelData // of type ScreenplayTrackItem, a Q_GADGET declared in screenplay.h
                                                        // struct ScreenplayTrackItem { int startIndex, endIndex; QString name; QColor color; }

                        property QtObject extents: QtObject {
                            property real from: {
                                if(_trackItem.startItem === null || _trackItem.endItem === null)
                                    return 0

                                return (_private.isHorizontalTrack ? _trackItem.startItem.x - listView.originX :
                                                                     _trackItem.startItem.y - listView.originY)
                            }
                            property real to: {
                                if(_trackItem.startItem === null || _trackItem.endItem === null)
                                    return 0

                                return (_private.isHorizontalTrack ? _trackItem.endItem.x + _trackItem.endItem.width - listView.originX :
                                                                     _trackItem.endItem.y + _trackItem.endItem.height - listView.originY)
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

                        x: _private.isHorizontalTrack ? extents.from : 0
                        y: _private.isHorizontalTrack ? 0 : extents.from

                        width: _private.isHorizontalTrack ? extents.to - extents.from : _private.trackSize
                        height: _private.isHorizontalTrack ? _private.trackSize : extents.to - extents.from

                        color: Runtime.colors.tint(modelData.color, Runtime.colors.screenplayTracksTint)
                        visible: GMath.doRectanglesIntersect(itemRect, _private.viewportRect)

                        border.color: Qt.darker(modelData.color, 1.2)
                        border.width: 0.5

                        VclLabel {
                            anchors.centerIn: parent

                            width: (_private.isHorizontalTrack ? parent.width : parent.height) - 10

                            rotation: _private.isHorizontalTrack ? 0 : -90
                            transformOrigin: Item.Center

                            font: Runtime.minimumFontMetrics.font
                            elide: Text.ElideMiddle
                            color: Color.textColorFor(Color.mix(_track.color, _trackItem.color))
                            horizontalAlignment: Text.AlignHCenter

                            text: _trackItem.name
                        }

                        MouseArea {
                            id: _trackItemMouseArea

                            anchors.fill: parent

                            acceptedButtons: Qt.LeftButton|Qt.RightButton
                            hoverEnabled: true

                            onClicked: (mouse) => {
                                           root.screenplay.currentElementIndex = _trackItem.startItem.index
                                       }

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
                                const isKeywordsTrack = _track.name === ""
                                let ret = "<b>" + (isKeywordsTrack ? "" : _track.name + " &gt; ") + _trackItem.name + "</b>, "
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

            width: _private.isHorizontalTrack ? 1 : root.contentWidth
            height: _private.isHorizontalTrack ? 1 : root.contentHeight

            function set(_x, _y, _text, _source) {
                if((_text === "" || _text === undefined) && source === _source) {
                    unset(_source)
                    return
                }

                x = _private.isHorizontalTrack ? _x : root.width
                y = _private.isHorizontalTrack ? 0 : _y
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

                x: _private.isHorizontalTrack ? 0 : Runtime.minimumFontMetrics.lineSpacing
                y: _private.isHorizontalTrack ? -height - 15 : 0

                container: _toolTip
                parseShortcutInText: false
            }
        }
    }

    Connections {
        target: listView

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

        function onContentWidthChanged() {
            if(_private.isHorizontalTrack)
                Qt.callLater(_private.reload)
        }

        function onContentHeightChanged() {
            if(!_private.isHorizontalTrack)
                Qt.callLater(_private.reload)
        }
    }

    QtObject {
        id: _private

        property int trackSize: Math.ceil(Runtime.minimumFontMetrics.lineSpacing) + 8
        property int totalTracksSize: model.trackCount * trackSize
        property bool isHorizontalTrack: listView.orientation === Qt.Horizontal

        property rect viewportRect: Qt.rect( visibleArea.xPosition * contentWidth,
                                            visibleArea.yPosition * contentHeight,
                                            visibleArea.widthRatio * contentWidth,
                                            visibleArea.heightRatio * contentHeight )

        property ScreenplayTracks model: ScreenplayTracks {
            property bool enabled: root.enabled && Runtime.screenplayTracksSettings.displayTracks && Runtime.appFeatures.structure.enabled

            structure: Scrite.document.structure
            screenplay: Scrite.document.screenplay
            includeStacks: enabled && Runtime.screenplayTracksSettings.displayStacks
            includeOpenTags: enabled && Runtime.screenplayTracksSettings.displayKeywordsTracks
            includeStructureTags: enabled && Runtime.screenplayTracksSettings.displayStructureTracks

            allowedOpenTags: {
                if(enabled) {
                    const userData = Scrite.document.userData
                    if(userData && userData.allowedOpenTagsInTracks !== undefined && userData.allowedOpenTagsInTracks.length > 0)
                        return userData.allowedOpenTagsInTracks
                }
                return []
            }

            onModelReset: {
                root.implicitWidth = _private.isHorizontalTrack ? 0 : trackCount * _private.trackSize
                root.implicitHeight = _private.isHorizontalTrack ? trackCount * _private.trackSize : 0
            }
        }

        function reload() {
            model.reload()
        }
    }
}
