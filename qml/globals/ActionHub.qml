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

pragma Singleton

import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0

import io.scrite.components 1.0

Item {
    id: root

    function setBinder(binder) { _private.setBinder(binder) }
    function resetBinder(binder) { _private.resetBinder(binder) }

    readonly property ActionManager paragraphFormats: ActionManager {
        function action(type) { return _paragraphFormatActions.action(type) }

        name: "Paragraph Format"
    }

    Repeater {
        id: _paragraphFormatActions

        function action(type) {
            if(type < 0 || type >= _private.availableParagraphFormats.length)
                return null

            return _paragraphFormatActions.itemAt(type)
        }

        model: _private.availableParagraphFormats

        // Repeater delegates can only be Item {}, they cannot be QObject types.
        // So, that rules out creating just Action {} as delegate. It has to be
        // nested in an Item.
        delegate: Item {
            required property int index
            required property var modelData // { value, name, display, icon }

            visible: false

            Action {
                property int sortOrder: index
                property string defaultShortcut: "Ctrl+" + index

                property string tooltip: modelData.display + "\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)

                ActionManager.target: root.paragraphFormats

                checkable: true
                checked: _private.binder !== null ? (_private.binder.currentElement ? _private.binder.currentElement.type === modelData.value : false) : false
                enabled: _private.binder !== null
                icon.source: modelData.icon
                objectName: modelData.name
                shortcut: defaultShortcut
                text: modelData.display

                onTriggered: {
                    if(index === 0) {
                        if(!_private.binder.scene.heading.enabled)
                        _private.binder.scene.heading.enabled = true
                        Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneHeading)
                    } else {
                        _private.binder.currentElement.type = modelData.value
                    }
                }
            }
        }
    }

    function init(_parent) {
        if( !(_parent && Scrite.app.verifyType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
        anchors.fill = parent
    }

    visible: false

    QtObject {
        id: _private

        property SceneDocumentBinder binder // reference to the current binder on which paragraph formatting must be applied

        function setBinder(binder) {
            _private.binder = binder
        }

        function resetBinder(binder) {
            if(_private.binder === binder)
                _private.binder = null
        }

        readonly property var availableParagraphFormats: [
            { "value": SceneElement.Heading, "name": "headingParagraph", "display": "Current Scene Heading", "icon": "qrc:/icons/screenplay/heading.png" },
            { "value": SceneElement.Action, "name": "actionParagraph", "display": "Action", "icon": "qrc:/icons/screenplay/action.png" },
            { "value": SceneElement.Character, "name": "characterParagraph", "display": "Character", "icon": "qrc:/icons/screenplay/character.png" },
            { "value": SceneElement.Dialogue, "name": "dialogueParagraph", "display": "Dialogue", "icon": "qrc:/icons/screenplay/dialogue.png" },
            { "value": SceneElement.Parenthetical, "name": "parentheticalParagraph", "display": "Parenthetical", "icon": "qrc:/icons/screenplay/parenthetical.png" },
            { "value": SceneElement.Shot, "name": "shotParagraph", "display": "Shot", "icon": "qrc:/icons/screenplay/shot.png" },
            { "value": SceneElement.Transition, "name": "transitionParagraph", "display": "Transition", "icon": "qrc:/icons/screenplay/transition.png" }
        ]
    }
}
