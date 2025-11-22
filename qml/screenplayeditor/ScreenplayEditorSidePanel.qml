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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor/scenelistpanel"

Item {
    id: root

    required property bool readOnly
    required property ScreenplayAdapter screenplayAdapter

    property alias expanded: _sidePanel.expanded
    property alias minPanelWidth: _sidePanel.minPanelWidth
    property alias maxPanelWidth: _sidePanel.maxPanelWidth

    signal positionScreenplayEditorAtTitlePage()

    width: _sidePanel.width

    SidePanel {
        id: _sidePanel

        height: parent.height

        z: expanded ? 1 : 0

        label: ""
        buttonY: 20
        maxPanelWidth: Runtime.screenplayEditorSettings.sidePanelWidth

        content: root.screenplayAdapter.elementCount === 0 ? _private.emptyScreenplayContent : _private.filledScreenplayContent
    }

    QtObject {
        id: _private

        readonly property Component emptyScreenplayContent: Item {
            VclLabel {
                anchors.top: parent.top
                anchors.topMargin: 50
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width * 0.9

                text: "Scene headings will be listed here as you add them into your screenplay."
                visible: root.screenplayAdapter.elementCount === 0
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        readonly property Component filledScreenplayContent: FocusScope {
            EventFilter.target: Scrite.app
            EventFilter.active: root.screenplayAdapter.isSourceScreenplay && root.screenplayAdapter.screenplay.hasSelectedElements
            EventFilter.events: [EventFilter.KeyPress]
            EventFilter.onFilter: (object,event,result) => {
                                      if(event.key === Qt.Key_Escape) {
                                          root.screenplayAdapter.screenplay.clearSelection()
                                          result.acceptEvent = true
                                          result.filter = true
                                      }
                                  }

            ColumnLayout {
                anchors.fill: parent

                spacing: 0

                SceneListPanelHeader {
                    Layout.fillWidth: true

                    color: Runtime.colors.primary.c400.background

                    leftPadding: 10
                    rightPadding: 10

                    onClicked: {
                        if(root.screenplayAdapter.isSourceScreenplay)
                            root.screenplayAdapter.screenplay.clearSelection()
                        root.screenplayAdapter.currentIndex = -1

                        root.positionScreenplayEditorAtTitlePage()
                    }
                }

                Loader {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    sourceComponent: _sceneListPanel
                }
            }
        }
    }

    Component {
        id: _sceneListPanel

        RowLayout {
            spacing: 0

            ScreenplayTracksView {
                id: _screenplayTracksView

                Layout.fillHeight: true

                enabled: _delegateCount.get === root.screenplayAdapter.elementCount && (isSceneTextModeHeading ? 1 : maximumLineCount) === 1
                visible: Runtime.sceneListPanelSettings.displayTracks && Runtime.screenplayTracksSettings.displayTracks && root.screenplayAdapter.isSourceScreenplay
                listView: _sceneListView
                screenplay: root.screenplayAdapter.screenplay

                DelayedPropertyBinder {
                    id: _delegateCount

                    set: _sceneListView.delegateCount
                    initial: 0
                    delay: 0
                }

                property int maximumLineCount: Runtime.bounded(1,Runtime.screenplayEditorSettings.slpSynopsisLineCount,5)
                property bool isSceneTextModeHeading: Runtime.sceneListPanelSettings.sceneTextMode === "HEADING"
                onMaximumLineCountChanged: reload()
                onIsSceneTextModeHeadingChanged: reload()
            }

            SceneListPanel {
                id: _sceneListView

                Layout.fillWidth: true
                Layout.fillHeight: true

                readOnly: root.readOnly
                screenplayAdapter: root.screenplayAdapter
                tracksVisible: _screenplayTracksView.visible
            }
        }
    }

    Component {
        id: _sceneTreePanel

        Item {
            // TODO:
        }
    }
}
