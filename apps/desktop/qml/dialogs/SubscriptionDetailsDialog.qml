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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.qmlmodels

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"

// TODO: Needs to be reviewed and tested.

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

        property var subscription

        width: Math.min(649,Scrite.window.width * 0.8)
        height: Math.min(650,Scrite.window.height * 0.8)
        title: "Subscription Details"

        content: Item {
            Component.onCompleted: SubscriptionPlanOperations.populateFeatureListTableModel(_dialog.subscription, _featureModel)

            ListModel {
                id: _featureModel
            }

            Flickable {
                id: _detailsView

                anchors.fill: parent

                clip: true

                contentWidth: width
                contentHeight: _detailsViewLayout.height
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds

                ScrollBar.vertical: VclScrollBar {
                    flickable: _detailsView
                }

                ColumnLayout {
                    id: _detailsViewLayout

                    width: _detailsView.width - 20
                    spacing: 10

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: _dialog.subscription.plan.title
                        wrapMode: Text.WordWrap
                        color: Runtime.colors.accent.c600.background
                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                        horizontalAlignment: Text.AlignHCenter
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: _dialog.subscription.plan.subtitle
                        font.italic: true
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    VclLabel {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: parent.width * 0.65

                        text: {
                            let ret = _dialog.subscription.hasExpired ? "Was used for " : (_dialog.subscription.isUpcoming ? "Will be valid for " : "Valid for ")
                            ret += "<b>" + Runtime.daysSpanAsString(_dialog.subscription.plan.duration) + "</b>"
                            ret += " from <b>" + Runtime.formatDateIncludingYear(new Date(_dialog.subscription.from)) + "</b> until <b>" + Runtime.formatDateIncludingYear(new Date(_dialog.subscription.until)) + "</b>"
                            ret += ", with activation limit of <b>" + _dialog.subscription.plan.devices + "</b> device(s)."
                            return ret
                        }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: {
                            let ret = "Status: "
                            if(_dialog.subscription.isActive)
                                ret += "<b>Active</b>"
                            else if(_dialog.subscription.isUpcoming)
                                ret += "Upcoming"
                            else if(_dialog.subscription.hasExpired)
                                ret += "Expired"
                            else
                                ret += "Unknown"

                            return ret
                        }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Link {
                        Layout.alignment: Qt.AlignHCenter

                        text: "Order #" + _dialog.subscription.wc_order_id + " »"
                        visible: _dialog.subscription.wc_order_id !== undefined && _dialog.subscription.wc_order_id !== ""

                        onClicked: Qt.openUrlExternally(_dialog.subscription.detailsUrl)
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                    }

                    GridLayout {
                        id: _featureTable

                        Layout.alignment: Qt.AlignHCenter

                        columns: 2
                        rowSpacing: 0
                        columnSpacing: 0

                        Repeater {
                            model: _featureModel

                            DelegateChooser {
                                role: "kind"

                                DelegateChoice {
                                    roleValue: "label"

                                    delegate: VclLabel {
                                        id: _labelDelegate
                                        required property int index
                                        required property var attributes

                                        Layout.fillHeight: true
                                        Layout.minimumWidth: (_labelDelegate.index%2 === 0) ? 200 : 0
                                        Layout.maximumWidth: (_labelDelegate.index%2 === 0) ? 200 : 100
                                        Layout.preferredWidth: (_labelDelegate.index%2 === 0) ? 200 : 100

                                        text: _labelDelegate.attributes.text
                                        font.pointSize: _labelDelegate.attributes.font.pointSize
                                        font.weight: _labelDelegate.attributes.font.weight
                                        font.italic: _labelDelegate.attributes.font.italic
                                        background: Rectangle {
                                            color: _labelDelegate.attributes.background
                                        }
                                        color: _labelDelegate.attributes.color
                                        horizontalAlignment: _labelDelegate.attributes.horizontalAlignment
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: 8; rightPadding: 8
                                        topPadding: 6; bottomPadding: 6
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                DelegateChoice {
                                    roleValue: "labelWithTooltip"

                                    delegate: VclLabel {
                                        id: _labelWithTooltipDelegate
                                        required property int index
                                        required property var attributes

                                        Layout.fillHeight: true
                                        Layout.minimumWidth: (_labelWithTooltipDelegate.index%2 === 0) ? 200 : 0
                                        Layout.maximumWidth: (_labelWithTooltipDelegate.index%2 === 0) ? 200 : 100
                                        Layout.preferredWidth: (_labelWithTooltipDelegate.index%2 === 0) ? 200 : 100

                                        text: _labelWithTooltipDelegate.attributes.text + " ⓘ"
                                        font.pointSize: _labelWithTooltipDelegate.attributes.font.pointSize
                                        font.weight: _labelWithTooltipDelegate.attributes.font.weight
                                        font.italic: _labelWithTooltipDelegate.attributes.font.italic
                                        background: Rectangle {
                                            color: _labelWithTooltipDelegate.attributes.background
                                        }
                                        color: _labelWithTooltipDelegate.attributes.color
                                        horizontalAlignment: _labelWithTooltipDelegate.attributes.horizontalAlignment
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: 8; rightPadding: 8
                                        topPadding: 6; bottomPadding: 6
                                        wrapMode: Text.WordWrap

                                        MouseArea {
                                            id: _featureLabelMouseArea

                                            anchors.fill: parent
                                            hoverEnabled: true
                                        }

                                        ToolTipPopup {
                                            container: _featureLabelMouseArea

                                            text: _labelWithTooltipDelegate.attributes.tooltip
                                            visible: _featureLabelMouseArea.containsMouse
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                    }
                }
            }
        }
    }
}
