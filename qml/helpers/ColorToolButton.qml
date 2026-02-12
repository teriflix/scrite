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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

Item {
    id: root

    property color selectedColor: Runtime.colors.transparent
    property alias hoverEnabled: cbMouseArea.hoverEnabled
    property alias containsMouse: cbMouseArea.containsMouse
    property bool colorsMenuVisible: colorsMenuLoader.active

    signal colorPicked(color newColor)

    width: implicitWidth
    height: implicitHeight
    implicitWidth: 36
    implicitHeight: 36

    opacity: enabled ? 1 : 0.5

    MouseArea {
        id: cbMouseArea
        anchors.fill: parent
        onClicked: colorsMenuLoader.active = true
    }

    Loader {
        id: colorsMenuLoader
        x: 0; y: parent.height
        active: false
        sourceComponent: Popup {
            id: colorsMenu
            x: 0; y: 0
            width: availableColorsPalette.suggestedWidth
            height: availableColorsPalette.suggestedHeight
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            Component.onCompleted: open()
            onClosed: Qt.callLater(() => { colorsMenuLoader.active = false})

            contentItem: AvailableColorsPalette {
                id: availableColorsPalette
                selectedColor: root.selectedColor
                onColorPicked: (newColor) => {
                                   root.colorPicked(newColor)
                                   colorsMenu.close()
                               }
            }
        }
    }

    component AvailableColorsPalette : Grid {
        id: colorsGrid
        property int cellSize: width/columns
        readonly property int suggestedWidth: 280
        readonly property int suggestedHeight: 200
        columns: 7
        opacity: enabled ? 1 : 0.25

        property color selectedColor: Runtime.colors.transparent
        signal colorPicked(color newColor)

        Item {
            width: colorsGrid.cellSize
            height: colorsGrid.cellSize

            Image {
                source: "qrc:/icons/navigation/close.png"
                anchors.fill: parent
                anchors.margins: 5
                mipmap: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: colorPicked(Runtime.colors.transparent)
            }
        }

        Repeater {
            model: Runtime.colors.forDocument

            delegate: Item {
                required property int index
                required property color modelData

                width: colorsGrid.cellSize
                height: colorsGrid.cellSize

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 3
                    border.width: colorsGrid.selectedColor === modelData ? 3 : 0.5
                    border.color: "black"
                    color: modelData
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: colorPicked(modelData)
                }
            }
        }
    }
}
