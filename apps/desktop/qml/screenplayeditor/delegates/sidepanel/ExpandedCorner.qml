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

import QtQml
import QtQuick
import QtQuick.Controls

import io.scrite.components

import "../../../globals"
import "../../../helpers"

Item {
    id: root

    enum Tab { CommentsTab, FeaturedImageTab, IndexCardFieldsTab, SceneMetaDataTab }

    required property Scene scene
    required property color downIndicatorColor

    property alias currentTab: _private.currentTab

    signal currentTabEdited()

    function cycleTab() { _private.cycleTab() }

    height: _layout.height

    Column {
        id: _layout

        width: parent.width

        spacing: 1

        Repeater {
            model: _private.buttonsModel

            delegate: FlatToolButton {
                required property int index
                required property var modelData

                toolTipText: modelData.toolTip
                suggestedWidth: _layout.width
                suggestedHeight: _layout.width

                down: _private.currentTab === modelData.tab
                visible: modelData.isVisible
                iconSource: down ? modelData.invertedIcon : modelData.normalIcon
                downIndicatorColor: root.downIndicatorColor

                onClicked: {
                    _private.currentTab = modelData.tab
                    root.currentTabEdited()
                }

                onVisibleChanged: {
                    if(!visible && _private.currentTab === modelData.tab)
                        _private.currentTab = modelData.tabWhenNotVisible
                }
            }
        }
    }

    QtObject {
        id: _private

        property int currentTab: ExpandedCorner.Tab.CommentsTab

        property var buttonsModel: [
            {
                "tab": ExpandedCorner.Tab.CommentsTab,
                "toolTip": "View/edit scene comments.",
                "isVisible": true,
                "normalIcon": Runtime.themedIcon("qrc:/icons/content/comments_panel.png"),
                "invertedIcon": "qrc:/icons/content/comments_panel_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.CommentsTab
            },
            {
                "tab": ExpandedCorner.Tab.FeaturedImageTab,
                "toolTip": "View/edit featured image.",
                "isVisible": true,
                "normalIcon": Runtime.themedIcon("qrc:/icons/filetype/photo.png"),
                "invertedIcon": "qrc:/icons/filetype/photo_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.FeaturedImageTab
            },
            {
                "tab": ExpandedCorner.Tab.IndexCardFieldsTab,
                "toolTip":  "View/edit index card fields.",
                "isVisible": Runtime.screenplayEditorSettings.displayIndexCardFields,
                "normalIcon": Runtime.themedIcon("qrc:/icons/content/form.png"),
                "invertedIcon": "qrc:/icons/content/form_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.CommentsTab
            },
            {
                "tab": ExpandedCorner.Tab.SceneMetaDataTab,
                "toolTip":  "View/edit scene meta data.",
                "isVisible": !Runtime.screenplayEditorSettings.displaySceneSynopsis || !Runtime.screenplayEditorSettings.displaySceneCharacters,
                "normalIcon": Runtime.themedIcon("qrc:/icons/action/description.png"),
                "invertedIcon": "qrc:/icons/action/description_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.CommentsTab
            }
        ]

        function cycleTab() {
            const buttons = buttonsModel
            let tab = (currentTab+1)%buttons.length
            let ct = currentTab
            while(1) {
                if(buttons[tab].isVisible) {
                    ct = buttons[tab].tab
                    if(ct !== currentTab) {
                        currentTab = ct
                        root.currentTabEdited()
                    }
                    return
                }
                tab = (tab+1)%buttons.length
            }
        }
    }
}
