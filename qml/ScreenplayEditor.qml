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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/floatingdockpanels"

Rectangle {
    id: root

    property alias sidePanelEnabled: _sidePanelLoader.active

    color: Runtime.colors.primary.windowColor

    Loader {
        id: _sidePanelLoader

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.topMargin: 5
        anchors.bottomMargin: 5

        active: Runtime.mainWindowTab === Runtime.e_ScreenplayTab

        sourceComponent: ScreenplayEditorSidePanel {
            readOnly: Scrite.document.readOnly
            screenplayAdapter: Runtime.screenplayAdapter

            onPositionScreenplayEditorAtTitlePage: _listView.positionViewAtBeginning()
        }
    }

    Item {
        id: _workspace

        anchors.top: parent.top
        anchors.left: _sidePanelLoader.right
        anchors.right: parent.right
        anchors.bottom: _statusBar.top

        clip: true

        RulerItem {
            id: _ruler

            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter

            width: _private.pageLayout.paperWidth * _private.zoomLevel * _private.dpi
            height: Runtime.minimumFontMetrics.lineSpacing

            visible: Runtime.screenplayEditorSettings.displayRuler
            zoomLevel: _private.zoomLevel
            resolution: _private.pageLayout.resolution
            leftMargin: _private.pageMargins.left
            rightMargin: _private.pageMargins.right

            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        }

        Rectangle {
            anchors.fill: _listView

            color: Runtime.colors.primary.c50.background
        }

        ScreenplayElementListView {
            id: _listView

            ScrollBar.vertical: _scrollBar

            anchors.top: _ruler.bottom
            anchors.left: _ruler.left
            anchors.right: _ruler.right
            anchors.bottom: parent.bottom

            readOnly: Scrite.document.readOnly
            zoomLevel: _private.zoomLevel
            pageMargins: _private.pageMargins
            screenplayAdapter: Runtime.screenplayAdapter
        }

        VclScrollBar {
            id: _scrollBar

            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            flickable: _listView
        }
    }

    ScreenplayEditorStatusBar {
        id: _statusBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        pageMargins: _private.pageMargins
        sceneHeadingFontMetrics: _private.sceneHeadingFontMetrics
        screenplayEditorListView: _listView
        screenplayEditorLastItemIndex: 0
        screenplayEditorFirstItemIndex: 0
    }

    QtObject {
        id: _private


        property ScreenplayFormat screenplayFormat: Scrite.document.displayFormat
        property ScreenplayPageLayout pageLayout: screenplayFormat.pageLayout

        property SceneElementFormat sceneHeadingFormat: screenplayFormat.elementFormat(SceneElement.Heading)
        property FontMetrics sceneHeadingFontMetrics: FontMetrics {
            font: _private.sceneHeadingFormat.font2
        }

        property var pageMargins: Utils.margins( _ruler.zoomLevel * dpi * pageLayout.leftMargin,
                                                 _ruler.zoomLevel * dpi * pageLayout.topMargin,
                                                 _ruler.zoomLevel * dpi * pageLayout.rightMargin,
                                                 _ruler.zoomLevel * dpi * pageLayout.bottomMargin )

        property real dpi: Screen.devicePixelRatio
        property alias zoomLevel: _statusBar.zoomLevel
    }
}
