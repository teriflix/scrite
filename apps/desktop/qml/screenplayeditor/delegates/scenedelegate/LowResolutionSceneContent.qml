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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import io.scrite.components

import "../../../globals"
import "../../../dialogs"
import "../../../helpers"
import "../../../controls"
import ".."
import "../sidepanel"
import "../sceneparteditors"
import "../sceneparteditors/helpers"

Rectangle {
    id: root

    required property bool showSceneComments
    required property AbstractScreenplayElementSceneDelegate sceneDelegate

    z: 1

    height: implicitHeight
    implicitHeight: Math.max( (_sceneSizeHint.active ? (_headerLayout.height + _sceneSizeHint.height + _pageBreakAfter.height)
                                                      : root.sceneDelegate.screenplayElement.heightHint * root.sceneDelegate.zoomLevel),
                              (showSceneComments && root.sceneDelegate.spaceAvailableForScenePanel >= Runtime.minSceneSidePanelWidth ? 300 : 0) )

    color: root.sceneDelegate.scene ? Runtime.colors.tintTx(root.sceneDelegate.scene.color, Runtime.colors.sceneHeadingTint) : Runtime.colors.primary.c300.background

    Column {
        id: _headerLayout

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Item {
            width: parent.width
            height: root.sceneDelegate.screenplayElement.pageBreakBefore ? root.sceneDelegate.fontMetrics.lineSpacing*0.7 : 1
        }

        Row {
            id: _placeHolderHeaderLayout

            width: parent.width

            VclLabel {
                width: root.sceneDelegate.pageLeftMargin

                text: root.sceneDelegate.screenplayElement.resolvedSceneNumber
                font: root.sceneDelegate.font
                color: root.sceneDelegate.screenplayElement.hasUserSceneNumber ? "black" : "gray"
                topPadding: root.sceneDelegate.fontMetrics.lineSpacing * 0.5
                bottomPadding: root.sceneDelegate.fontMetrics.lineSpacing * 0.5
                rightPadding: 20
                horizontalAlignment: Text.AlignRight
            }

            VclLabel {
                width: parent.width - root.sceneDelegate.pageLeftMargin - root.sceneDelegate.pageRightMargin

                font: root.sceneDelegate.font
                color: root.sceneDelegate.screenplayElementType === ScreenplayElement.BreakElementType ? "gray" : "black"
                elide: Text.ElideMiddle
                topPadding: root.sceneDelegate.fontMetrics.lineSpacing * 0.5
                bottomPadding: root.sceneDelegate.fontMetrics.lineSpacing * 0.5

                text: {
                    if(root.sceneDelegate.screenplayElementType === ScreenplayElement.BreakElementType)
                        return root.sceneDelegate.screenplayElement.breakTitle
                    if(root.sceneDelegate.screenplayElement.omitted)
                        return "[OMITTED]"
                    if(root.sceneDelegate.scene && root.sceneDelegate.scene.heading.enabled)
                        return root.sceneDelegate.scene.heading.text
                    return "NO SCENE HEADING"
                }
            }
        }
    }

    Item {
        anchors.top: _headerLayout.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: _pageBreakAfter.top

        Image {
            anchors.fill: parent

            source: "qrc:/images/sample_scene.png"
            opacity: Runtime.colors.scheme === Qt.ColorScheme.Dark ? 0.25 : 0.5
            fillMode: Image.TileVertically
        }

        SceneSizeHintItem {
            id: _sceneSizeHint

            width: contentWidth * root.sceneDelegate.zoomLevel
            height: contentHeight * root.sceneDelegate.zoomLevel

            scene: root.sceneDelegate.scene
            active: root.sceneDelegate.screenplayElement.heightHint === 0
            format: Scrite.document.printFormat
            visible: false
            asynchronous: false
        }
    }

    Item {
        id: _pageBreakAfter

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: root.sceneDelegate.screenplayElement.pageBreakAfter ? root.sceneDelegate.fontMetrics.lineSpacing*0.7 : 1
    }
}

