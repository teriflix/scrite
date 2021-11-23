/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml 2.13
import QtQuick 2.13

Text {
    property alias containsMouse: linkMouseArea.containsMouse
    signal clicked()

    font.pointSize: app.idealFontPointSize
    font.underline: true
    color: linkMouseArea.containsMouse ? hoverColor : defaultColor

    property color hoverColor: "blue"
    property color defaultColor: primaryColors.c10.text

    MouseArea {
        id: linkMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
        cursorShape: Qt.PointingHandCursor
        onContainsMouseChanged: console.log("PA: " + text + "/" + containsMouse)
    }
}
