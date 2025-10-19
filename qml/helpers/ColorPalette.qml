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

import io.scrite.components 1.0

import "qrc:/qml/globals"

GridLayout {
    id: root

    property int cellSize: Runtime.iconImageSize
    property color selectedColor: Runtime.colors.transparent
    property alias colors: _colorsView.model

    signal colorPicked(color newColor)

    columns: 7
    opacity: enabled ? 1 : 0.25

    Item {
        Layout.preferredWidth: root.cellSize
        Layout.preferredHeight: root.cellSize

        Image {
            anchors.fill: parent
            anchors.margins: 5

            source: "qrc:/icons/navigation/close.png"
            mipmap: true
        }

        MouseArea {
            anchors.fill: parent

            onClicked: colorPicked(Runtime.colors.transparent)
        }
    }

    Repeater {
        id: _colorsView

        model: Runtime.colors.forDocument

        Item {
            required property int index
            required property color modelData

            Layout.preferredWidth: root.cellSize
            Layout.preferredHeight: root.cellSize

            Rectangle {
                anchors.fill: parent
                anchors.margins: 3

                color: modelData

                border.width: root.selectedColor === modelData ? 3 : 0.5
                border.color: "black"
            }

            MouseArea {
                anchors.fill: parent

                onClicked: colorPicked(modelData)
            }
        }
    }
}
