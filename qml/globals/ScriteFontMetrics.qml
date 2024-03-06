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

import io.scrite.components 1.0

QtObject {
    readonly property FontMetrics minimum: FontMetrics {
        font.pointSize: Math.min(Scrite.app.idealFontPointSize-2, 12)
    }

    readonly property FontMetrics ideal: FontMetrics {
        font.pointSize: Scrite.app.idealFontPointSize
    }

    readonly property FontMetrics sceneEditor: FontMetrics {
        property SceneElementFormat format: Scrite.document.formatting.elementFormat(SceneElement.Action)
        property int lettersPerLine: 70
        property int marginLetters: 5
        property real paragraphWidth: Math.ceil(lettersPerLine*averageCharacterWidth)
        property real paragraphMargin: Math.ceil(marginLetters*averageCharacterWidth)
        property real pageWidth: Math.ceil(paragraphWidth + 2*paragraphMargin)
        font: format ? format.font2 : Scrite.document.formatting.defaultFont2
    }
}
