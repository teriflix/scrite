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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"

Loader {
    id: root

    required property var pageMargins
    required property bool readOnly
    required property real zoomLevel
    required property ScreenplayAdapter screenplayAdapter

    z: 10 // So that the UiElementHightlight doesnt get clipped at the top-edge of the footer.

    sourceComponent: screenplayAdapter.isSourceScreenplay ? _private.footerButtonsComponent : undefined

    onItemChanged: Object.resetProperty(root, "height")

    QtObject {
        id: _private

        readonly property Component footerButtonsComponent: Item {
            id: _footerButtons

            height: _buttonsLayout.height + 2*_buttonsLayout.spacing * root.zoomLevel

            Component.onCompleted: {
                if(Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab &&
                  !Runtime.screenplayEditorSettings.screenplayEditorAddButtonsAnimationShown &&
                   root.screenplayAdapter.elementCount === 1) {
                    let highlight = _private.footerButtonsHighlighterComponent.createObject(_footerButtons, {"uiElement": _buttonsLayout})
                    highlight.done.connect(highlight.destroy)
                }
            }

            RowLayout {
                id: _buttonsLayout

                anchors.centerIn: parent

                enabled: !root.readOnly
                spacing: 20

                VclToolButton {
                    toolTipText: "Add Scene (Ctrl+Shift+N)"

                    icon.source: "qrc:/icons/action/add_scene.png"

                    onClicked: {
                        Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.lastSceneElementIndex()
                        if(!Scrite.document.readOnly)
                            Scrite.document.createNewScene(true)
                    }
                }

                VclToolButton {
                    toolTipText: "Add Act Break (Ctrl+Shift+B)"

                    icon.source: "qrc:/icons/action/add_act.png"

                    onClicked: Scrite.document.screenplay.addBreakElement(Screenplay.Act)
                }

                VclToolButton {
                    toolTipText: "Add Episode Break (Ctrl+Shift+P)"

                    icon.source: "qrc:/icons/action/add_episode.png"

                    onClicked: Scrite.document.screenplay.addBreakElement(Screenplay.Episode)
                }
            }
        }

        readonly property UiElementHighlight footerButtonsHighlighterComponent: UiElementHighlight {
            description: "Use these buttons to add new a scene, act or episode."
            uiElementBoxVisible: true
            descriptionPosition: Item.Bottom
            highlightAnimationEnabled: false

            Component.onCompleted: Runtime.screenplayEditorSettings.screenplayEditorAddButtonsAnimationShown = true
        }
    }
}
