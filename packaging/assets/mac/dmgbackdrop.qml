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

import QtQml
import QtQuick

Item {
    id: dmgBackdrop
    width: 448
    height: 340

    Rectangle {
        anchors.fill: parent
        color: "white"
    }

    Image {
        id: backdropImage
        source: "dmgbackdrop.png"
        anchors.fill: parent
        smooth: true; mipmap: true
        fillMode: Image.PreserveAspectFit
    }

    Text {
        text: "https://www.scrite.io"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        font.family: "Rubik"
        font.pixelSize: parent.height * 0.03
        anchors.margins: 8
        color: "#65318f"
    }

    Text {
        text: "You're so close to writing your next blockbuster on Scrite!\nDrag the Scrite icon to the Applications folder."
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        anchors.topMargin: 68
        font.family: "Rubik"
        font.pixelSize: 10
        color: "gray"
    }

    Text {
        text: "{{VERSION}}"
        font.pixelSize: 7
        font.family: "Rubik"
        color: "black"
        x: parent.width - width - 15
        y: 20 - height/2
    }

    Timer {
        running: backdropImage.status === Image.Ready
        interval: 250
        repeat: false
        onTriggered: {
            dmgBackdrop.grabToImage(function(result) {
                result.saveToFile("background.png");
                Qt.quit()
            }, Qt.size(dmgBackdrop.width, dmgBackdrop.height));
        }
    }
}
