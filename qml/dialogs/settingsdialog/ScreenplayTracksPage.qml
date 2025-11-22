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
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    clip: true

    VclGroupBox {
        anchors.fill: parent
        anchors.margins: 10
        anchors.leftMargin: 0

        spacing: 10

        enabled: Runtime.appFeatures.structure.enabled
        opacity: enabled ? 1 : 0.5

        label: VclCheckBox {
            text: "Display tracks"

            checked: Runtime.screenplayTracksSettings.displayTracks

            onToggled: Runtime.screenplayTracksSettings.displayTracks = !Runtime.screenplayTracksSettings.displayTracks
        }

        ColumnLayout {
            anchors.fill: parent

            enabled: Runtime.screenplayTracksSettings.displayTracks
            opacity: enabled ? 1 : 0.5

            RowLayout {
                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Display structure tracks"
                    checked: enabled ? Runtime.screenplayTracksSettings.displayStructureTracks : false

                    onToggled: Runtime.screenplayTracksSettings.displayStructureTracks = !Runtime.screenplayTracksSettings.displayStructureTracks
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Display stack/sequence tracks"
                    checked: enabled ? Runtime.screenplayTracksSettings.displayStacks : false

                    onToggled: Runtime.screenplayTracksSettings.displayStacks = !Runtime.screenplayTracksSettings.displayStacks
                }
            }

            VclGroupBox {
                Layout.fillWidth: true
                Layout.fillHeight: true

                label: VclCheckBox {
                    id: _chkKeywordsTracks

                    text: "Display keywords tracks"
                    checked: enabled ? Runtime.screenplayTracksSettings.displayKeywordsTracks : false

                    onToggled: Runtime.screenplayTracksSettings.displayKeywordsTracks = !Runtime.screenplayTracksSettings.displayKeywordsTracks
                }

                ColumnLayout {
                    anchors.fill: parent

                    VclLabel {
                        Layout.fillWidth: true
                        Layout.bottomMargin: 10

                        wrapMode: Text.WordWrap
                        text: _private.keywords.length === 0 ? "All keywords are captured." : "Only the following keywords are captured in this document."
                    }

                    Flickable {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ScrollBar.vertical: VclScrollBar { }

                        contentWidth: width
                        contentHeight: _keywordsList.height

                        TextListInput {
                            id: _keywordsList

                            width: parent.width - 20

                            enabled: _chkKeywordsTracks.checked
                            opacity: enabled ? 1 : 0.5

                            textList: _private.keywords
                            completionStrings: Scrite.document.structure.sceneTags
                            addTextButtonTooltip: "Click here to include a keyword to allowed list, and eliminate others."
                            font: Runtime.idealFontMetrics.font
                            labelText: "Keywords"
                            labelIconSource:  "qrc:/icons/action/keyword.png"
                            labelIconVisible: true

                            readOnly: Scrite.document.readOnly

                            onEnsureVisible: (item, area) => {  }
                            onTextClicked: (text, source) => {  }
                            onTextCloseRequest: (text, source) => { _private.removeKeyword(text) }
                            onConfigureTextRequest: (text, tag) => { tag.closable = true }
                            onNewTextRequest: (text) => { _private.addKeyword(text) }
                            onNewTextCancelled: () => { }
                        }
                    }
                }
            }
        }
    }

    DisabledFeatureNotice {
        anchors.fill: parent

        visible: !Runtime.appFeatures.structure.enabled
        featureName: "Structure"
    }

    QtObject {
        id: _private

        Component.onCompleted: {
            const userData = Scrite.document.userData
            if(userData && userData.allowedOpenTagsInTracks !== undefined && userData.allowedOpenTagsInTracks.length > 0)
                keywords = userData.allowedOpenTagsInTracks
        }

        property var keywords: []

        function addKeyword(keyword) {
            if(keyword === undefined || keyword === "")
                return

            keyword = keyword.toLowerCase()
            if(keywords.indexOf(keyword) >= 0)
                return

            let k = keywords
            k.push(keyword)
            keywords = k
            saveKeywords()
        }

        function removeKeyword(keyword) {
            if(keyword === undefined || keyword === "")
                return

            keyword = keyword.toLowerCase()

            const idx = keywords.indexOf(keyword)
            if(idx < 0)
                return

            let k = keywords
            k.splice(idx, 1)
            keywords = k
            saveKeywords()
        }

        function saveKeywords() {
            let userData = Scrite.document.userData
            userData.allowedOpenTagsInTracks = keywords
            Scrite.document.userData = userData
        }
    }
}
