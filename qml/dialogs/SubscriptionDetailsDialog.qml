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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

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
        id: dialog

        property var subscription

        width: Math.min(649,Scrite.window.width * 0.8)
        height: Math.min(650,Scrite.window.height * 0.8)
        title: "Subscription Details"

        content: Item {
            Component.onCompleted: SubscriptionPlanOperations.populateFeatureListTableModel(dialog.subscription, featureModel)

            ListModel {
                id: featureModel
            }

            Flickable {
                id: detailsView

                anchors.fill: parent

                clip: true

                contentWidth: width
                contentHeight: detailsViewLayout.height
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds

                ScrollBar.vertical: VclScrollBar {
                    flickable: detailsView
                }

                ColumnLayout {
                    id: detailsViewLayout

                    width: detailsView.width - 20
                    spacing: 10

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: dialog.subscription.plan.title
                        wrapMode: Text.WordWrap
                        color: Runtime.colors.accent.c600.background
                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                        horizontalAlignment: Text.AlignHCenter
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: dialog.subscription.plan.subtitle
                        font.italic: true
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    VclLabel {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: parent.width * 0.65

                        text: {
                            let ret = dialog.subscription.hasExpired ? "Was used for " : (dialog.subscription.isUpcoming ? "Will be valid for " : "Valid for ")
                            ret += "<b>" + Runtime.daysSpanAsString(dialog.subscription.plan.duration) + "</b>"
                            ret += " from <b>" + Runtime.formatDateIncludingYear(new Date(dialog.subscription.from)) + "</b> until <b>" + Runtime.formatDateIncludingYear(new Date(dialog.subscription.until)) + "</b>"
                            ret += ", with activation limit of <b>" + dialog.subscription.plan.devices + "</b> device(s)."
                            return ret
                        }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: {
                            let ret = "Status: "
                            if(dialog.subscription.isActive)
                                ret += "<b>Active</b>"
                            else if(dialog.subscription.isUpcoming)
                                ret += "Upcoming"
                            else if(dialog.subscription.hasExpired)
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

                        text: "Order #" + dialog.subscription.wc_order_id + " »"
                        visible: dialog.subscription.wc_order_id !== undefined && dialog.subscription.wc_order_id !== ""

                        onClicked: Qt.openUrlExternally(dialog.subscription.detailsUrl)
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                    }

                    GridLayout {
                        id: featureTable

                        Layout.alignment: Qt.AlignHCenter

                        columns: 2
                        rowSpacing: 0
                        columnSpacing: 0

                        Repeater {
                            model: featureModel

                            DelegateChooser {
                                role: "kind"

                                DelegateChoice {
                                    roleValue: "label"

                                    delegate: VclLabel {
                                        required property int index
                                        required property var attributes

                                        Layout.fillHeight: true
                                        Layout.minimumWidth: (index%2 === 0) ? 200 : 0
                                        Layout.maximumWidth: (index%2 === 0) ? 200 : 100
                                        Layout.preferredWidth: (index%2 === 0) ? 200 : 100

                                        text: attributes.text
                                        font.pointSize: attributes.font.pointSize
                                        font.weight: attributes.font.weight
                                        font.italic: attributes.font.italic
                                        background: Rectangle {
                                            color: attributes.background
                                        }
                                        color: attributes.color
                                        horizontalAlignment: attributes.horizontalAlignment
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: 8; rightPadding: 8
                                        topPadding: 6; bottomPadding: 6
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                DelegateChoice {
                                    roleValue: "labelWithTooltip"

                                    delegate: VclLabel {
                                        required property int index
                                        required property var attributes

                                        Layout.fillHeight: true
                                        Layout.minimumWidth: (index%2 === 0) ? 200 : 0
                                        Layout.maximumWidth: (index%2 === 0) ? 200 : 100
                                        Layout.preferredWidth: (index%2 === 0) ? 200 : 100

                                        text: attributes.text + " ⓘ"
                                        font.pointSize: attributes.font.pointSize
                                        font.weight: attributes.font.weight
                                        font.italic: attributes.font.italic
                                        background: Rectangle {
                                            color: attributes.background
                                        }
                                        color: attributes.color
                                        horizontalAlignment: attributes.horizontalAlignment
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

                                            text: attributes.tooltip
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
