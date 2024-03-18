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

Item {
    id: dmgBackdrop
    readonly property real iconSize: 128
    width: iconSize * 7
    height: iconSize * 5

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
        anchors.margins: 15
        color: "#65318f"
    }

    Text {
        text: "You're so close to writing your next blockbuster on Scrite!\nDrag the Scrite icon to the Applications folder."
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        anchors.topMargin: 128
        font.family: "Rubik"
        font.pixelSize: parent.height * 0.0325
        color: "gray"
    }

    Text {
        text: "{{VERSION}}"
        font.pixelSize: 18
        font.family: "Rubik"
        color: "black"
        x: parent.width - width - 30
        y: 55 - height/2
    }

    Timer {
        running: backdropImage.status === Image.Ready
        interval: 250
        repeat: false
        onTriggered: {
            dmgBackdrop.grabToImage(function(result) {
                result.saveToFile("background.png");
                Qt.quit()
            }, Qt.size(dmgBackdrop.width*2,dmgBackdrop.height*2));
        }
    }
}
