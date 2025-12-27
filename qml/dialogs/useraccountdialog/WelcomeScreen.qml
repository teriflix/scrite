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

Item {
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
            enabled: !_welcomeTextApi.busy

            onClicked: {
                Runtime.userAccountDialogSettings.welcomeScreenShown = true
                Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "AccountEmailScreen")
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: _welcomeTextApi.busy
    }

    AppWelcomeTextApiCall {
        id: _welcomeTextApi

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
