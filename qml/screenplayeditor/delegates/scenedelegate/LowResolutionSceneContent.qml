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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor/delegates"
import "qrc:/qml/screenplayeditor/delegates/sidepanel"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

Rectangle {
    id: root

    required property AbstractScreenplayElementDelegate sceneDelegate

    z: 1

    implicitHeight: _sceneSizeHint.active ? (_headerLayout.height + _sceneSizeHint.height + _pageBreakAfter.height)
                                          : sceneDelegate.screenplayElement.heightHint * sceneDelegate.zoomLevel

    color: sceneDelegate.scene ? Qt.tint(sceneDelegate.scene.color, Runtime.colors.sceneHeadingTint) : Runtime.colors.primary.c300.background

    Column {
        id: _headerLayout

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Item {
            width: parent.width
            height: sceneDelegate.screenplayElement.pageBreakBefore ? sceneDelegate.fontMetrics.lineSpacing*0.7 : 1
        }

        Row {
            id: _placeHolderHeaderLayout

            width: parent.width

            VclLabel {
                width: sceneDelegate.pageLeftMargin

                text: sceneDelegate.screenplayElement.resolvedSceneNumber
                font: sceneDelegate.font
                color: sceneDelegate.screenplayElement.hasUserSceneNumber ? "black" : "gray"
                topPadding: sceneDelegate.fontMetrics.lineSpacing * 0.5
                bottomPadding: sceneDelegate.fontMetrics.lineSpacing * 0.5
                rightPadding: 20
                horizontalAlignment: Text.AlignRight
            }

            VclLabel {
                width: parent.width - sceneDelegate.pageLeftMargin - sceneDelegate.pageRightMargin

                font: sceneDelegate.font
                color: screenplayElementType === ScreenplayElement.BreakElementType ? "gray" : "black"
                elide: Text.ElideMiddle
                topPadding: sceneDelegate.fontMetrics.lineSpacing * 0.5
                bottomPadding: sceneDelegate.fontMetrics.lineSpacing * 0.5

                text: {
                    if(sceneDelegate.screenplayElementType === ScreenplayElement.BreakElementType)
                        return sceneDelegate.screenplayElement.breakTitle
                    if(sceneDelegate.screenplayElement.omitted)
                        return "[OMITTED]"
                    if(sceneDelegate.scene && sceneDelegate.scene.heading.enabled)
                        return sceneDelegate.scene.heading.text
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
            opacity: 0.5
            fillMode: Image.TileVertically
        }

        SceneSizeHintItem {
            id: _sceneSizeHint

            width: contentWidth * zoomLevel
            height: contentHeight * zoomLevel

            scene: sceneDelegate.scene
            active: sceneDelegate.screenplayElement.heightHint === 0
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

        height: sceneDelegate.screenplayElement.pageBreakAfter ? sceneDelegate.fontMetrics.lineSpacing*0.7 : 1
    }
}

