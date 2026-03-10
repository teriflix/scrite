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

pragma Singleton

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
        id: _dialog

        width: Math.max(500, Math.min( Math.min(_private.maxDialogWidth,_private.idealColumnSize*(plans.length+1)), Scrite.window.width * 0.8))
        height: Math.min(_private.maxDialogHeight,Scrite.window.height * 0.8)
        title: "Subscription Plans"

        property var plans: []
        titleBarCloseButtonVisible: true

        content: Item {
            Component.onCompleted: SubscriptionPlanOperations.populateComparisonTableModel(_dialog.plans, _comparisonTableModel)

            Rectangle {
                anchors.fill: parent

                color: Runtime.colors.accent.c600.background
            }

            ListModel {
                id: _comparisonTableModel
            }

            Flickable {
                id: _comparisionTableView

                anchors.fill: parent

                clip: true

                contentWidth: _comparisonTable.width
                contentHeight: _comparisonTable.height
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds

                ScrollBar.vertical: VclScrollBar {
                    id: _verticialScrollBar
                    flickable: _comparisionTableView
                }
                ScrollBar.horizontal: VclScrollBar {
                    flickable: _verticialScrollBar
                }

                GridLayout {
                    id: _comparisonTable

                    columns: 1 + _dialog.plans.length
                    rowSpacing: 0
                    columnSpacing: -1

                    property real idealColumnWidth: (_comparisionTableView.width/_comparisonTable.columns)
                    property real columnWidth: Math.max(_private.idealColumnSize, idealColumnWidth)

                    Repeater {
                        model: _comparisonTableModel

                        DelegateChooser {
                            role: "kind"

                            DelegateChoice {
                                roleValue: "label"

                                delegate: VclLabel {
                                    required property int index
                                    required property var attributes

                                    Layout.fillHeight: true
                                    Layout.minimumWidth: _comparisonTable.columnWidth
                                    Layout.maximumWidth: _comparisonTable.columnWidth
                                    Layout.preferredWidth: _comparisonTable.columnWidth

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
                                    Layout.minimumWidth: _comparisonTable.columnWidth
                                    Layout.maximumWidth: _comparisonTable.columnWidth
                                    Layout.preferredWidth: _comparisonTable.columnWidth

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
                                        id: _comparisonLabelMouseArea

                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }

                                    ToolTipPopup {
                                        container: _comparisonLabelMouseArea

                                        text: attributes.tooltip
                                        visible: _comparisonLabelMouseArea.containsMouse
                                    }
                                }
                            }

                            DelegateChoice {
                                roleValue: "link"

                                Link {
                                    required property var attributes

                                    Layout.fillHeight: true
                                    Layout.minimumWidth: _comparisonTable.columnWidth
                                    Layout.maximumWidth: _comparisonTable.columnWidth
                                    Layout.preferredWidth: _comparisonTable.columnWidth

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
                                        _dialog.close()
                                    }
                                }
                            }
                        }
                    }

                    VclLabel {
                        Layout.fillWidth: true
                        Layout.columnSpan: _comparisonTable.columns

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
