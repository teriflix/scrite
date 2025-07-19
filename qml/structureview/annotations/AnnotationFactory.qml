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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview/annotations"

QtObject {
    id: root

    readonly property var keys: [
        { "title": "Text", "what": "text" },
        { "title": "Oval", "what": "oval" },
        { "title": "Image", "what": "image" },
        { "title": "Rectangle", "what": "rectangle" },
        { "title": "Website Link", "what": "url" },
        { "title": "Vertical Line", "what": "vline" },
        { "title": "Horizontal Line", "what": "hline" }
    ]

    function create(type, x, y, parent) {
        switch(type) {
        case "hline": return LineAnnotationFactory.create(parent, x, y, "Horizontal")
        case "vline": return LineAnnotationFactory.create(parent, x, y, "Vertical")
        case "rectangle": return RectangleAnnotationFactory.create(parent, x, y)
        case "text": return TextAnnotationFactory.create(parent, x, y)
        case "url": return UrlAnnotationFactory.create(parent, x, y)
        case "image": return ImageAnnotationFactory.create(parent, x, y)
        case "oval": return OvalAnnotationFactory.create(parent, x, y)
        }

        return null
    }

    function delegateFor(type) {
        switch(type) {
        case "rectangle": return RectangleAnnotationFactory.delegate
        case "text": return TextAnnotationFactory.delegate
        case "url": return UrlAnnotationFactory.delegate
        case "image": return ImageAnnotationFactory.delegate
        case "line": return LineAnnotationFactory.delegate
        case "oval": return OvalAnnotationFactory.delegate
        }
        return null
    }

    function createDelegate(annotation, annotationIndex, parent) {
        if(!annotation || annotationIndex < 0 || parent === null)
            return null

        let comp = getAnnotationDelegate(annotation.type)
        const props = {
            "annotation": annotation,
            "annotationIndex": annotationIndex
        }

        let delegate = comp.createObject(parent, props)
        return delegate
    }
}
