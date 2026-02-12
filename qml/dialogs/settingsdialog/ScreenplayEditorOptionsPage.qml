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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    Rectangle {
        anchors.fill: _flickable
        anchors.margins: -3

        visible: _flickable.clip

        border.width: 1
        border.color: Runtime.colors.primary.borderColor
    }

    Flickable {
        id: _flickable

        anchors.fill: parent
        anchors.margins: 20

        ScrollBar.vertical: VclScrollBar { }

        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        clip: contentHeight > height
        contentWidth: _layout.width
        contentHeight: _layout.height

        ColumnLayout {
            id: _layout

            width: _flickable.width - (_flickable.clip ? 20 : 0)

            VclGroupBox {
                Layout.fillWidth: true

                title: "Visual Options"

                RowLayout {
                    width: parent.width

                    VclLabel {
                        text: "Color Intensity"
                    }

                    Slider {
                        orientation: Qt.Horizontal
                        to: 1
                        from: 0
                        value: Runtime.applicationSettings.colorIntensity
                        onMoved: Runtime.applicationSettings.colorIntensity = value
                    }
                }
            }

            VclGroupBox {
                Layout.fillWidth: true

                title: "Toggleable Options"

                Grid {
                    id: _grid

                    width: parent.width

                    columns: 2
                    flow: Grid.TopToBottom

                    Repeater {
                        model: ActionHub.screenplayEditorOptions.visibleActions

                        delegate: VclCheckBox {
                            required property int index
                            required property var modelData

                            property var qmlAction: modelData

                            width: (_grid.width-_grid.columnSpacing)/_grid.columns

                            action: qmlAction

                            ToolTipPopup {
                                text: {
                                    const sc = Gui.nativeShortcut(qmlAction.shortcut)
                                    if(sc === "")
                                        return qmlAction.tooltip !== undefined ? qmlAction.tooltip : ""

                                    const tt = qmlAction.tooltip !== undefined ? qmlAction.tooltip : qmlAction.text
                                    return tt + " (" + sc + " )"
                                }
                                visible: text !== "" && parent.hovered
                            }
                        }
                    }
                }
            }
        }
    }
}
