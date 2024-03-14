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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    ColumnLayout {
        id: layout

        height: parent.height-30
        width: Math.min(parent.width-15, 300)
        anchors.fill: parent
        anchors.margins: 15
        anchors.leftMargin: 0

        spacing: 10

        GroupBox {
            Layout.fillWidth: true

            label: CheckBox {
                text: "Show Grid in Structure Canvas"
                checked: Runtime.structureCanvasSettings.showGrid
                onToggled: Runtime.structureCanvasSettings.showGrid = checked
            }

            RowLayout {
                enabled: Runtime.structureCanvasSettings.showGrid
                opacity: enabled ? 1 : 0.5
                width: parent.width
                spacing: 20

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    spacing: parent.spacing/2

                    VclText {
                        text: "Background Color"
                    }

                    Rectangle {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor
                        color: Runtime.structureCanvasSettings.canvasColor

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Runtime.structureCanvasSettings.canvasColor = Scrite.app.pickColor(Runtime.structureCanvasSettings.canvasColor)
                        }
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/reset.png"
                        onClicked: Runtime.structureCanvasSettings.restoreDefaultCanvasColor()
                        ToolTip.text: "Reset canvas color"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    spacing: parent.spacing/2

                    VclText {
                        text: "Grid Color"
                    }

                    Rectangle {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor
                        color: Runtime.structureCanvasSettings.gridColor

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Runtime.structureCanvasSettings.gridColor = Scrite.app.pickColor(Runtime.structureCanvasSettings.gridColor)
                        }
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/reset.png"
                        onClicked: Runtime.structureCanvasSettings.restoreDefaultGridColor()
                        ToolTip.text: "Reset grid color"
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "Parameters"
            }

            ColumnLayout {
                width: parent.width

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Use Index Card UI"
                    enabled: Scrite.document.structure.elementStacks.objectCount === 0
                    checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI
                    onToggled: {
                        Announcement.shout(Runtime.announcementIds.reloadMainUiRequest)
                        Scrite.document.structure.canvasUIMode = Structure.IndexCardUI
                        Scrite.document.structure.indexCardContent = Structure.Synopsis
                    }
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Show Pull Handle Animation"
                    checked: Runtime.structureCanvasSettings.showPullHandleAnimation
                    onToggled: Runtime.structureCanvasSettings.showPullHandleAnimation = checked
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Runtime.colors.primary.borderColor
                }

                VclText {
                    Layout.fillWidth: true
                    text: "Zoom Speed"
                }

                Slider {
                    Layout.fillWidth: true

                    from: 1
                    to: 100
                    orientation: Qt.Horizontal
                    snapMode: Slider.SnapAlways
                    value: Runtime.scrollAreaSettings.zoomFactor * 100
                    onMoved: Runtime.scrollAreaSettings.zoomFactor = value / 100
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "Timeline"
            }

            ColumnLayout {
                spacing: 20
                width: parent.width

                VclText {
                    Layout.fillWidth: true
                    text: "What text do you want to display on cards in the timeline?"
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }

                VclComboBox {
                    Layout.fillWidth: true

                    model: [
                        { "label": "Scene Heading Or Title", "value": "HeadingOrTitle" },
                        { "label": "Scene Synopsis", "value": "Synopsis" }
                    ]
                    textRole: "label"
                    currentIndex: Runtime.timelineViewSettings.textMode === "HeadingOrTitle" ? 0 : 1
                    onActivated: Runtime.timelineViewSettings.textMode = model[currentIndex].value
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
