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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../controls"

Item {
    id: root
    readonly property bool modal: true
    readonly property string title: "Greetings!"

    Image {
        anchors.fill: parent
        source: "qrc:/images/useraccountdialogbg.png"
        fillMode: Image.PreserveAspectCrop
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 50
        anchors.leftMargin: 50
        anchors.rightMargin: 175
        anchors.bottomMargin: 50

        spacing: 40

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c10.background
            border.width: _welcomeTextFlick.clip ? 1 : 0
            border.color: Runtime.colors.primary.borderColor

            Flickable {
                id: _welcomeTextFlick

                anchors.fill: parent
                anchors.margins: 1

                ScrollBar.vertical: VclScrollBar { }

                clip: contentHeight > height
                contentWidth: _welcomeText.width
                contentHeight: _welcomeText.height

                TextArea {
                    id: _welcomeText

                    width: _welcomeTextFlick.width - 20
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                    wrapMode: Text.WordWrap
                    readOnly: true
                    padding: _welcomeTextFlick.clip ? 10 : 0

                    background: Item { }

                    onLinkActivated: (link) => {
                                         Qt.openUrlExternally(link)
                                     }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: parent.linkAt(mouseX, mouseY) === "" ? Qt.ArrowCursor : Qt.PointingHandCursor
                        onClicked: {
                            const link = parent.linkAt(mouseX, mouseY)
                            if(link !== "")
                                parent.linkActivated(link)
                        }
                    }
                }
            }
        }

        VclButton {
            Layout.alignment: Qt.AlignRight

            text: "Next »"
            enabled: !_root_2.busy

            onClicked: {
                Runtime.userAccountDialogSettings.welcomeScreenShown = true
                Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "AccountEmailScreen")
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: _root_2.busy
    }

    AppWelcomeTextApiCall {
        id: _root_2

        onFinished: {
            if(hasResponse && !hasError) {
                const wt = welcomeText
                switch(wt.format) {
                case "html":
                    _welcomeText.textFormat = TextEdit.RichText
                    break
                case "markdown":
                    _welcomeText.textFormat = TextEdit.MarkdownText
                    break
                default:
                    _welcomeText.textFormat = TextEdit.PlainText
                    break
                }

                _welcomeText.text = wt.content
            }
        }

        Component.onCompleted: call()
    }
}
