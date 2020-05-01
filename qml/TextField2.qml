/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

TextField {
    id: textField
    property alias completionStrings: completer.strings
    property Item tabItem
    property Item backTabItem
    property bool enableTransliteration: false
    selectedTextColor: "white"
    selectionColor: "blue"

    signal editingComplete()

    onEditingFinished: {
        if(enableTransliteration)
            text = app.transliterationEngine.transliteratedSentence(text, true)
        editingComplete()
    }
    Component.onDestruction: {
        if(enableTransliteration)
            text = app.transliterationEngine.transliteratedSentence(text, true)
        editingComplete()
    }

    onActiveFocusChanged: {
        if(activeFocus)
            selectAll()
    }

    Item {
        x: parent.cursorRectangle.x
        y: parent.cursorRectangle.y
        width: parent.cursorRectangle.width
        height: parent.cursorRectangle.height

        ToolTip.visible: parent.activeFocus && completer.hasSuggestion && completer.suggestion !== parent.text
        ToolTip.text: completer.suggestion
    }

    Completer {
        id: completer
        suggestionMode: Completer.CompleteSuggestion
        completionPrefix: textField.text.toUpperCase()
    }

    onTextEdited: {
        if(enableTransliteration)
            text = app.transliterationEngine.transliteratedSentence(text, false)
    }

    Keys.onReturnPressed: {
        if(tabItem)
            tabItem.forceActiveFocus()
    }

    Keys.onTabPressed: {
        if(completer.hasSuggestion && completer.suggestion !== text) {
            text = completer.suggestion
            editingFinished()
        } else if(tabItem)
            tabItem.forceActiveFocus()
    }

    KeyNavigation.tab: tabItem
    KeyNavigation.backtab: backTabItem
}
