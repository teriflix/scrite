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
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"

Rectangle {
    id: root

    required property int canvasEpisodeBoxCount

    required property bool canvasPreviewVisible
    required property bool canvasScrollInteractive

    required property real canvasScale
    required property real canvasScrollMinimumScale
    required property real canvasScrollMaximumScale

    required property BoundingBoxEvaluator canvasItemsBoundingBox

    signal zoomInRequest()
    signal zoomOutRequest()
    signal zoomToRequest(real zoomValue)
    signal zoomFitRequest(rect boundingBox)
    signal zoomOneRequest()

    height: 30

    clip: true
    color: Runtime.colors.primary.windowColor
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    VclText {
        anchors.left: parent.left
        anchors.right: _statusBarControls.left
        anchors.margins: 10
        anchors.verticalCenter: parent.verticalCenter

        elide: Text.ElideRight
        font.pixelSize: root.height * 0.5

        text: {
            if(!canvasScrollInteractive)
                return "Canvas Locked While Index Card Has Focus. Hit ESC To Release Focus."
            var ret = Scrite.document.structure.elementCount + " Scenes";
            if(canvasEpisodeBoxCount > 0)
                ret += ", " + canvasEpisodeBoxCount + " Episodes";
            if(Scrite.document.structure.forceBeatBoardLayout)
                ret += ", Scenes Not Movable"
            ret += "."
            return ret;
        }

        MouseArea {
            anchors.fill: parent

            ToolTip.text: parent.text
            ToolTip.visible: containsMouse

            enabled: parent.truncated
            hoverEnabled: true
        }
    }

    Row {
        id: _statusBarControls

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        height: parent.height-6
        spacing: 10

        FlatToolButton {
            ToolTip.text: "Toggle between index-card view options."

            suggestedWidth: parent.height
            suggestedHeight: parent.height

            down: _structureViewOptionsMenu.active
            checkable: false
            autoRepeat: false
            iconSource: "qrc:/icons/content/view_options.png"

            onClicked: _structureViewOptionsMenu.show()

            MenuLoader {
                id: _structureViewOptionsMenu

                anchors.left: parent.left
                anchors.bottom: parent.top
                anchors.bottomMargin: item ? item.height : 0

                menu: VclMenu {
                    VclMenuItem {
                        property bool _checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI &&
                                                Scrite.document.structure.indexCardContent === Structure.Synopsis

                        text: "Index Cards"
                        icon.source: _checked ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                        onClicked: {
                            if(!_checked) {
                                if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI)
                                    Scrite.document.structure.indexCardContent = Structure.Synopsis
                                else
                                    Runtime.resetMainWindowUi( () => {
                                        Scrite.document.structure.canvasUIMode = Structure.IndexCardUI
                                        Scrite.document.structure.indexCardContent = Structure.Synopsis
                                    } )
                            }
                        }
                    }

                    VclMenuItem {
                        property bool _checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI &&
                                                Scrite.document.structure.indexCardContent === Structure.FeaturedPhoto

                        text: "Photo Cards"
                        icon.source: _checked ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                        onClicked: {
                            if(!_checked)
                                if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI)
                                    Scrite.document.structure.indexCardContent = Structure.FeaturedPhoto
                                else
                                    Runtime.resetMainWindowUi( () => {
                                        Scrite.document.structure.canvasUIMode = Structure.IndexCardUI
                                        Scrite.document.structure.indexCardContent = Structure.FeaturedPhoto
                                    } )
                        }
                    }

                    VclMenuItem {
                        property bool _checked: Scrite.document.structure.canvasUIMode === Structure.SynopsisEditorUI

                        text: "Synopsis Cards"
                        icon.source: _checked ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                        onClicked: {
                            if(!_checked) {
                                if(Scrite.document.structure.elementCount > 0)
                                    MessageBox.information("Cannot Switch to Synopsis Cards",
                                                           "Switching to Synopsis Cards is only possible when the structure canvas is empty.")
                                else
                                    Runtime.resetMainWindowUi( () => {
                                        Scrite.document.structure.canvasUIMode = Structure.SynopsisEditorUI
                                    } )
                            }
                        }
                    }
                }
            }
        }

        FlatToolButton {
            ToolTip.text: "Mouse wheel currently " + (checked ? "zooms" : "scrolls") + ". Click this button to make it " + (checked ? "scroll" : "zoom") + "."

            suggestedWidth: parent.height
            suggestedHeight: parent.height

            checked: Runtime.workspaceSettings.mouseWheelZoomsInStructureCanvas
            checkable: true
            autoRepeat: false
            iconSource: "qrc:/icons/hardware/mouse.png"

            onCheckedChanged: Runtime.workspaceSettings.mouseWheelZoomsInStructureCanvas = checked
        }

        FlatToolButton {
            ToolTip.text: "Preview"

            suggestedWidth: parent.height
            suggestedHeight: parent.height

            down: canvasPreviewVisible
            checked: canvasPreviewVisible
            checkable: true
            iconSource: "qrc:/icons/action/thumbnail.png"

            onToggled: Runtime.structureCanvasSettings.showPreview = checked
        }

        Rectangle {
            height: parent.height
            width: 1
            color: Runtime.colors.primary.borderColor
        }

        FlatToolButton {
            id: _cmdZoomOne

            ToolTip.text: "Zoom One"

            suggestedWidth: parent.height
            suggestedHeight: parent.height

            enabled: root.canvasItemsBoundingBox.itemCount > 0
            autoRepeat: true
            iconSource: "qrc:/icons/navigation/zoom_one.png"

            onClicked: root.zoomOneRequest()
        }

        FlatToolButton {
            id: _cmdZoomFit

            ToolTip.text: "Zoom Fit"

            suggestedWidth: parent.height
            suggestedHeight: parent.height

            enabled: root.canvasItemsBoundingBox.itemCount > 0
            autoRepeat: true
            iconSource: "qrc:/icons/navigation/zoom_fit.png"

            onClicked: root.zoomFitRequest(root.canvasItemsBoundingBox.boundingBox)
        }

        ZoomSlider {
            id: _zoomSlider

            anchors.verticalCenter: parent.verticalCenter

            height: parent.height

            to: root.canvasScrollMaximumScale
            from: root.canvasScrollMinimumScale
            value: root.canvasScale
            stepSize: 0.0

            onSliderMoved: root.zoomToRequest(zoomLevel)
            onZoomInRequest: root.zoomInRequest()
            onZoomOutRequest: root.zoomOutRequest()
        }
    }
}
