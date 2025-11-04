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

import QtQml 2.15
import QtQuick 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"

QtObject {
    id: root

    readonly property string type: "rectangle"

    function create(parent, x, y, w, h) {
        if(Scrite.document.readOnly)
            return

        let doNotAlignRect = w && h

        w = w ? w : 200
        h = h ? h : 200

        let geometry = doNotAlignRect ? Qt.rect(x, y, w, h) : Qt.rect(x - w/2, y-h/2, w, h)
        let annot = Runtime.newAnnotation(parent, type, geometry, null)
        Scrite.document.structure.addAnnotation(annot)

        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d
    }
}
