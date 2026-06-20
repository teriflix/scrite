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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../controls"

Rectangle {
    id: _panel

    property string headerText
    property color headerBgColor
    property color headerTextColor
    property int headerBorderWidth: 0
    property ListModel listModel
    property color highlightColor
    property string titlePrefix: ""
    property alias listInteractive: _panelListView.interactive

    color: Runtime.colors.primary.c100.background
    border.color: Runtime.colors.primary.c400.background
    border.width: 1
    clip: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: _panelHeader.implicitHeight

            color: _panel.headerBgColor

            border.color: Runtime.colors.primary.c400.background
            border.width: _panel.headerBorderWidth

            VclLabel {
                id: _panelHeader

                width: parent.width
                text: _panel.headerText
                color: _panel.headerTextColor
                padding: 8

                font.bold: true
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: _panelListView

                property color highlightColor: _panel.highlightColor
                property string titlePrefix: _panel.titlePrefix

                ScrollBar.vertical: VclScrollBar { }

                anchors.fill: parent
                anchors.margins: 1
                anchors.topMargin: 0

                model: _panel.listModel

                clip: true
                interactive: true
                keyNavigationEnabled: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0

                delegate: Item {
                    id: _delegate
                    required property int index
                    required property string featureTitle
                    required property string featureDescription
                    readonly property bool isCurrent: ListView.isCurrentItem
                    readonly property var listView: ListView.view

                    width: listView ? listView.width : 0
                    height: _delegateLayout.implicitHeight

                    Rectangle {
                        anchors.fill: parent

                        color: _delegate.isCurrent
                               ? _delegate.listView.highlightColor
                               : (_delegate.index % 2 === 0)
                                 ? Runtime.colors.primary.c50.background
                                 : Runtime.colors.primary.c100.background
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            _delegate.listView.forceActiveFocus()
                            _delegate.listView.interactive = true
                            _delegate.listView.currentIndex = _delegate.isCurrent ? -1 : _delegate.index
                        }
                    }

                    ColumnLayout {
                        id: _delegateLayout

                        width: parent.width
                        spacing: 0

                        VclLabel {
                            Layout.fillWidth: true

                            text: _delegate.listView.titlePrefix + _delegate.featureTitle
                            wrapMode: Text.WordWrap

                            topPadding: 8
                            bottomPadding: _delegate.isCurrent ? 2 : 8
                            leftPadding: 8
                            rightPadding: 8

                            font.bold: _delegate.isCurrent
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            text: _delegate.featureDescription
                            visible: _delegate.isCurrent && text !== ""
                            wrapMode: Text.WordWrap

                            topPadding: 0
                            bottomPadding: 8
                            leftPadding: 8
                            rightPadding: 8

                            font.italic: true
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        }
                    }
                }
            }
        }
    }
}
