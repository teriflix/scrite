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

pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "LicenseDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        readonly property string _versionString: Scrite.app.versionAsString +
            (Scrite.app.versionType !== "" ? "-" + Scrite.app.versionType : "")

        title: ""
        titleBarCloseButtonVisible: false

        width: Math.min(880, Scrite.window.width * 0.85)
        height: Math.min(680, Scrite.window.height * 0.85)

        content: Item {
            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Item {
                    Layout.fillWidth: true
                    implicitHeight: 80

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 1
                        color: Runtime.colors.tx("black")
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Image {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            Layout.alignment: Qt.AlignVCenter
                            source: "qrc:/images/appicon.png"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                        }

                        ColumnLayout {
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 2

                            VclLabel {
                                text: "Welcome to Scrite"
                                font.bold: true
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                            }

                            VclLabel {
                                text: "Version " + _dialog._versionString
                                color: Runtime.colors.tx("#5d208e")
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }
                }

                // Body — instruction + scrollable license text
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.topMargin: 12
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    spacing: 8

                    VclLabel {
                        Layout.fillWidth: true
                        text: "Please read and accept the following license agreement to use Scrite:"
                        wrapMode: Text.WordWrap
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                        TextArea {
                            text: Scrite.licenseText()
                            readOnly: true
                            selectByMouse: true
                            wrapMode: TextEdit.NoWrap
                            font.family: "Courier Prime"
                            font.pointSize: 10
                            background: Item { }
                        }
                    }
                }
            }
        }

        bottomBar: Component {
            Item {
                height: _buttons.height + 20

                RowLayout {
                    id: _buttons
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    spacing: 12

                    VclButton {
                        text: "Decline"
                        onClicked: Qt.quit()
                    }

                    VclButton {
                        text: "Accept"
                        onClicked: {
                            Scrite.acceptLicense()
                            _dialog.accept()
                        }
                    }
                }
            }
        }
    }
}
