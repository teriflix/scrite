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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

/**
Once upon a time sceneNumber was the same as element index.
It is no longer the case. Scene number can event be user specified.
So rather than using ScreenplayElement.sceneNumber we now use ScreenplayElement.elementIndex
to let the report generator know which scenes the user has selected.
*/

ColumnLayout {
    property var fieldInfo
    property AbstractReportGenerator report

    spacing: 5

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap

        text: fieldInfo.label
    }

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap
        font.italic: true
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: fieldInfo.note ? (fieldInfo.note + ". Filter by: ") : "Filter by: "
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.rightMargin: 20

        spacing: 10

        function split(string) {
            string = string.trim()
            if(string === "")
                return []
            var items = string.split(",")
            for(var i=0; i<items.length; i++)
                items[i] = items[i].trim().toUpperCase()
            items.contains = function(string) {
                for(var i=0; i<this.length; i++) {
                    if(string.indexOf(this[i]) >= 0)
                        return true
                }
                return false
            }

            return items
        }

        VclTextField {
            id: locTypeFilter

            Layout.preferredWidth: (parent.width - parent.spacing*2)*0.25

            label: ""
            placeholderText: "INT, EXT ..."

            property var items: parent.split(text)
            font.capitalization: Font.AllUppercase
        }

        VclTextField {
            id: locFilter

            Layout.fillWidth: true

            label: ""
            placeholderText: Scrite.document.structure.allLocations()[0] + " ..."

            property var items: parent.split(text)
            font.capitalization: Font.AllUppercase
        }

        VclTextField {
            id: momentFilter

            Layout.preferredWidth: (parent.width - parent.spacing*2)*0.25

            label: ""
            placeholderText: "DAY, NIGHT ..."

            property var items: parent.split(text)
            font.capitalization: Font.AllUppercase
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: 320

        background: Rectangle {
            color: Runtime.colors.primary.c50.background
            border.width: 1
            border.color: Runtime.colors.primary.c50.text
        }

        ListView {
            id: sceneListView

            property var selectedSceneNumbers: []
            property var selectedEpisodeNumbers: null

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            function filter(scene) {
                if(scene) {
                    if(selectedEpisodeNumbers && selectedEpisodeNumbers.length > 0) {
                        if(scene && selectedEpisodeNumbers.indexOf(scene.episodeIndex+1) < 0)
                            return false
                    }

                    if(scene.heading.enabled) {
                        var ret = true
                        if(ret && locTypeFilter.items.length > 0)
                            ret &= locTypeFilter.items.contains(scene.heading.locationType)
                        if(ret && momentFilter.items.length > 0)
                            ret &= momentFilter.items.contains(scene.heading.moment)
                        if(ret && locFilter.items.length > 0)
                            ret &= locFilter.items.contains(scene.heading.location)

                        return ret
                    }
                }

                return locTypeFilter.items.length === 0 && momentFilter.items.length === 0 && locFilter.items.length === 0
            }

            function select(sceneNumber, flag) {
                var numbers = report.getConfigurationValue(fieldInfo.name)
                var idx = numbers.indexOf(sceneNumber)
                if(flag) {
                    if(idx < 0)
                        numbers.push(sceneNumber)
                    else
                        return
                } else {
                    if(idx >= 0)
                        numbers.splice(idx, 1)
                    else
                        return
                }
                selectedSceneNumbers = numbers
                report.setConfigurationValue(fieldInfo.name, numbers)
            }

            model: Scrite.document.screenplay
            clip: true

            delegate: Item {
                required property int index
                required property int screenplayElementType
                required property int breakType
                required property string sceneID
                required property ScreenplayElement screenplayElement

                width: sceneListView.width-1
                height: sceneCheckBox.visible ? sceneCheckBox.height : 0

                VclCheckBox {
                    id: sceneCheckBox

                    width: parent.width-1

                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    visible: screenplayElement.scene && sceneListView.filter(screenplayElement.scene)
                    text: {
                        var scene = screenplayElement.scene
                        if(scene && scene.heading.enabled)
                            return "[" + screenplayElement.resolvedSceneNumber + "] " + (scene && scene.heading.enabled ? scene.heading.text : "")
                        return "NO SCENE HEADING"
                    }
                    checked: sceneListView.selectedSceneNumbers.indexOf(screenplayElement.elementIndex) >= 0
                    enabled: screenplayElement.scene

                    onToggled: sceneListView.select(screenplayElement.elementIndex, checked)
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        spacing: 10

        VclButton {
            text: "Select All"
            onClicked: {
                var count = Scrite.document.screenplay.elementCount
                var numbers = report.getConfigurationValue(fieldInfo.name)
                for(var i=0; i<count; i++) {
                    var element = Scrite.document.screenplay.elementAt(i)
                    if( sceneListView.filter(element.scene) ) {
                        if(numbers.indexOf(element.elementIndex) < 0)
                            numbers.push(element.elementIndex)
                    }
                }
                sceneListView.selectedSceneNumbers = numbers
                report.setConfigurationValue(fieldInfo.name, numbers)
            }
        }

        VclButton {
            text: "Unselect All"
            onClicked: {
                var count = Scrite.document.screenplay.elementCount
                var numbers = report.getConfigurationValue(fieldInfo.name)
                for(var i=0; i<count; i++) {
                    var element = Scrite.document.screenplay.elementAt(i)
                    if( sceneListView.filter(element.scene) ) {
                        var idx = numbers.indexOf(element.elementIndex)
                        if(idx >= 0)
                            numbers.splice(idx, 1)
                    }
                }
                sceneListView.selectedSceneNumbers = numbers
                report.setConfigurationValue(fieldInfo.name, numbers)
            }
        }

        VclLabel {
            Layout.fillWidth: true

            text: sceneListView.selectedSceneNumbers.length === 0 ? "All Scenes Are Selected" : ("" + sceneListView.selectedSceneNumbers.length + " Scene(s) Are Selected")
            padding: 5
        }
    }

    function getReady() {
        const ssn = report ? report.getConfigurationValue(fieldInfo.name) : []
        sceneListView.selectedSceneNumbers = ssn
        sceneListView.selectedEpisodeNumbers = report.episodeNumbers
        const idx = ssn && ssn.length > 0 ? ssn[0] : 0
        sceneListView.positionViewAtIndex(idx, ListView.Beginning)
    }
}
