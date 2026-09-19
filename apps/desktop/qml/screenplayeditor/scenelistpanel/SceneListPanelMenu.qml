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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App


import "../../globals"
import "../../dialogs"
import "../../helpers"
import "../../controls"

SctMenu {
    id: root

    SctMenu {
        title: "Text"

        SctMenuItem {
            text: "Heading"

            readonly property string option: "HEADING"

            icon.source: Runtime.sceneListPanelSettings.sceneTextMode === option ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.sceneTextMode = option
        }

        SctMenuItem {
            text: "Summary"

            readonly property string option: "SUMMARY"

            icon.source: Runtime.sceneListPanelSettings.sceneTextMode === option ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.sceneTextMode = option
        }

        MenuSeparator { }

        SctMenuItem {
            text: "Show Tooltip"

            icon.source: Runtime.sceneListPanelSettings.showTooltip ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.showTooltip = !Runtime.sceneListPanelSettings.showTooltip
        }
    }

    SctMenu {
        title: "Length"

        SctMenuItem {
            text: "Scene Duration"

            readonly property string option: "TIME"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }

        SctMenuItem {
            text: "Page Length"

            readonly property string option: "PAGE"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }

        SctMenuItem {
            text: "1/8th Length"

            readonly property string option: "PAGE_1_8"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }

        SctMenuItem {
            text: "None"

            readonly property string option: "NO"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }
    }

    SctMenu {
        title: "Tracks"

        SctMenuItem {
            text: "Display"

            enabled: Runtime.appFeatures.structure.enabled && Runtime.screenplayTracksSettings.displayTracks
            icon.source: Runtime.sceneListPanelSettings.displayTracks ? Runtime.themedIcon("qrc:/icons/navigation/check.png") : Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: Runtime.sceneListPanelSettings.displayTracks = !Runtime.sceneListPanelSettings.displayTracks
        }

        SctMenuItem {
            text: "Configure"

            enabled: Runtime.appFeatures.structure.enabled
            icon.source: Runtime.themedIcon("qrc:/icons/content/blank.png")

            onClicked: ScreenplayTracksDialog.launch()
        }
    }
}
