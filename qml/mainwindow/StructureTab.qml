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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/"
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notifications"

Item {
    id: root

    SplitView {
        id: _mainSplit

        Material.background: _private.splitViewBackgroundColor

        anchors.fill: parent

        orientation: Qt.Vertical

        Item {
            id: _row1

            SplitView.fillHeight: true

            SplitView {
                id: _editorSplit

                Material.background: _private.splitViewBackgroundColor

                anchors.fill: parent

                orientation: Qt.Horizontal

                Item {
                    id: _col1

                    Rectangle {
                        anchors.fill: parent

                        color: Runtime.colors.primary.c10.background
                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor
                    }

                    RowLayout {
                        anchors.fill: parent

                        spacing: 0

                        Item {
                            // TODO: toolbar
                        }

                        Loader {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            // TODO: structure canvas or notebook
                        }
                    }
                }

                Item {
                    id: _col2

                    ScreenplayTab {
                        id: _screenplayEditor

                        anchors.fill: parent
                    }
                }
            }
        }


        Item {
            id: _row2

            SplitView.minimumHeight: 16
            SplitView.maximumHeight: _private.preferredTimelineHeight
            SplitView.preferredHeight: _private.preferredTimelineHeight

            TimelineView {
                id: _timeline

                anchors.fill: parent

                showNotesIcon: Runtime.showNotebookInStructure
            }
        }

    }

    QtObject {
        id: _private

        property real preferredTimelineHeight: 140 + Runtime.minimumFontMetrics.height*Runtime.screenplayTracks.trackCount
        property color splitViewBackgroundColor: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

        readonly property Component structureCanvas: StructureView {

        }

        readonly property Component notebook: NotebookView {

        }
    }
}
