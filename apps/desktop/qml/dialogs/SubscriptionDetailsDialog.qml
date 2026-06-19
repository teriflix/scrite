/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com)
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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"
import "../commandcenter"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    name: "SubscriptionDetailsDialog"
    singleInstanceOnly: true

    function init() { }
    function launch(subscription) {
        if(SubscriptionPlanOperations.taxonomy === undefined) {
            MessageBox.information("Error", "Unable to show subscription details at this point.")
            return
        }

        doLaunch({"subscription": subscription})
    }

    dialogComponent: VclDialog {
        id: _dialog

        property scriteUserSubscriptionInfo subscription

        width: Math.min(680, Scrite.window.width * 0.85)
        height: Math.min(520, Scrite.window.height * 0.8)
        title: "Subscription Details"
        titleBarCloseButtonVisible: true

        content: Item {
            Component.onCompleted: {
                if (!SubscriptionPlanOperations.taxonomy || !_dialog.subscription.valid)
                    return

                SubscriptionPlanOperations.taxonomy.features.forEach((feature) => {
                    if (feature.group === true) return
                    if (feature.display === false) return

                    const enabled = Scrite.isFeatureNameEnabled(feature.name, _dialog.subscription.plan.features)
                    if (enabled)
                        _includedModel.append({ "featureTitle": feature.title, "featureDescription": feature.description })
                    else
                        _excludedModel.append({ "featureTitle": feature.title, "featureDescription": feature.description })
                })
            }

            ListModel { id: _includedModel }
            ListModel { id: _excludedModel }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                // Plan summary — two columns: identity (left) | subscription status (right)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    // Left: Title, subtitle, duration · devices, support note, exclusive
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        VclLabel {
                            Layout.fillWidth: true
                            text: _dialog.subscription.plan.title
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                            wrapMode: Text.WordWrap
                        }

                        VclLabel {
                            Layout.fillWidth: true
                            text: _dialog.subscription.plan.subtitle
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            visible: text !== ""
                            wrapMode: Text.WordWrap
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 6

                            Row {
                                spacing: 4

                                VclLabel {
                                    text: Runtime.daysSpanAsString(_dialog.subscription.plan.duration) + "  ·  " +
                                          "Device Count: " + _dialog.subscription.plan.devices
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                }

                                VclLabel {
                                    text: "ⓘ"
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            const n = _dialog.subscription.plan.devices
                                            const opening = n === 1
                                                ? "This plan allows Scrite to be activated on 1 device at a time."
                                                : "This plan allows Scrite to be activated on up to " + n + " devices at a time."
                                            MessageBox.question(
                                                "About Device Activations",
                                                opening + "\n\nEach activation is tied to the specific OS installation and user account used at sign-in. This means reinstalling your OS, or signing in from a different user account on the same machine, counts as a separate activation.\n\nTo activate on a new device once the limit is reached, sign out from one of your existing activated devices first. Exceeding the limit without doing so will result in an activation error.",
                                                ["More Info", "Ok"],
                                                (btn) => {
                                                    if (btn === "More Info") {
                                                        const url = HelpCenter.lookup("device limits")
                                                        if (url.toString() !== "")
                                                            Qt.openUrlExternally(url)
                                                    }
                                                }
                                            )
                                        }
                                    }
                                }
                            }

                            Row {
                                spacing: 4
                                visible: !Scrite.isFeatureNameEnabled("support/email", _dialog.subscription.plan.features)

                                VclLabel {
                                    text: "·  Discord community support only."
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                }

                                VclLabel {
                                    text: "ⓘ"
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: JoinDiscordCommunity.launch()
                                    }
                                }
                            }
                        }

                        VclLabel {
                            Layout.fillWidth: true
                            text: "★  Exclusive Plan"
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            visible: _dialog.subscription.plan.exclusive
                        }
                    }

                    // Right: Status, date range, order link
                    ColumnLayout {
                        spacing: 4

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            text: _dialog.subscription.isActive   ? "Active"
                                : _dialog.subscription.isUpcoming ? "Upcoming"
                                : _dialog.subscription.hasExpired ? "Expired"
                                : "Unknown"
                            font.family: Runtime.shortcutFontMetrics.font.family
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 10
                            color: _dialog.subscription.isActive   ? Runtime.colors.primary.c700.background
                                 : _dialog.subscription.hasExpired ? Runtime.colors.accent.a700.background
                                 : Runtime.colors.primary.c400.background
                        }

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            text: Runtime.formatDateIncludingYear(new Date(_dialog.subscription.from)) +
                                  "  —  " +
                                  Runtime.formatDateIncludingYear(new Date(_dialog.subscription.until))
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }

                        Link {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Order #" + _dialog.subscription.wc_order_id + " »"
                            visible: _dialog.subscription.wc_order_id !== undefined && _dialog.subscription.wc_order_id !== ""
                            onClicked: Qt.openUrlExternally(_dialog.subscription.detailsUrl)
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Runtime.colors.primary.c400.background
                }

                // Two-column feature table
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 10

                    // Included column
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Runtime.colors.primary.c100.background
                        border.color: Runtime.colors.primary.c400.background
                        border.width: 1
                        clip: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: _includedHeader.implicitHeight
                                color: Runtime.colors.tx(Runtime.colors.accent.c600.background)
                                border.color: Runtime.colors.primary.c400.background
                                border.width: 1

                                VclLabel {
                                    id: _includedHeader
                                    width: parent.width
                                    text: "✓   Included"
                                    color: Runtime.colors.accent.c600.text
                                    font.bold: true
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                    padding: 8
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                ListView {
                                    id: _includedList

                                    anchors.fill: parent
                                    anchors.margins: 1
                                    anchors.topMargin: 0

                                    model: _includedModel
                                    clip: true
                                    keyNavigationEnabled: true

                                    ScrollBar.vertical: VclScrollBar { }

                                    delegate: Item {
                                        id: _includedDelegate
                                        required property int index
                                        required property string featureTitle
                                        required property string featureDescription

                                        readonly property bool isCurrent: ListView.isCurrentItem

                                        width: _includedList.width
                                        height: _includedDelegateLayout.implicitHeight

                                        Rectangle {
                                            anchors.fill: parent
                                            color: _includedDelegate.isCurrent
                                                   ? Runtime.colors.accent.c200.background
                                                   : (_includedDelegate.index % 2 === 0) ? Runtime.colors.primary.c50.background
                                                                                         : Runtime.colors.primary.c100.background
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                _includedList.forceActiveFocus()
                                                _includedList.currentIndex = _includedDelegate.isCurrent ? -1 : _includedDelegate.index
                                            }
                                        }

                                        ColumnLayout {
                                            id: _includedDelegateLayout
                                            width: parent.width
                                            spacing: 0

                                            VclLabel {
                                                Layout.fillWidth: true
                                                text: _includedDelegate.featureTitle
                                                font.bold: _includedDelegate.isCurrent
                                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                                topPadding: 8
                                                bottomPadding: _includedDelegate.isCurrent ? 2 : 8
                                                leftPadding: 8
                                                rightPadding: 8
                                                wrapMode: Text.WordWrap
                                            }

                                            VclLabel {
                                                Layout.fillWidth: true
                                                text: _includedDelegate.featureDescription
                                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                                font.italic: true
                                                topPadding: 0
                                                bottomPadding: 8
                                                leftPadding: 8
                                                rightPadding: 8
                                                wrapMode: Text.WordWrap
                                                visible: _includedDelegate.isCurrent && text !== ""
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Not included column
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Runtime.colors.primary.c100.background
                        border.color: Runtime.colors.primary.c400.background
                        border.width: 1
                        clip: true
                        visible: _excludedModel.count > 0

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: _excludedHeader.implicitHeight
                                color: Runtime.colors.primary.c600.background

                                VclLabel {
                                    id: _excludedHeader
                                    width: parent.width
                                    text: "✗   Not Included"
                                    color: Runtime.colors.primary.c600.text
                                    font.bold: true
                                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                    padding: 8
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                ListView {
                                    id: _excludedList

                                    anchors.fill: parent
                                    anchors.margins: 1
                                    anchors.topMargin: 0

                                    model: _excludedModel
                                    clip: true
                                    keyNavigationEnabled: true

                                    ScrollBar.vertical: VclScrollBar { }

                                    delegate: Item {
                                        id: _excludedDelegate
                                        required property int index
                                        required property string featureTitle
                                        required property string featureDescription

                                        readonly property bool isCurrent: ListView.isCurrentItem

                                        width: _excludedList.width
                                        height: _excludedDelegateLayout.implicitHeight

                                        Rectangle {
                                            anchors.fill: parent
                                            color: _excludedDelegate.isCurrent
                                                   ? Runtime.colors.primary.c300.background
                                                   : (_excludedDelegate.index % 2 === 0) ? Runtime.colors.primary.c50.background
                                                                                         : Runtime.colors.primary.c100.background
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                _excludedList.forceActiveFocus()
                                                _excludedList.currentIndex = _excludedDelegate.isCurrent ? -1 : _excludedDelegate.index
                                            }
                                        }

                                        ColumnLayout {
                                            id: _excludedDelegateLayout
                                            width: parent.width
                                            spacing: 0

                                            VclLabel {
                                                Layout.fillWidth: true
                                                text: "✗  " + _excludedDelegate.featureTitle
                                                font.bold: _excludedDelegate.isCurrent
                                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                                topPadding: 8
                                                bottomPadding: _excludedDelegate.isCurrent ? 2 : 8
                                                leftPadding: 8
                                                rightPadding: 8
                                                wrapMode: Text.WordWrap
                                            }

                                            VclLabel {
                                                Layout.fillWidth: true
                                                text: _excludedDelegate.featureDescription
                                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                                font.italic: true
                                                topPadding: 0
                                                bottomPadding: 8
                                                leftPadding: 8
                                                rightPadding: 8
                                                wrapMode: Text.WordWrap
                                                visible: _excludedDelegate.isCurrent && text !== ""
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

        bottomBar: RowLayout {
            width: parent.width
            height: implicitHeight + 16
            spacing: 20

            Item { Layout.fillWidth: true }

            VclButton {
                text: "Close"
                onClicked: _dialog.close()
            }

            Item { Layout.preferredWidth: 8 }
        }
    }
}
