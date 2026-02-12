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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

VclLabel {
    property alias containsMouse: _mouseArea.containsMouse

    property color hoverColor: enabled ? Runtime.colors.accent.c700.background : Runtime.colors.primary.c700.background
    property color defaultColor: enabled ? Runtime.colors.accent.c500.background : Runtime.colors.primary.c500.background

    signal clicked()

    color: _mouseArea.containsMouse ? hoverColor : defaultColor

    font.pointSize: Runtime.idealFontMetrics.font.pointSize
    font.underline: true

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onClicked: parent.clicked()
    }
}
