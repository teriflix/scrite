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
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

VclTextField {
    id: root

    required property bool sceneOmitted
    required property SceneHeading sceneHeading

    property SceneElementFormat sceneHeadingFormat: Scrite.document.displayFormat.elementFormat(SceneElement.Heading)

    label: ""
    text: _private.text
    color: sceneOmitted ? "gray" : sceneHeadingFormat.textColor
    readOnly: Scrite.document.readOnly || !(sceneHeading.enabled && !sceneOmitted)
    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    hoverEnabled: sceneOmitted
    maximumLength: 140
    placeholderText: sceneHeading.enabled ? "INT. SOMEPLACE - DAY" : "NO SCENE HEADING"
    completionPrefix: _private.completionPrefix
    completionStrings: _private.completionStrings
    enableTransliteration: true
    singleClickAutoComplete: Runtime.screenplayEditorSettings.singleClickAutoComplete

    font.bold: _private.font.bold
    font.family: _private.font.family
    font.italic: _private.font.italic
    font.pointSize: _private.font.pointSize
    font.underline: _private.font.underline
    font.letterSpacing: _private.font.letterSpacing
    font.capitalization: _private.fontCapitalization

    background: Item { }

    includeSuggestion: (suggestion) => { return _private.includeSuggestion(suggestion) }

    onEditingComplete: (text) => { _private.updateText(text) }

    onActiveFocusChanged: () => {
                              if(activeFocus) {
                                  _private.previouslyActiveLanguage = Scrite.app.transliterationEngine.language
                                  sceneHeadingFormat.activateDefaultLanguage()
                              } else {
                                  _private.updateText(text)
                                  Scrite.app.transliterationEngine.language = _private.previouslyActiveLanguage
                              }
                          }

    QtObject {
        id: _private

        Component.onCompleted: {
            root.font.capitalization = Qt.binding( () => { return _private.fontCapitalization } )
        }

        property string text: {
            if(sceneOmitted)
                return "[OMITTED] " + (hovered ? sceneHeading.displayText : "")

            if(sceneHeading.enabled)
                return activeFocus ? sceneHeading.editText : sceneHeading.displayText

            return ""
        }

        property int currentLanguage: Scrite.app.transliterationEngine.language
        property int fontCapitalization: activeFocus ? (currentLanguage === TransliterationEngine.English ? Font.AllUppercase : Font.MixedCase) : Font.AllUppercase
        property int previouslyActiveLanguage: TransliterationEngine.English
        property font font: root.sceneHeadingFormat.font2

        property int dotPosition: text.indexOf(".")
        property int dashPosition: text.lastIndexOf("-")
        property bool editingLocationTypePart: dotPosition < 0 || cursorPosition < dotPosition
        property bool editingMomentPart: dashPosition > 0 && cursorPosition >= dashPosition
        property bool editingLocationPart: dotPosition > 0 ? (cursorPosition >= dotPosition && (dashPosition < 0 ? true : cursorPosition < dashPosition)) : false

        property var completionStrings: {
            if(editingLocationPart)
                return Scrite.document.structure.allLocations()
            if(editingLocationTypePart)
                return Scrite.document.structure.standardLocationTypes()
            if(editingMomentPart)
                return Scrite.document.structure.standardMoments()
            return []
        }

        property string completionPrefix: {
            if(editingLocationPart)
                return text.substring(dotPosition+1, dashPosition < 0 ? text.length : dashPosition).trim()
            if(editingLocationTypePart)
                return dotPosition < 0 ? text : text.substring(0, dotPosition).trim()
            if(editingMomentPart)
                return text.substring(dashPosition+1).trim()
            return ""
        }

        function updateText(text) {
            if(!readOnly)
                return

            sceneHeading.parseFrom(text)
        }

        function includeSuggestion(suggestion) {
            if(editingLocationPart || editingLocationTypePart || editingMomentPart) {
                var one = editingLocationTypePart ? suggestion : text.substring(0, dotPosition).trim()
                var two = editingLocationPart ? suggestion : (dotPosition > 0 ? text.substring(dotPosition+1, dashPosition < 0 ? text.length : dashPosition).trim() : "")
                var three = editingMomentPart ? suggestion : (dashPosition < 0 ? "" : text.substring(dashPosition+1).trim())

                var cp = 0
                if(editingLocationTypePart)
                    cp = one.length + 2
                else if(editingLocationPart)
                    cp = one.length + 2 + two.length + 3
                else if(editingMomentPart)
                    cp = one.length + two.length + three.length + 2 + 3

                Qt.callLater( function() {
                    root.cursorPosition = cp
                })

                var ret = one + ". "
                if(two.length > 0 || three.length > 0)
                    ret += two + " - " + three
                return ret
            }

            return suggestion
        }
    }
}

