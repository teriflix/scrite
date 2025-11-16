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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    GridLayout {
        id: layout

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 15
        anchors.leftMargin: 0

        columns: 2
        rowSpacing: 10
        columnSpacing: 10

        GroupBox {
            Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.columnSpacing)/layout.columns
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

            label: VclLabel {
                text: "Canvas Grid"
            }

            ColumnLayout {
                width: parent.width

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Show Grid in Structure Canvas"
                    checked: Runtime.structureCanvasSettings.showGrid
                    onToggled: Runtime.structureCanvasSettings.showGrid = checked
                }

                GridLayout {
                    Layout.alignment: Qt.AlignHCenter

                    enabled: Runtime.structureCanvasSettings.showGrid
                    opacity: enabled ? 1 : 0.5

                    columns: 3
                    rowSpacing: 10
                    columnSpacing: 10

                    Rectangle {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor
                        color: Runtime.structureCanvasSettings.canvasColor

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Runtime.structureCanvasSettings.canvasColor = Color.pick(Runtime.structureCanvasSettings.canvasColor)
                        }
                    }

                    VclLabel {
                        text: "Background Color"
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/reset.png"
                        toolTipText: "Reset canvas color"
                        onClicked: Runtime.structureCanvasSettings.restoreDefaultCanvasColor()
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
                            onClicked: Runtime.structureCanvasSettings.gridColor = Color.pick(Runtime.structureCanvasSettings.gridColor)
                        }
                    }

                    VclLabel {
                        text: "Grid Color"
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/reset.png"
                        toolTipText: "Reset grid color"
                        onClicked: Runtime.structureCanvasSettings.restoreDefaultGridColor()
                    }
                }
            }
        }

        GroupBox {
            Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.columnSpacing)/layout.columns
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

            label: VclLabel {
                text: "Parameters"
            }

            ColumnLayout {
                width: parent.width
                spacing: 10

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Use Index Card UI"
                    enabled: Scrite.document.structure.elementStacks.objectCount === 0
                    checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI
                    onToggled: {
                        Runtime.resetMainWindowUi()
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
            }
        }

        GroupBox {
            Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.columnSpacing)/layout.columns
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

            label: VclLabel {
                text: "Timeline"
            }

            ColumnLayout {
                width: parent.width

                spacing: 10

                VclLabel {
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

        GroupBox {
            Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.columnSpacing)/layout.columns
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

            label: VclLabel {
                text: "Zoom Speed"
            }

            ColumnLayout {
                width: parent.width

                spacing: 10

                VclLabel {
                    Layout.fillWidth: true

                    text: "Configure how fast/slow you want zoom in/out to be on the structure canvas while using mouse wheel."
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
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
            Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.columnSpacing)/layout.columns
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

            label: VclLabel {
                text: "Defaults"
            }

            GridLayout {
                anchors.horizontalCenter: parent.horizontalCenter

                enabled: Runtime.structureCanvasSettings.showGrid
                opacity: enabled ? 1 : 0.5

                columns: 3
                rowSpacing: 10
                columnSpacing: 10

                Rectangle {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor
                    color: Runtime.workspaceSettings.defaultSceneColor

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: defaultSceneColorMenu.open()
                    }

                    ColorMenu {
                        id: defaultSceneColorMenu
                        onMenuItemClicked: (color) => { Runtime.workspaceSettings.defaultSceneColor = color }
                    }
                }

                VclLabel {
                    text: "Default Scene Color"
                }

                FlatToolButton {
                    iconSource: "qrc:/icons/action/reset.png"
                    toolTipText: "Reset default scene color"
                    onClicked: Runtime.workspaceSettings.defaultSceneColor = SceneColors.palette[0]
                }
            }
        }

        GroupBox {
            Layout.preferredWidth: (layout.width-(layout.columns-1)*layout.columnSpacing)/layout.columns
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

            label: VclLabel {
                text: "Preview"
            }

            ColumnLayout {
                width: parent.width

                spacing: 10

                VclLabel {
                    Layout.fillWidth: true

                    text: "Configure the max-size (width or height) the preview panel can occupy in the structure canvas."
                    wrapMode: Text.WordWrap
                }

                VclTextField {
                    Layout.fillWidth: true

                    placeholderText: "Preview Size (50 - 1000)"
                    text: Runtime.structureCanvasSettings.previewSize
                    validator: DoubleValidator {
                        bottom: 50
                        top: 1000
                        decimals: 0
                    }

                    onTextEdited: Runtime.structureCanvasSettings.previewSize = parseFloat(text)
                }
            }
        }
    }
}
