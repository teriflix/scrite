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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"

Item {
    id: root

    enum Tab { CommentsTab, FeaturedImageTab, IndexCardFieldsTab, SceneMetaDataTab }

    required property Scene scene
    required property color downIndicatorColor

    property alias currentTab: _private.currentTab

    function cycleTab() { _private.cycleTab() }

    height: _layout.height

    Column {
        id: _layout

        width: parent.width

        spacing: 1

        Repeater {
            model: _private.buttonsModel

            FlatToolButton {
                required property var modelData

                toolTipText: modelData.toolTip
                suggestedWidth: _layout.width
                suggestedHeight: _layout.width

                down: _private.currentTab === modelData.tab
                visible: modelData.isVisible
                iconSource: down ? modelData.invertedIcon : modelData.normalIcon
                downIndicatorColor: root.downIndicatorColor

                onClicked: _private.currentTab = modelData.tab

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
                "normalIcon": "qrc:/icons/content/comments_panel.png",
                "invertedIcon": "qrc:/icons/content/comments_panel_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.CommentsTab
            },
            {
                "tab": ExpandedCorner.Tab.FeaturedImageTab,
                "toolTip": "View/edit featured image.",
                "isVisible": true,
                "normalIcon": "qrc:/icons/filetype/photo.png",
                "invertedIcon": "qrc:/icons/filetype/photo_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.FeaturedImageTab
            },
            {
                "tab": ExpandedCorner.Tab.IndexCardFieldsTab,
                "toolTip":  "View/edit index card fields.",
                "isVisible": Runtime.screenplayEditorSettings.displayIndexCardFields,
                "normalIcon": "qrc:/icons/content/form.png",
                "invertedIcon": "qrc:/icons/content/form_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.CommentsTab
            },
            {
                "tab": ExpandedCorner.Tab.SceneMetaDataTab,
                "toolTip":  "View/edit scene meta data.",
                "isVisible": !Runtime.screenplayEditorSettings.displaySceneSynopsis || !Runtime.screenplayEditorSettings.displaySceneCharacters,
                "normalIcon": "qrc:/icons/action/description.png",
                "invertedIcon": "qrc:/icons/action/description_inverted.png",
                "tabWhenNotVisible": ExpandedCorner.Tab.CommentsTab
            }
        ]

        function cycleTab() {
            const buttons = buttonsModel
            let tab = (currentTab+1)%buttons.length
            while(1) {
                if(buttons[tab].isVisible) {
                    currentTab = buttons[tab].tab
                    return
                }
                tab = (tab+1)%buttons.length
            }
        }
    }
}
