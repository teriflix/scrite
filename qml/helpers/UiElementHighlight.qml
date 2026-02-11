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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

/**
  This item is used for highlighting UI elements to educate users about where
  certain options are present.
  */

Item {
    id: root

    property int descriptionPosition: Item.Right

    property string description

    property bool uiElementBoxVisible: false
    property bool highlightAnimationEnabled: true

    property Item uiElement

    signal done()
    signal scaleAnimationDone()

    ItemPositionMapper {
        id: _uiElementPosition

        to: root
        from: uiElement
    }

    Item {
        id: _uiElementOverlay

        x: _uiElementPosition.mappedPosition.x
        y: _uiElementPosition.mappedPosition.y
        width: root.uiElement ? (root.uiElement.width * uiElement.scale) : 0
        height: root.uiElement ? (root.uiElement.height * uiElement.scale) : 0

        Rectangle {
            anchors.fill: parent
            anchors.margins: -2.5

            color: Qt.rgba(0,0,0,0)
            visible: root.uiElementBoxVisible

            border.width: 2
            border.color: Runtime.colors.accent.highlight.background
        }

        BoxShadow {
            anchors.fill: _descTip
        }

        Rectangle {
            id: _descTip

            width: _descLabel.width
            height: _descLabel.height

            color: Runtime.colors.accent.highlight.background
            border.width: 1
            border.color: Runtime.colors.accent.borderColor

            VclLabel {
                id: _descLabel

                text: description
                color: Runtime.colors.accent.highlight.text

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize+2

                topPadding: descriptionPosition === Item.Bottom || descriptionPosition === Item.Top ? _descIcon.height : 10
                leftPadding: (descriptionPosition === Item.Right ? _descIcon.width : (descriptionPosition === Item.Bottom || descriptionPosition === Item.Top ? 20 : 0)) + 5
                rightPadding: (descriptionPosition === Item.Left ? _descIcon.width : (descriptionPosition === Item.Bottom || descriptionPosition === Item.Top ? 20 : 0)) + 5
                bottomPadding: descriptionPosition === Item.Top || descriptionPosition === Item.Bottom ? _descIcon.height : 10

                Image {
                    id: _descIcon

                    width: Runtime.idealFontMetrics.height
                    height: width
                    smooth: true

                    source: {
                        switch(descriptionPosition) {
                        case Item.Right:
                            return "qrc:/icons/navigation/arrow_left_inverted.png"
                        case Item.Right:
                            return "qrc:/icons/navigation/arrow_right_inverted.png"
                        case Item.Top:
                        case Item.TopLeft:
                        case Item.TopRight:
                            return "qrc:/icons/navigation/arrow_down_inverted.png"
                        case Item.Bottom:
                        case Item.BottomLeft:
                        case Item.BottomRight:
                            return "qrc:/icons/navigation/arrow_up_inverted.png"
                        }
                    }
                }
            }

            Component.onCompleted: {
                switch(descriptionPosition) {
                case Item.Right:
                    _descTip.anchors.verticalCenter = _uiElementOverlay.verticalCenter
                    _descTip.anchors.left = _uiElementOverlay.right
                    _descIcon.anchors.verticalCenter = _descLabel.verticalCenter
                    _descIcon.anchors.left = _descLabel.left
                    break
                case Item.Left:
                    _descTip.anchors.verticalCenter = _uiElementOverlay.verticalCenter
                    _descTip.anchors.right = _uiElementOverlay.left
                    _descIcon.anchors.verticalCenter = _descLabel.verticalCenter
                    _descIcon.anchors.right = _descLabel.right
                    break
                case Item.Top:
                case Item.TopLeft:
                case Item.TopRight:
                    if(descriptionPosition === Item.Top) {
                        _descTip.anchors.horizontalCenter = _uiElementOverlay.horizontalCenter
                        _descIcon.anchors.horizontalCenter = _descLabel.horizontalCenter
                    } else if(descriptionPosition === Item.TopRight) {
                        _descTip.anchors.left = _uiElementOverlay.left
                        _descTip.anchors.leftMargin = _descLabel.leftPadding
                        _descIcon.anchors.left = _descLabel.left
                        _descIcon.anchors.leftMargin = _descLabel.leftPadding
                    } else {
                        _descTip.anchors.right = _uiElementOverlay.right
                        _descTip.anchors.rightMargin = _descLabel.rightPadding
                        _descIcon.anchors.right = _descLabel.right
                        _descIcon.anchors.right = _descLabel.rightPadding
                    }
                    _descTip.anchors.bottom = _uiElementOverlay.top
                    _descIcon.anchors.bottom = _descLabel.bottom
                    break
                case Item.Bottom:
                case Item.BottomLeft:
                case Item.BottomRight:
                    if(descriptionPosition === Item.Top) {
                        _descTip.anchors.horizontalCenter = _uiElementOverlay.horizontalCenter
                        _descIcon.anchors.horizontalCenter = _descLabel.horizontalCenter
                    } else if(descriptionPosition === Item.BottomRight) {
                        _descTip.anchors.left = _uiElementOverlay.left
                        _descTip.anchors.leftMargin = _descLabel.leftPadding
                        _descIcon.anchors.left = _descLabel.left
                        _descIcon.anchors.leftMargin = _descLabel.leftPadding
                    } else {
                        _descTip.anchors.right = _uiElementOverlay.right
                        _descTip.anchors.rightMargin = _descLabel.rightPadding
                        _descIcon.anchors.right = _descLabel.right
                        _descIcon.anchors.rightMargin = _descLabel.rightPadding
                    }
                    _descTip.anchors.top = _uiElementOverlay.bottom
                    _descIcon.anchors.top = _descLabel.top
                    break
                }
            }
        }
    }

    SequentialAnimation {
        running: true

        NumberAnimation {
            target: uiElement
            property: "scale"
            from: 1; to: highlightAnimationEnabled ? 2 : 1
            duration: 500
        }

        PauseAnimation {
            duration: 250
        }

        NumberAnimation {
            target: uiElement
            property: "scale"
            from: highlightAnimationEnabled ? 2 : 1; to: 1
            duration: 500
        }

        ScriptAction {
            script: scaleAnimationDone()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: done()
    }

    Timer {
        running: true
        repeat: false
        interval: 4000
        onTriggered: done()
    }
}
