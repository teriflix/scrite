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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "AboutDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "About Scrite"
        width: {
            const bgImageAspectRatio = 1464.0/978.0
            return height * bgImageAspectRatio
        }
        height: {
            const bgImageHeight = 978
            return Math.min(bgImageHeight*0.8, Scrite.window.height * 0.8)
        }

        content: Item {
            implicitHeight: aboutInfoLayout.implicitHeight + 40

            Image {
                anchors.fill: parent
                source: "../../images/aboutbox.jpg"
                fillMode: Image.PreserveAspectCrop
                smooth: true; mipmap: true
            }

            VclLabel {
                id: versionText
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 30

                text: "Version-"+Scrite.app.versionAsString + (Scrite.app.versionType !== "" ? "-" + Scrite.app.versionType : "") + " for " + [Platform.typeString, Platform.osVersionString].join("-")
                width: Math.min(Runtime.idealFontMetrics.advanceWidth(text), dialog.width*0.5)
                elide: Text.ElideLeft
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }

            VclText {
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
                    Layout.preferredWidth: dialog.width * 0.3
                    Layout.preferredHeight: sourceSize.height * Layout.preferredWidth/sourceSize.width

                    source: "../../images/scrite_logo_for_report_header.png"
                    fillMode: Image.PreserveAspectFit
                    mipmap: true; smooth: true
                }

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 14
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 10

                    Link {
                        text: "Terms Of Use"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/terms-of-use/")
                    }

                    VclText {
                        text: "•"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }

                    Link {
                        text: "Privacy Policy"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/privacy-policy/")
                    }

                    VclText {
                        text: "•"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }

                    Link {
                        text: "Refund Policy"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/refund-and-cancellation-policy/")
                    }
                }

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 14
                }

                VclLabel {
                    Layout.alignment: Qt.AlignHCenter

                    text: "The app uses:"
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: dialog.width * 0.5
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

                            Component.onCompleted: {
                                append({
                                           "credits": "<strong>Qt</strong> " + Platform.qtVersionString + " as UI framework for the entire app.",
                                           "url": "https://www.qt.io"
                                       })

                                if(Platform.isWindowsDesktop || Platform.isLinuxDesktop) {
                                    const v = Platform.openSslVersionString
                                    append({
                                                "credits": "<strong>" + v + "</strong> for use with https protocol.",
                                                "url": "https://openssl-library.org/news/openssl-1.1.1-notes/index.html"
                                           })
                                }
                            }
                        }
                        ScrollBar.vertical: ScrollBar { }
                        delegate: VclLabel {
                            required property string credits
                            required property url url

                            id: creditLabel
                            text: credits
                            color: creditLabelMouseArea.containsMouse ? "blue" : "black"
                            width: creditsView.width // - (creditsView.ScrollBar.vertical.needed ? 20 : 0)
                            wrapMode: Text.WordWrap
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
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

                    VclLabel {
                        Layout.alignment: Qt.AlignVCenter

                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        leftPadding: 10; rightPadding: 3
                        text: "Share Scrite: "
                    }

                    RowLayout {
                        spacing: -8

                        VclToolButton {
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50

                            flat: true
                            toolTipText: "Post about Scrite on your Facebook page."
                            icon.source: "../../icons/action/share_on_facebook.png"

                            onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_facebook")
                        }

                        VclToolButton {
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50

                            flat: true
                            toolTipText: "Post about Scrite on your LinkedIn page."
                            icon.source: "../../icons/action/share_on_linkedin.png"

                            onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_linkedin")
                        }

                        VclToolButton {
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50

                            flat: true
                            toolTipText: "Tweet about Scrite from your handle."
                            icon.source: "../../icons/action/share_on_twitter.png"

                            onClicked: Qt.openUrlExternally("https://www.scrite.io?share_on_twitter")
                        }

                        VclToolButton {
                            readonly property string url: "mailto:?Subject=Take a look at Scrite&Body=I am using Scrite and I thought you should check it out as well. Visit https://www.scrite.io"

                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50

                            flat: true
                            toolTipText: "Send an email about Scrite."
                            icon.source: "../../icons/action/share_on_email.png"

                            onClicked: Qt.openUrlExternally(url)
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
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/help/")
                    }

                    Button {
                        text: "Discord"
                        onClicked: JoinDiscordCommunity.launch()
                    }
                }
            }
        }
    }
}
