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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    property int currentTab: -1
    property var tabs: []
    property string name: "Tabs"
    property alias spacing: _tabsRow.spacing
    property alias switchTabHandlerEnabled: _switchTabActionHandler.enabled

    height: implicitHeight
    implicitHeight: _tabsRow.height + Runtime.idealFontMetrics.descent + _currentTabUnderline.height

    ActionHandler {
        id: _switchTabActionHandler

        property int nextIndex: (root.currentTab+1)%_tabsRepeater.count
        property string text: "Switch to <b>" + _tabsRepeater.itemAt(nextIndex).text + "</b> tab in <i>" + root.name + "</i>"

        enabled: false
        action: ActionHub.applicationOptions.find("tabRight")
        onTriggered: root.currentTab = (root.currentTab+1)%_tabsRepeater.count
    }

    ActionHandler {
        property int previousIndex: (root.currentTab-1) < 0 ? _tabsRepeater.count-1 : root.currentTab-1
        property string text: "Switch to <b>" + _tabsRepeater.itemAt(previousIndex).text + "</b> tab in <i>" + root.name + "</i>"

        enabled: _switchTabActionHandler.enabled
        action: ActionHub.applicationOptions.find("tabLeft")
        onTriggered: root.currentTab =previousIndex
    }

    Row {
        id: _tabsRow

        width: parent.width

        spacing: 16

        VclLabel {
            id: _nameText

            rightPadding: 10

            text: name + ": "

            font.bold: true
            font.family: Runtime.idealFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            font.capitalization: Font.AllUppercase
        }

        Repeater {
            id: _tabsRepeater

            model: tabs

            VclLabel {
                color: root.currentTab === index ? Runtime.colors.accent.c900.background : Runtime.colors.primary.c700.background
                font: Runtime.idealFontMetrics.font
                text: modelData

                MouseArea {
                    anchors.fill: parent

                    onClicked: root.currentTab = index
                }
            }
        }
    }

    ItemPositionMapper {
        id: _currentTabItemPositionMapper

        from: _tabsRepeater.count > 0 ? _tabsRepeater.itemAt(root.currentTab) : null
        to: root

        onMappedPositionChanged: Qt.callLater( function() { _currentTabUnderline.placedOnce = true } )
    }

    Rectangle {
        id: _currentTabUnderline

        property bool placedOnce: false

        anchors.top: _tabsRow.bottom
        anchors.topMargin: Runtime.idealFontMetrics.descent

        height: 2
        width: _currentTabItemPositionMapper.from.width

        x: _currentTabItemPositionMapper.mappedPosition.x

        color: Runtime.colors.accent.c900.background

        Behavior on x {
            enabled: _currentTabUnderline.placedOnce && Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: 100 }
        }

        Behavior on width {
            enabled: _currentTabUnderline.placedOnce && Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: 100 }
        }
    }
}

