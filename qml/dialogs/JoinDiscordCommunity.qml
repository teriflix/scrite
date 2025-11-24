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

    readonly property url infoUrl: "https://www.scrite.io/index.php/forum/"
    readonly property url inviteUrl: "https://discord.gg/bGHquFX5jK"
    property bool infoUrlOpened: false

    name: "JoinDiscordCommunity"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "Join us on Discord"

        width: 640
        height: 550

        content: Item {
            AppFeature {
                id: emailSupport
                featureName: "support/email"
            }

            ColumnLayout {
                anchors.centerIn: parent

                spacing: 20

                Rectangle {
                    Layout.preferredWidth: 450
                    Layout.preferredHeight: 74
                    Layout.alignment: Qt.AlignHCenter

                    color: Runtime.colors.accent.c600.background

                    Image {
                        anchors.centerIn: parent

                        height: 64

                        source: "qrc:/images/scrite_discord_button.png"
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                }

                VclLabel {
                    Layout.preferredWidth: 450
                    Layout.alignment: Qt.AlignHCenter

                    wrapMode: Text.WordWrap
                    text: "Join the Scrite community on <b>Discord</b>. It is the best place to find <font color=\"" + Runtime.colors.accent.a700.background + "\"><b>support</b></font>, to connect with the Scrite team and a growing network of Scrite users who <b>share feedback</b>, place <b>feature requests</b> and stay upto date with Scrite <b>updates</b>, <b>features</b>, <b>bug fixes</b> and more.<br/><br/>If you already have Discord installed, use the invite link below."
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 20

                    VclText {
                        id: discordInviteLink
                        font.family: "Courier New"
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                        text: root.inviteUrl
                    }

                    ToolButton {
                        icon.source: "qrc:/icons/content/content_copy.png"
                        onClicked: {
                            clipboard.text = root.inviteUrl
                            MessageBox.information("Copy Successful",
                                                   "The invite link was copied to clipboard",
                                                   () => { })
                        }

                        SystemClipboard {
                            id: clipboard
                        }
                    }
                }

                VclText {
                    Layout.preferredWidth: 450
                    Layout.alignment: Qt.AlignHCenter

                    visible: !emailSupport.enabled
                    text: "Please note: There is <b>no phone or email support</b> available for Scrite."
                    color: Runtime.colors.primary.c600.background
                    wrapMode: Text.WordWrap
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    horizontalAlignment: Text.AlignHCenter
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 20

                    VclButton {
                        text: "More Info"
                        onClicked: {
                            Qt.openUrlExternally(root.infoUrl)
                            root.infoUrlOpened = true
                            if(dialog.titleBarCloseButtonVisible)
                                Qt.callLater(dialog.close)
                        }
                    }

                    VclButton {
                        text: "Open Discord"
                        onClicked: {
                            Qt.openUrlExternally(root.inviteUrl)
                            if(dialog.titleBarCloseButtonVisible)
                                Qt.callLater(dialog.close)
                        }
                    }
                }
            }
        }
    }
}
