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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components


import "../../globals"
import "../../controls"
import "../../helpers"

/**
Once upon a time sceneNumber was the same as element index.
It is no longer the case. Scene number can event be user specified.
So rather than using ScreenplayElement.sceneNumber we now use ScreenplayElement.elementIndex
to let the report generator know which scenes the user has selected.
*/

ColumnLayout {
    id: root
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
            id: _root_2

            Layout.preferredWidth: (parent.width - parent.spacing*2)*0.25

            label: ""
            placeholderText: "INT, EXT ..."

            property var items: parent.split(text)
            font.capitalization: Font.AllUppercase
        }

        VclTextField {
            id: _locFilter

            Layout.fillWidth: true

            label: ""
            placeholderText: Scrite.document.structure.allLocations()[0] + " ..."

            property var items: parent.split(text)
            font.capitalization: Font.AllUppercase
        }

        VclTextField {
            id: _momentFilter

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
            id: _sceneListView

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
                        if(ret && _root_2.items.length > 0)
                            ret &= _root_2.items.contains(scene.heading.locationType)
                        if(ret && _momentFilter.items.length > 0)
                            ret &= _momentFilter.items.contains(scene.heading.moment)
                        if(ret && _locFilter.items.length > 0)
                            ret &= _locFilter.items.contains(scene.heading.location)

                        return ret
                    }
                }

                return _root_2.items.length === 0 && _momentFilter.items.length === 0 && _locFilter.items.length === 0
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

                width: _sceneListView.width-1
                height: _sceneCheckBox.visible ? _sceneCheckBox.height : 0

                VclCheckBox {
                    id: _sceneCheckBox

                    width: parent.width-1

                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    visible: screenplayElement.scene && _sceneListView.filter(screenplayElement.scene)
                    text: {
                        var scene = screenplayElement.scene
                        if(scene && scene.heading.enabled)
                            return "[" + screenplayElement.resolvedSceneNumber + "] " + (scene && scene.heading.enabled ? scene.heading.text : "")
                        return "NO SCENE HEADING"
                    }
                    checked: _sceneListView.selectedSceneNumbers.indexOf(screenplayElement.elementIndex) >= 0
                    enabled: screenplayElement.scene

                    onToggled: _sceneListView.select(screenplayElement.elementIndex, checked)
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
                    if( _sceneListView.filter(element.scene) ) {
                        if(numbers.indexOf(element.elementIndex) < 0)
                            numbers.push(element.elementIndex)
                    }
                }
                _sceneListView.selectedSceneNumbers = numbers
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
                    if( _sceneListView.filter(element.scene) ) {
                        var idx = numbers.indexOf(element.elementIndex)
                        if(idx >= 0)
                            numbers.splice(idx, 1)
                    }
                }
                _sceneListView.selectedSceneNumbers = numbers
                report.setConfigurationValue(fieldInfo.name, numbers)
            }
        }

        VclLabel {
            Layout.fillWidth: true

            text: _sceneListView.selectedSceneNumbers.length === 0 ? "All Scenes Are Selected" : ("" + _sceneListView.selectedSceneNumbers.length + " Scene(s) Are Selected")
            padding: 5
        }
    }

    function getReady() {
        const ssn = report ? report.getConfigurationValue(fieldInfo.name) : []
        _sceneListView.selectedSceneNumbers = ssn
        _sceneListView.selectedEpisodeNumbers = report.episodeNumbers
        const idx = ssn && ssn.length > 0 ? ssn[0] : 0
        _sceneListView.positionViewAtIndex(idx, ListView.Beginning)
    }
}
