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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"

MenuLoader {
    id: root

    required property int index
    required property ScreenplayAdapter screenplayAdapter
    required property ScreenplayElement screenplayElement

    property Scene scene: screenplayElement ? screenplayElement.scene : null

    menu: VclMenu {
        id: _sceneMenu

        VclMenuItem {
            enabled: !root.screenplayElement.omitted

            action: Action {
                text: "Scene Heading"
                checkable: true
                checked: root.scene.heading.enabled
            }

            onTriggered: {
                root.scene.heading.enabled = action.checked
                _sceneMenu.close()
            }
        }

        VclMenu {
            title: "Page Breaks"

            VclMenuItem {
                action: Action {
                    text: "Before"
                    checkable: true
                    checked: root.screenplayElement.pageBreakBefore
                }

                onTriggered: root.screenplayElement.pageBreakBefore = action.checked
            }

            VclMenuItem {
                action: Action {
                    text: "After"
                    checkable: true
                    checked: root.screenplayElement.pageBreakAfter
                }
                onTriggered: root.screenplayElement.pageBreakAfter = action.checked
            }
        }

        ColorMenu {
            title: "Color"

            onMenuItemClicked: {
                root.scene.color = color
                _sceneMenu.close()
            }
        }

        MarkSceneAsMenu {
            title: "Mark Scene As"
            scene: root.scene
            enabled: !root.screenplayElement.omitted
        }

        VclMenu {
            title: "Reports"

            width: 250

            Repeater {
                model: Runtime.sceneReports.reports ? Runtime.sceneReports.reports : 0

                delegate: VclMenuItem {
                    required property int index
                    required property var modelData

                    text: modelData.name
                    icon.source: "qrc" + modelData.icon

                    onTriggered: {
                        let sceneNumbers = Scrite.document.screenplay.selectedElementIndexes()
                        if(sceneNumbers.length > 0)
                            sceneNumbers.splice(0, sceneNumbers.length)
                        sceneNumbers.push(root.screenplayElement.elementIndex)

                        let props = {}
                        props[Runtime.sceneReports.propertyName] = sceneNumbers
                        ReportConfigurationDialog.launch(modelData.name, props, {initialPage: modelData.group})
                    }
                }
            }
        }

        MenuSeparator { }

        VclMenuItem {
            text: "Copy"
            enabled: root.screenplayAdapter.isSourceScreenplay

            onClicked: {
                Scrite.document.screenplay.clearSelection()
                root.screenplayElement.selected = true
                Scrite.document.screenplay.copySelection()
            }
        }

        VclMenuItem {
            text: "Paste After"
            enabled: root.screenplayAdapter.isSourceScreenplay && Scrite.document.screenplay.canPaste

            onClicked: Scrite.document.screenplay.pasteAfter( root.index )
        }

        MenuSeparator { }

        VclMenuItem {
            text: root.screenplayElement.omitted ? "Include" : "Omit"
            enabled: root.screenplayAdapter.isSourceScreenplay

            onClicked: {
                _sceneMenu.close()
                root.screenplayElement.omitted = !root.screenplayElement.omitted
            }
        }

        VclMenuItem {
            text: "Remove"
            enabled: root.screenplayAdapter.isSourceScreenplay

            onClicked: {
                _sceneMenu.close()
                Scrite.document.screenplay.removeSceneElements(root.scene)
            }
        }
    }
}
