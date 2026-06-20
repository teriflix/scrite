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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"
import "../commandcenter"

import "./useraccountdialog"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    name: "SubscriptionPlanPurchaseDialog"
    singleInstanceOnly: true

    function launch(plan) {
        doLaunch({"plan": plan})
    }

    dialogComponent: VclDialog {
        id: _dialog

        property var plan

        width: Math.min(680, Scrite.window.width * 0.85)
        height: Math.min(520, Scrite.window.height * 0.8)
        title: "Plan Details"
        titleBarCloseButtonVisible: true

        content: Item {
            Component.onCompleted: {
                if (!SubscriptionPlanOperations.taxonomy || !_dialog.plan)
                    return

                SubscriptionPlanOperations.taxonomy.features.forEach((feature) => {
                    if (feature.group === true) return
                    if (feature.display === false) return

                    const enabled = Scrite.isFeatureNameEnabled(feature.name, _dialog.plan.features)
                    if (enabled)
                        _includedModel.append({ "featureTitle": feature.title, "featureDescription": feature.description })
                    else
                        _excludedModel.append({ "featureTitle": feature.title, "featureDescription": feature.description })
                })
            }

            ListModel { id: _includedModel }
            ListModel { id: _excludedModel }

            ActionHandler {
                action: _dialog.acceptAction
                onTriggered: {
                    Qt.openUrlExternally(_dialog.plan.action.url)
                    _dialog.close()
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                // Plan summary — two columns: identity (left) | pricing (right)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    // Left: Title, subtitle, duration · devices, exclusive, featureNote
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        VclLabel {
                            Layout.fillWidth: true
                            text: _dialog.plan.title
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                            wrapMode: Text.WordWrap
                        }

                        VclLabel {
                            Layout.fillWidth: true
                            text: _dialog.plan.subtitle
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            visible: text !== ""
                            wrapMode: Text.WordWrap
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            VclLabel {
                                text: Runtime.daysSpanAsString(_dialog.plan.duration) + "  ·  " +
                                      "Device Count: " + _dialog.plan.devices
                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                            }

                            VclLabel {
                                text: "ⓘ"
                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        const n = _dialog.plan.devices
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

                            VclLabel {
                                text: "  ·  Discord community support only."
                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                visible: !Scrite.isFeatureNameEnabled("support/email", _dialog.plan.features)
                            }

                            VclLabel {
                                text: "ⓘ"
                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                visible: !Scrite.isFeatureNameEnabled("support/email", _dialog.plan.features)

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: JoinDiscordCommunity.launch()
                                }
                            }

                            Item { Layout.fillWidth: true }
                        }

                        VclLabel {
                            Layout.fillWidth: true
                            text: "★  Exclusive Plan"
                            font.bold: true
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                            visible: _dialog.plan.exclusive
                        }
                    }

                    // Right: Pricing block
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            text: Scrite.currencySymbol(_dialog.plan.pricing.currency) + _dialog.plan.pricing.actual
                            font.family: Runtime.shortcutFontMetrics.font.family
                            font.strikeout: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                            opacity: 0.5
                            visible: _dialog.plan.pricing.actual > 0 && _dialog.plan.pricing.actual > _dialog.plan.pricing.price
                        }

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            text: _dialog.plan.pricing.price === 0
                                  ? "FREE"
                                  : (Scrite.currencySymbol(_dialog.plan.pricing.currency) + _dialog.plan.pricing.price)
                            font.family: Runtime.shortcutFontMetrics.font.family
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 10
                        }

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            text: {
                                const p = _dialog.plan.pricing
                                const pct = Math.round((1 - p.price / p.actual) * 100)
                                const saved = Math.round(p.actual - p.price)
                                return Scrite.currencySymbol(p.currency) + saved + " off - " + pct  + "% discount"
                            }
                            font.family: Runtime.shortcutFontMetrics.font.family
                            font.bold: true
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                            visible: _dialog.plan.pricing.actual > 0 && _dialog.plan.pricing.actual > _dialog.plan.pricing.price
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

                    FeatureListPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        headerText: "✓   Included"
                        headerBgColor: Runtime.colors.tx(Runtime.colors.accent.c600.background)
                        headerTextColor: Runtime.colors.accent.c600.text
                        headerBorderWidth: 1
                        listModel: _includedModel
                        highlightColor: Runtime.colors.accent.c200.background
                    }

                    FeatureListPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        headerText: "✗   Not Included"
                        headerBgColor: Runtime.colors.primary.c600.background
                        headerTextColor: Runtime.colors.primary.c600.text
                        listModel: _excludedModel
                        highlightColor: Runtime.colors.primary.c300.background
                        titlePrefix: "✗  "
                        visible: _excludedModel.count > 0
                    }
                }
            }
        }

        bottomBar: RowLayout {
            width: parent.width
            height: implicitHeight + 16

            spacing: 20

            VclLabel {
                Layout.fillWidth: true
                Layout.leftMargin: 20

                text: "* Plan availability and prices are subject to change"
                wrapMode: Text.WordWrap
                visible: _dialog.plan && _dialog.plan.pricing.price > 0
            }

            VclButton {
                text: "Buy »"
                onClicked: _dialog.acceptAction.trigger()
            }

            Item { Layout.preferredWidth: 8 }
        }
    }
}
