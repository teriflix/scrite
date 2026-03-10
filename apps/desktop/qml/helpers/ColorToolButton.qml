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

import QtQuick
import QtQuick.Controls

import io.scrite.components

import "../globals"

Item {
    id: root

    property color selectedColor: Runtime.colors.transparent
    property alias hoverEnabled: _cbMouseArea.hoverEnabled
    property alias containsMouse: _cbMouseArea.containsMouse
    property bool _colorsMenuVisible: _colorsMenuLoader.active

    signal colorPicked(color newColor)

    width: implicitWidth
    height: implicitHeight
    implicitWidth: 36
    implicitHeight: 36

    opacity: enabled ? 1 : 0.5

    MouseArea {
        id: _cbMouseArea
        anchors.fill: parent
        onClicked: _colorsMenuLoader.active = true
    }

    Loader {
        id: _colorsMenuLoader

        x: 0
        y: parent.height

        active: false

        sourceComponent: Popup {
            id: _colorsMenu

            Component.onCompleted: open()

            x: 0
            y: 0

            width: _availableColorsPalette.suggestedWidth
            height: _availableColorsPalette.suggestedHeight

            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            onClosed: Qt.callLater(() => { _colorsMenuLoader.active = false})

            contentItem: AvailableColorsPalette {
                id: _availableColorsPalette

                selectedColor: root.selectedColor

                onColorPicked: (newColor) => {
                                   root.colorPicked(newColor)
                                   _colorsMenu.close()
                               }
            }
        }
    }

    component AvailableColorsPalette : Grid {
        id: _colorsGrid

        property int cellSize: width/columns
        readonly property int suggestedWidth: 280
        readonly property int suggestedHeight: 200

        columns: 7
        opacity: enabled ? 1 : 0.25

        property color selectedColor: Runtime.colors.transparent
        signal colorPicked(color newColor)

        Item {
            width: _colorsGrid.cellSize
            height: _colorsGrid.cellSize

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
                id: _delegate

                required property int index
                required property color modelData

                width: _colorsGrid.cellSize
                height: _colorsGrid.cellSize

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 3
                    border.width: _colorsGrid.selectedColor === _delegate.modelData ? 3 : 0.5
                    border.color: "black"
                    color: _delegate.modelData
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: colorPicked(_delegate.modelData)
                }
            }
        }
    }
}
