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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../"
import "../../tasks"
import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    ColumnLayout {
        id: _userInstallationsPageLayout

        anchors.fill: parent
        anchors.margins: 20
        anchors.leftMargin: 0

        // Header Section
        VclLabel {
            Layout.fillWidth: true

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 1
            text: {
                const total = Scrite.user.info.installations.length
                const active = Scrite.user.info.activeInstallationCount
                if(active === total)
                    return active + " Installations Active"
                return total + " Total Installations, " + active + " Active"
            }
        }

        // Devices Grid
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.transparent
            border.width: _devicesFlickable.contentHeight > _devicesFlickable.height ? 1 : 0
            border.color: Runtime.colors.primary.borderColor

            Flickable {
                id: _devicesFlickable

                anchors.fill: parent
                anchors.margins: clip ? 2 : 0

                enabled: !_deactivateOtherCall.busy && !Scrite.user.busy
                opacity: enabled ? 1 : 0.5

                ScrollBar.vertical: VclScrollBar { }
                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                clip: contentHeight > height
                contentWidth: width
                contentHeight: _devicesFlow.height

                Flow {
                    id: _devicesFlow

                    width: _devicesFlickable.width - (_devicesFlickable.clip ? 20 : 0)
                    spacing: 10

                    Repeater {
                        model: Scrite.user.info.installations

                        delegate: Rectangle {
                            id: _deviceDelegate

                            required property int index
                            required property var modelData

                            width: Math.min(450, (_devicesFlow.width - _devicesFlow.spacing) / 2 - 1)
                            height: _deviceCardLayout.implicitHeight + 40

                            color: Runtime.colors.primary.c10.background
                            border.width: modelData.isCurrent ? 3 : 1
                            border.color: modelData.isCurrent ? Runtime.colors.accent.c600.background : Runtime.colors.primary.borderColor

                            ColumnLayout {
                                id: _deviceCardLayout

                                anchors.fill: parent
                                anchors.margins: 20

                                spacing: 10

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 15

                                    Rectangle {
                                        Layout.preferredWidth: 60
                                        Layout.preferredHeight: 60
                                        Layout.alignment: Qt.AlignTop

                                        color: Runtime.colors.primary.c100.background
                                        border.width: 1
                                        border.color: Runtime.colors.primary.borderColor
                                        radius: 4

                                        VclText {
                                            anchors.centerIn: parent

                                            text: "Mac"
                                            visible: _deviceDelegate.modelData.platform === "macOS"

                                            font.capitalization: Font.AllUppercase
                                            font.pixelSize: parent.height * 0.25
                                        }

                                        Image {
                                            anchors.fill: parent
                                            anchors.margins: 5

                                            mipmap: true
                                            fillMode: Image.PreserveAspectFit
                                            opacity: _deviceDelegate.modelData.isCurrent ? 1 : 0.6
                                            visible: _deviceDelegate.modelData.platform !== "macOS"

                                            source: {
                                                switch(_deviceDelegate.modelData.platform.toLowerCase()) {
                                                    case "windows": return "qrc:/icons/hardware/windows-platform.png"
                                                    case "linux": return "qrc:/icons/hardware/linux-platform.png"
                                                }
                                                return "qrc:/icons/hardware/desktop-platform.png"
                                            }
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 5

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 10

                                            VclLabel {
                                                Layout.fillWidth: true

                                                font.bold: _deviceDelegate.modelData.isCurrent
                                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 1
                                                text: {
                                                    let ret = ""
                                                    if(_deviceDelegate.modelData.hostName !== "")
                                                        ret = _deviceDelegate.modelData.hostName
                                                    else
                                                        ret = _deviceDelegate.modelData.platform + " Device"
                                                    return ret
                                                }
                                                elide: Text.ElideRight
                                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                                maximumLineCount: 2
                                            }

                                            Rectangle {
                                                visible: _deviceDelegate.modelData.isCurrent

                                                Layout.preferredHeight: _thisDeviceLabel.implicitHeight + 8
                                                Layout.preferredWidth: _thisDeviceLabel.implicitWidth + 12

                                                color: Runtime.colors.accent.c500.background
                                                radius: 4

                                                VclLabel {
                                                    id: _thisDeviceLabel
                                                    anchors.centerIn: parent

                                                    text: "THIS DEVICE"
                                                    color: Runtime.colors.accent.c500.text
                                                    font.bold: true
                                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                                                }
                                            }
                                        }

                                        VclLabel {
                                            Layout.fillWidth: true

                                            text: _deviceDelegate.modelData.platform + " " + _deviceDelegate.modelData.platformVersion
                                            elide: Text.ElideRight
                                            color: Runtime.colors.primary.c600.text
                                        }

                                        VclLabel {
                                            Layout.fillWidth: true

                                            text: "Scrite " + _deviceDelegate.modelData.appVersion
                                            elide: Text.ElideRight
                                            color: Runtime.colors.primary.c600.text
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    color: Runtime.colors.primary.borderColor
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    VclLabel {
                                        Layout.fillWidth: true

                                        text: _deviceDelegate.modelData.isCurrent ?
                                                    "Active since: " + Qt.formatDateTime(new Date(_deviceDelegate.modelData.lastSessionDate), "dddd, h:mm AP (MMM dd, yyyy)") :
                                                    "Last Login: " + TMath.relativeTime(new Date(_deviceDelegate.modelData.lastSessionDate))
                                        elide: Text.ElideRight
                                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        maximumLineCount: 2
                                    }

                                    VclButton {
                                        visible: !_deviceDelegate.modelData.isCurrent
                                        enabled: _deviceDelegate.modelData.activated

                                        text: "Sign Out"
                                        icon.source: "qrc:/icons/action/logout.png"
                                        icon.width: 16
                                        icon.height: 16

                                        onClicked: {
                                            _deactivateOtherCall.installationId = _deviceDelegate.modelData.id
                                            _deactivateOtherCall.call()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: _deactivateOtherCall.busy || Scrite.user.busy
    }

    InstallationDeactivateOtherRestApiCall {
        id: _deactivateOtherCall
    }
}
