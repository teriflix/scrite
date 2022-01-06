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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

/**
  This item is used for highlighting UI elements to educate users about where
  certain options are present.
  */

Item {
    id: uiElementHighlight
    property Item uiElement
    property string description
    property int descriptionPosition: Item.Right
    property bool uiElementBoxVisible: false
    property bool highlightAnimationEnabled: true

    signal done()
    signal scaleAnimationDone()

    ItemPositionMapper {
        id: uiElementPosition
        from: uiElement
        to: uiElementHighlight
    }

    Item {
        id: uiElementOverlay
        x: uiElementPosition.mappedPosition.x
        y: uiElementPosition.mappedPosition.y
        width: uiElement.width * uiElement.scale
        height: uiElement.height * uiElement.scale

        Rectangle {
            anchors.fill: parent
            anchors.margins: -2.5
            color: Qt.rgba(0,0,0,0)
            border.width: 2
            border.color: accentColors.highlight.background
            visible: uiElementBoxVisible
        }

        BoxShadow {
            anchors.fill: descTip
        }

        Rectangle {
            id: descTip
            color: accentColors.highlight.background
            width: descLabel.width
            height: descLabel.height
            border.width: 1
            border.color: accentColors.borderColor

            Label {
                id: descLabel
                text: description
                font.bold: true
                font.pointSize: Scrite.app.idealFontPointSize+2
                color: accentColors.highlight.text
                leftPadding: (descriptionPosition === Item.Right ? descIcon.width : (descriptionPosition === Item.Bottom || descriptionPosition === Item.Top ? 20 : 0)) + 5
                rightPadding: (descriptionPosition === Item.Left ? descIcon.width : (descriptionPosition === Item.Bottom || descriptionPosition === Item.Top ? 20 : 0)) + 5
                topPadding: descriptionPosition === Item.Bottom || descriptionPosition === Item.Top ? descIcon.height : 10
                bottomPadding: descriptionPosition === Item.Top || descriptionPosition === Item.Bottom ? descIcon.height : 10

                Image {
                    id: descIcon
                    width: idealAppFontMetrics.height
                    height: width
                    smooth: true
                    source: {
                        switch(descriptionPosition) {
                        case Item.Right:
                            return "../icons/navigation/arrow_left_inverted.png"
                        case Item.Right:
                            return "../icons/navigation/arrow_right_inverted.png"
                        case Item.Top:
                            return "../icons/navigation/arrow_down_inverted.png"
                        case Item.Bottom:
                            return "../icons/navigation/arrow_up_inverted.png"
                        }
                    }
                }
            }

            Component.onCompleted: {
                switch(descriptionPosition) {
                case Item.Right:
                    descTip.anchors.verticalCenter = uiElementOverlay.verticalCenter
                    descTip.anchors.left = uiElementOverlay.right
                    descIcon.anchors.verticalCenter = descLabel.verticalCenter
                    descIcon.anchors.left = descLabel.left
                    break
                case Item.Left:
                    descTip.anchors.verticalCenter = uiElementOverlay.verticalCenter
                    descTip.anchors.right = uiElementOverlay.left
                    descIcon.anchors.verticalCenter = descLabel.verticalCenter
                    descIcon.anchors.right = descLabel.right
                    break
                case Item.Top:
                    descTip.anchors.horizontalCenter = uiElementOverlay.horizontalCenter
                    descTip.anchors.bottom = uiElementOverlay.top
                    descIcon.anchors.horizontalCenter = descLabel.horizontalCenter
                    descIcon.anchors.bottom = descLabel.bottom
                    break
                case Item.Bottom:
                    descTip.anchors.horizontalCenter = uiElementOverlay.horizontalCenter
                    descTip.anchors.top = uiElementOverlay.bottom
                    descIcon.anchors.horizontalCenter = descLabel.horizontalCenter
                    descIcon.anchors.top = descLabel.top
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
