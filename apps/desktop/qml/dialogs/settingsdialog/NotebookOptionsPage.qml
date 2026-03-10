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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../controls"

Item {
    id: root

    height: _layout.height + 2*_layout.margin

    ColumnLayout {
        id: _layout

        readonly property real margin: 10

        width: parent.width-2*margin
        y: 10

        spacing: 10

        GroupBox {
            Layout.fillWidth: true

            label: VclLabel {
                text: "Text Notes"
            }

            ColumnLayout {
                width: parent.width

                TextArea {
                    Layout.fillWidth: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    wrapMode: Text.WordWrap
                    textFormat: TextArea.RichText
                    readOnly: true
                    background: Item { }
                    text: "If you are unable to open scene, story or character notes, then uncheck this option. Scrite uses a web-based text editor for accepting rich formatted text notes. However, on some computers this may fail to launch causing Scrite to crash. In such cases unchecking the option below allows you to capture plain text notes."
                }

                VclCheckBox {
                    text: "Use Rich Text Notes"
                    checked: Runtime.notebookSettings.richTextNotesEnabled
                    onToggled: Runtime.notebookSettings.richTextNotesEnabled = checked
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclLabel {
                text: "Relationship Graph"
            }

            ColumnLayout {
                width: parent.width

                spacing: 10

                TextArea {
                    Layout.fillWidth: true

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    wrapMode: Text.WordWrap
                    textFormat: TextArea.RichText
                    readOnly: true
                    background: Item { }
                    text: "<p>Relationship graphs are automatically constructed using the Force Directed Graph algorithm. You can configure attributes of the algorithm using the fields below. The default values work for most cases.</p>" +
                          "<font size=\"-1\"><ul><li><strong>Max Time</strong> is the number of milliseconds the algorithm can take to compute the graph.</li><li><strong>Max Iterations</strong> is the number of times within max-time the graph can go over each character to determine the ideal placement of nodes and edges in the graph.</li></ul></font>"
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: parent.spacing

                    ColumnLayout {
                        Layout.preferredWidth: (parent.width-parent.spacing)/2

                        VclLabel {
                            Layout.fillWidth: true

                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            text: "Max Time In Milliseconds"
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            font.bold: false
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                            text: "Default: 1000"
                        }

                        TextField {
                            id: _txtMaxTime
                            Layout.fillWidth: true

                            text: Runtime.notebookSettings.graphLayoutMaxTime
                            placeholderText: "if left empty, default of 1000 will be used"
                            validator: IntValidator {
                                bottom: 250
                                top: 5000
                            }
                            onTextEdited: {
                                if(length === 0 || text.trim() === "")
                                    Runtime.notebookSettings.graphLayoutMaxTime = 1000
                                else
                                    Runtime.notebookSettings.graphLayoutMaxTime = parseInt(text)
                            }
                            KeyNavigation.tab: _txtMaxIterations
                        }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: (parent.width-parent.spacing)/2

                        VclLabel {
                            Layout.fillWidth: true

                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            text: "Max Iterations"
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            font.bold: false
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                            text: "Default: 50000"
                        }

                        TextField {
                            id: _txtMaxIterations
                            Layout.fillWidth: true

                            text: Runtime.notebookSettings.graphLayoutMaxIterations
                            placeholderText: "if left empty, default of 50000 will be used"
                            validator: IntValidator {
                                bottom: 1000
                                top: 250000
                            }
                            onTextEdited: {
                                if(length === 0 || text.trim() === "")
                                    Runtime.notebookSettings.graphLayoutMaxIterations = 50000
                                else
                                    Runtime.notebookSettings.graphLayoutMaxIterations = parseInt(text)
                            }
                            KeyNavigation.tab: _txtMaxTime
                        }
                    }
                }
            }
        }
    }
}
