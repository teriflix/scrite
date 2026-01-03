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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

VclMenu {
    id: root

    VclMenu {
        title: "Text"

        VclMenuItem {
            text: "Heading"

            readonly property string option: "HEADING"

            icon.source: Runtime.sceneListPanelSettings.sceneTextMode === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.sceneTextMode = option
        }

        VclMenuItem {
            text: "Summary"

            readonly property string option: "SUMMARY"

            icon.source: Runtime.sceneListPanelSettings.sceneTextMode === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.sceneTextMode = option
        }

        MenuSeparator { }

        VclMenuItem {
            text: "Show Tooltip"

            icon.source: Runtime.sceneListPanelSettings.showTooltip ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.showTooltip = !Runtime.sceneListPanelSettings.showTooltip
        }
    }

    VclMenu {
        title: "Length"

        VclMenuItem {
            text: "Scene Duration"

            readonly property string option: "TIME"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }

        VclMenuItem {
            text: "Page Length"

            readonly property string option: "PAGE"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }

        VclMenuItem {
            text: "None"

            readonly property string option: "NO"

            enabled: !Runtime.paginator.paused
            icon.source: Runtime.sceneListPanelSettings.displaySceneLength === option ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.displaySceneLength = option
        }
    }

    VclMenu {
        title: "Tracks"

        VclMenuItem {
            text: "Display"

            enabled: Runtime.appFeatures.structure.enabled && Runtime.screenplayTracksSettings.displayTracks
            icon.source: Runtime.sceneListPanelSettings.displayTracks ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

            onClicked: Runtime.sceneListPanelSettings.displayTracks = !Runtime.sceneListPanelSettings.displayTracks
        }

        VclMenuItem {
            text: "Configure"

            enabled: Runtime.appFeatures.structure.enabled
            icon.source: "qrc:/icons/content/blank.png"

            onClicked: ScreenplayTracksDialog.launch()
        }
    }
}
