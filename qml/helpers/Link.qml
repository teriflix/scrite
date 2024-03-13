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

import io.scrite.components 1.0

import "qrc:/qml/globals"

Label {
    property alias containsMouse: linkMouseArea.containsMouse
    signal clicked()

    font.pointSize: Runtime.idealFontMetrics.font.pointSize
    font.underline: true
    color: linkMouseArea.containsMouse ? hoverColor : defaultColor

    property color hoverColor: "#65318f"
    property color defaultColor: Runtime.colors.primary.c10.text

    MouseArea {
        id: linkMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
        cursorShape: Qt.PointingHandCursor
    }
}
