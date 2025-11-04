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

    name: "SubscriptionPlanComparisonDialog"
    singleInstanceOnly: true

    function init() { }
    function launch(plans, title) {
        if(_private.taxonomy === undefined) {
            MessageBox.information("Error", "Unable to show plan details at this point.")
            return
        }

        doLaunch({
            "plans": plans,
            "title": title ? title : "Subscription Plans"
        })
    }

    dialogComponent: VclDialog {
        id: dialog

        width: Math.max(500, Math.min( Math.min(_private.maxDialogWidth,_private.idealColumnSize*(plans.length+1)), Scrite.window.width * 0.8))
        height: Math.min(_private.maxDialogHeight,Scrite.window.height * 0.8)
        title: "Subscription Plans"

        property var plans: []
        titleBarCloseButtonVisible: true

        content: Item {
            Component.onCompleted: SubscriptionPlanOperations.populateComparisonTableModel(dialog.plans, comparisonTableModel)

            Rectangle {
                anchors.fill: parent

                color: Runtime.colors.accent.c600.background
            }

            ListModel {
                id: comparisonTableModel
            }

            Flickable {
                id: comparisionTableView

                anchors.fill: parent

                clip: true

                contentWidth: comparisonTable.width
                contentHeight: comparisonTable.height
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds

                ScrollBar.vertical: VclScrollBar {
                    id: verticialScrollBar
                    flickable: comparisionTableView
                }
                ScrollBar.horizontal: VclScrollBar {
                    flickable: verticialScrollBar
                }

                GridLayout {
                    id: comparisonTable

                    columns: 1 + dialog.plans.length
                    rowSpacing: 0
                    columnSpacing: -1

                    property real idealColumnWidth: (comparisionTableView.width/comparisonTable.columns)
                    property real columnWidth: Math.max(_private.idealColumnSize, idealColumnWidth)

                    Repeater {
                        model: comparisonTableModel

                        DelegateChooser {
                            role: "kind"

                            DelegateChoice {
                                roleValue: "label"

                                VclLabel {
                                    required property var attributes

                                    Layout.fillHeight: true
                                    Layout.minimumWidth: comparisonTable.columnWidth
                                    Layout.maximumWidth: comparisonTable.columnWidth
                                    Layout.preferredWidth: comparisonTable.columnWidth

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

                                VclLabel {
                                    required property var attributes

                                    Layout.fillHeight: true
                                    Layout.minimumWidth: comparisonTable.columnWidth
                                    Layout.maximumWidth: comparisonTable.columnWidth
                                    Layout.preferredWidth: comparisonTable.columnWidth

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
                                        ToolTip.text: attributes.tooltip
                                        ToolTip.visible: containsMouse

                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }
                                }
                            }

                            DelegateChoice {
                                roleValue: "link"

                                Link {
                                    required property var attributes

                                    Layout.fillHeight: true
                                    Layout.minimumWidth: comparisonTable.columnWidth
                                    Layout.maximumWidth: comparisonTable.columnWidth
                                    Layout.preferredWidth: comparisonTable.columnWidth

                                    text: attributes.text
                                    padding: 8
                                    font.bold: true
                                    background: Rectangle {
                                        color: attributes.background
                                    }
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter

                                    onClicked: {
                                        SubscriptionPlanOperations.subscribeTo(attributes.plan)
                                        dialog.close()
                                    }
                                }
                            }
                        }
                    }

                    VclLabel {
                        Layout.fillWidth: true
                        Layout.columnSpan: comparisonTable.columns

                        padding: 12
                        color: Runtime.colors.accent.c600.text
                        horizontalAlignment: Text.AlignHCenter
                        text: "* All prices are subject to change without notice."
                    }
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property real idealColumnSize: 180
        readonly property real maxDialogWidth: 900
        readonly property real maxDialogHeight: 650

        property var taxonomy: SubscriptionPlanOperations.taxonomy
    }
}
