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
import "qrc:/qml/controls"

Popup {
    id: root

    property alias message: _message.text

    signal deletionConfirmed()

    width: Math.min(parent ? parent.width * 0.75 : Scrite.window * 0.75, 400)
    height: _layout.implicitHeight + 22

    modal: true
    closePolicy: Popup.NoAutoClose

    ColumnLayout {
        id: _layout

        anchors.centerIn: parent

        width: root.width - 22

        VclLabel {
            id: _message

            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            text: "Are you sure you want to delete this?"
            wrapMode: Text.WordWrap

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            VclButton {
                focusPolicy: Qt.NoFocus
                text: "Yes"

                onClicked: {
                    root.deletionConfirmed()
                    root.close()
                }
            }

            VclButton {
                focusPolicy: Qt.NoFocus
                text: "No"

                onClicked: {
                    root.close()
                }
            }
        }
    }
}
