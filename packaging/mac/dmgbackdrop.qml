import QtQuick 2.13

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
        text: "You're so close to writing your next blockbuster on Scrite!\nTo install Scrite, drag the icon to the Applications folder."
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
        color: "white"
        x: parent.width * 0.045
        y: parent.height * 0.145
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
