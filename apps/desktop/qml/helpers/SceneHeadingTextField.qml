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

import QtQuick
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

VclTextField {
    id: root

    required property bool sceneOmitted
    required property SceneHeading sceneHeading

    property SceneElementFormat sceneHeadingFormat: Scrite.document.displayFormat.elementFormat(SceneElement.Heading)

    label: ""
    text: _private.headingText
    color: Runtime.colors.tx(sceneOmitted ? "gray" : sceneHeadingFormat.textColor)
    readOnly: Scrite.document.readOnly || !(sceneHeading.enabled && !sceneOmitted)
    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    hoverEnabled: sceneOmitted
    maximumLength: 140
    placeholderText: sceneHeading.enabled ? "INT. SOMEPLACE - DAY" : "NO SCENE HEADING"
    undoRedoEnabled: false
    completionPrefix: _private.completionPrefix
    completionStrings: _private.completionStrings
    enableTransliteration: true

    font.bold: _private.font.bold
    font.family: _private.font.family
    font.italic: _private.font.italic
    font.pointSize: _private.font.pointSize
    font.underline: _private.font.underline
    font.letterSpacing: _private.font.letterSpacing
    font.capitalization: _private.fontCapitalization

    background: Item { }

    includeSuggestion: (suggestion) => {
                           return _private.includeSuggestion(suggestion)
                       }

    onEditingComplete: () => {
                           _private.updateText(text)
                       }

    onActiveFocusChanged: () => {
                              if(activeFocus) {
                                  _private.previouslyActiveLanguageCode = Runtime.language.activeCode
                                  sceneHeadingFormat.activateDefaultLanguage()
                              } else {
                                  _private.updateText(text)
                                  Runtime.language.setActiveCode(_private.previouslyActiveLanguageCode)
                              }
                          }

    QtObject {
        id: _private

        property string headingText: {
            if(root.sceneOmitted)
                return "[OMITTED] " + (root.hovered ? root.sceneHeading.displayText : "")

            if(root.sceneHeading.enabled)
                return root.activeFocus ? root.sceneHeading.editText : root.sceneHeading.displayText

            return ""
        }

        property int fontCapitalization: root.activeFocus ? (Runtime.language.active.charScript() === QtChar.Script_Latin ? Font.AllUppercase : Font.MixedCase) : Font.AllUppercase
        property int previouslyActiveLanguageCode: QtLocale.English
        property font font: root.sceneHeadingFormat.font2

        property int dotPosition: root.text.indexOf(".")
        property int dashPosition: root.text.lastIndexOf("-")
        property bool editingLocationTypePart: root.activeFocus ? dotPosition < 0 || root.cursorPosition < dotPosition : false
        property bool editingMomentPart: root.activeFocus ? dashPosition > 0 && root.cursorPosition >= dashPosition : false
        property bool editingLocationPart: root.activeFocus ? dotPosition > 0 ? (root.cursorPosition >= dotPosition && (dashPosition < 0 ? true : root.cursorPosition < dashPosition)) : false : false

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
            let pickPrefix = () => {
                if(editingLocationPart)
                    return root.text.substring(dotPosition+1, dashPosition < 0 ? root.text.length : dashPosition).trim()
                if(editingLocationTypePart)
                    return dotPosition < 0 ? root.text : root.text.substring(0, dotPosition).trim()
                if(editingMomentPart)
                    return root.text.substring(dashPosition+1).trim()
                return ""
            }
            if(fontCapitalization == Font.AllUppercase)
                return pickPrefix().toUpperCase()
            return pickPrefix()
        }

        function updateText(text) {
            if(root.readOnly)
                return

            root.sceneHeading.parseFrom(text)
        }

        function includeSuggestion(suggestion) {
            if(editingLocationPart || editingLocationTypePart || editingMomentPart) {
                let one = editingLocationTypePart ? suggestion : root.text.substring(0, dotPosition).trim()
                let two = editingLocationPart ? suggestion : (dotPosition > 0 ? root.text.substring(dotPosition+1, dashPosition < 0 ? root.text.length : dashPosition).trim() : "")
                let three = editingMomentPart ? suggestion : (dashPosition < 0 ? "" : root.text.substring(dashPosition+1).trim())

                let cp = 0
                if(editingLocationTypePart)
                    cp = one.length + 2
                else if(editingLocationPart)
                    cp = one.length + 2 + two.length + 3
                else if(editingMomentPart)
                    cp = one.length + two.length + three.length + 2 + 3

                Qt.callLater( function() {
                    root.cursorPosition = cp
                })

                let ret = one + ". "
                if(two.length > 0 || three.length > 0)
                    ret += two + " - " + three
                return ret
            }

            return suggestion
        }
    }

    ActionHandler {
        id: _transliterateActionHandler

        property string text: "Transliterate to " + Runtime.language.active.name

        action: ActionHub.editOptions.find("translateToActiveLanguage") as Action
        enabled: !root.readOnly && root.activeFocus && root.selectedText !== "" &&
                 Runtime.language.textSelectionTransliterationEnabled

        onTriggered: () => {
            if(!enabled) return

            root.forceActiveFocus()
            const option = Runtime.language.active.preferredTransliterationOption()
            if(option && option.inApp) {
                const pos = root.selectionStart
                const txText = option.transliterateParagraph(root.selectedText)
                if(txText !== "" && txText !== root.selectedText) {
                    root.remove(root.selectionStart, root.selectionEnd)
                    root.insert(pos, txText)
                }
            }
        }
    }

    Component.onCompleted: {
        if(contextMenu) {
            contextMenu.addItem(_menuSeparator.createObject(contextMenu))
            contextMenu.addItem(_transliterateMenuItem.createObject(contextMenu))
        }
    }

    Component {
        id: _menuSeparator

        MenuSeparator { }
    }

    Component {
        id: _transliterateMenuItem

        // We don't set action: here because MenuItem binds its enabled to action.enabled,
        // which is ActionHandler.canHandle. When the context menu opens the field loses
        // focus, disabling the ActionHandler and graying out the item before it can be
        // clicked. Instead we manage enabled independently and call forceActiveFocus()
        // before triggering, so the ActionHandler is active when the action fires.
        VclMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Transliterate to " + Runtime.language.active.name + "\t" + _transliterateActionHandler.action.shortcut
            enabled: !root.readOnly && root.selectedText !== "" && Runtime.language.activeCode === QtLocale.English &&
                     Runtime.language.textSelectionTransliterationEnabled

            onTriggered: () => {
                             root.forceActiveFocus()
                             _transliterateActionHandler.action.trigger()
                         }
        }
    }
}

