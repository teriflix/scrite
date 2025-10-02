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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Popup {
    id: root

    property font editorFont: Runtime.idealFontMetrics.font

    property ImTransliterator transliterator

    property rect textRect: transliterator ? Scrite.window.contentItem.mapFromItem(transliterator.editor, transliterator.textRect) : Qt.rgba(0,0,0,0)

    x: textRect.x + 10
    y: textRect.y + textRect.height + 5

    parent: Scrite.window.contentItem
    visible: transliterator ? transliterator.commitString !== "" : false
    closePolicy: Popup.NoAutoClose

    contentItem: Label {
        text: root.transliterator ? root.transliterator.commitString : ""
        font: root.editorFont
    }
}
