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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "../../js/utils.js" as Utils
import "../globals"
import "../controls"

VclDialog {
    id: aboutDialog

    title: "About Scrite"
    width: {
        const bgImageAspectRatio = 1464.0/978.0
        return height * bgImageAspectRatio
    }
    height: {
        const bgImageHeight = 978
        return Math.min(bgImageHeight*0.8, Scrite.window.height * 0.8)
    }

    backdrop: Image {
        source: "../../images/aboutbox.jpg"
        fillMode: Image.PreserveAspectFit
        smooth: true; mipmap: true
    }

    content: Item {
        implicitHeight: aboutInfoLayout.implicitHeight + 40

        Text {
            id: versionText
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 30
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            text: Scrite.app.applicationVersion
            font.letterSpacing: Runtime.applicationSettings.enableAnimations ? 20 : 0

            NumberAnimation {
                target: versionText
                property: "font.letterSpacing"
                from: 20; to: 0
                duration: 1500
                running: Runtime.applicationSettings.enableAnimations
                easing.type: Easing.OutBack
            }
        }

        Text {
            font.pixelSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Build Timestamp:\n" + Scrite.app.buildTimestamp
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 30
        }

        ColumnLayout {
            id: aboutInfoLayout
            spacing: 10
            anchors.centerIn: parent

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: aboutDialog.width * 0.3
                Layout.preferredHeight: sourceSize.height * Layout.preferredWidth/sourceSize.width

                source: "../../images/scrite_logo_for_report_header.png"
                fillMode: Image.PreserveAspectFit
                mipmap: true; smooth: true
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 14
            }

            Text {
                Layout.alignment: Qt.AlignHCenter

                text: "This app is released under <strong>GPLv3</strong>.<br/><font color=\"blue\">Click here</font> to view the license terms."
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                color: "gray"

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: licenseTermsDialog.open()
                }
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 14
            }

            Text {
                Layout.alignment: Qt.AlignHCenter

                text: "The app uses:"
                font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: aboutDialog.width * 0.5
                Layout.preferredHeight: (Runtime.minimumFontMetrics.height+creditsView.spacing) * (creditsView.model.count+1) + creditsView.anchors.topMargin + creditsView.anchors.bottomMargin

                // color: creditsView.ScrollBar.vertical.needed ? Runtime.colors.primary.c100.background : Qt.rgba(0,0,0,0)

                // Refactoring QML TODO: Add ScrollBar back to this.
                ListView {
                    id: creditsView
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 7
                    clip: true
                    model: ListModel {
                        ListElement {
                            credits: "<strong>Phonetic Translator</strong> library for providing static transliteration."
                            url: "https://sourceforge.net/projects/phtranslator/"
                        }

                        ListElement {
                            credits: "<strong>Sonnet</strong> from KDE Frameworks for powering English spell check."
                            url: "https://api.kde.org/frameworks/sonnet/html/index.html"
                        }

                        ListElement {
                            credits: "<strong>QuaZip</strong> for (un)compressing Scrite documents."
                            url: "https://github.com/stachenov/quazip"
                        }

                        ListElement {
                            credits: "<strong>SimpleCrypt</strong> for encrypting Scrite documents."
                            url: "https://wiki.qt.io/Simple_encryption_with_SimpleCrypt"
                        }

                        ListElement {
                            credits: "<strong>Curved-Arrows</strong> library for evaluating curved arrow connectors."
                            url: "https://github.com/dragonman225/curved-arrows"
                        }

                        ListElement {
                            credits: "<strong>QuillJS</strong> for powering rich text editor in Notebook."
                            url: "https://quilljs.com/"
                        }

                        ListElement {
                            credits: "<strong>Qt</strong> 5.15 LTS for developing the entire app."
                            url: "https://www.qt.io"
                        }
                    }
                    ScrollBar.vertical: ScrollBar { }
                    delegate: Label {
                        required property string credits
                        required property url url

                        id: creditLabel
                        text: credits
                        color: creditLabelMouseArea.containsMouse ? "blue" : "black"
                        width: creditsView.width // - (creditsView.ScrollBar.vertical.needed ? 20 : 0)
                        wrapMode: Text.WordWrap
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                        horizontalAlignment: Text.AlignHCenter

                        MouseArea {
                            id: creditLabelMouseArea
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally(url)
                            hoverEnabled: true
                        }
                    }
                }
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 14
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter

                Image {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24

                    source: "../../icons/action/share.png"
                }

                Text {
                    Layout.alignment: Qt.AlignVCenter

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    leftPadding: 10; rightPadding: 3
                    text: "Share Scrite: "
                }

                RowLayout {
                    spacing: -8

                    ToolButton {
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        flat: true
                        icon.source: "../../icons/action/share_on_facebook.png"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_facebook")
                        ToolTip.text: "Post about Scrite on your Facebook page."
                    }

                    ToolButton {
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        flat: true
                        icon.source: "../../icons/action/share_on_linkedin.png"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_linkedin")
                        ToolTip.text: "Post about Scrite on your LinkedIn page."
                    }

                    ToolButton {
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        flat: true
                        icon.source: "../../icons/action/share_on_twitter.png"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_twitter")
                        ToolTip.text: "Tweet about Scrite from your handle."
                    }

                    ToolButton {
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        flat: true
                        icon.source: "../../icons/action/share_on_email.png"

                        readonly property string url: "mailto:?Subject=Take a look at Scrite&Body=I am using Scrite and I thought you should check it out as well. Visit https://www.scrite.io"
                        onClicked: Qt.openUrlExternally(url)
                        ToolTip.text: "Send an email about Scrite."
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter

                spacing: 20

                Button {
                    text: "Website"
                    onClicked: Qt.openUrlExternally("https://www.scrite.io")
                }

                Button {
                    text: "Learning Guides"
                    onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/help/")
                }

                Button {
                    text: "Discord"
                    onClicked: Qt.openUrlExternally("https://discord.gg/bGHquFX5jK")
                }
            }
        }
    }

    VclDialog {
        id: licenseTermsDialog

        title: "Terms Of Use"
        width: aboutDialog.width * 0.9
        height: aboutDialog.height * 0.9

        content: TextEdit {
            padding: 40
            readOnly: true
            font.family: "Courier Prime"
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            topPadding: backButton.y
            bottomPadding: backButton.y
            text: Scrite.app.fileContents(":/LICENSE.txt")
            selectByMouse: true
        }
    }
}
