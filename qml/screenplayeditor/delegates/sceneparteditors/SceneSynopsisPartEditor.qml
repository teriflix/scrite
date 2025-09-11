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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"

AbstractScenePartEditor {
    id: root

    height: _synopsisInput.contentHeight

    TextAreaInput {
        id: _synopsisInput

        Transliterator.spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: root.pageLeftMargin
        anchors.rightMargin: root.pageRightMargin

        text: root.scene.synopsis
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        readOnly: root.readOnly
        placeholderText: "Scene Synopsis"

        font.pointSize: _private.synopsisFontPointSize

        background: Item { }

        TextAreaSpellingSuggestionsMenu { }

        onTextChanged: if(activeFocus) root.scene.synopsis = text
        onEditingFinished: root.scene.synopsis = text

        onActiveFocusChanged: {
            if(activeFocus)
                root.ensureVisible(_synopsisInput, Qt.rect(0, -10, cursorRectangle.width, cursorRectangle.height+20))
        }

        Announcement.onIncoming: (type,data) => {
            if(!root.screenplayElementDelegateHasFocus && root.readOnly)
                return

            var sdata = "" + data
            var stype = "" + type
            if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.sceneSynopsis) {
                _synopsisInput.forceActiveFocus()
            }
        }
    }

    QtObject {
        id: _private

        property real synopsisFontPointSize: Math.max(sceneHeadingFormat.font2.pointSize*0.7, 6)
        property SceneElementFormat sceneHeadingFormat: Scrite.document.displayFormat.elementFormat(SceneElement.Heading)
    }
}
