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

import io.scrite.components 1.0

import "qrc:/qml/structureview"

FocusScope {
    id: root

    required property int elementIndex
    required property StructureElement element

    required property bool canvasScrollMoving
    required property bool canvasScrollFlicking
    required property bool canvasScaleIsLessForEdit

    required property Item canvasScrollViewport
    required property rect canvasScrollViewportRect

    required property TabSequenceManager canvasTabSequence
    required property BoundingBoxEvaluator canvasItemsBoundingBox

    signal zoomOneToItemRequest(Item item)
    signal deleteElementRequest(StructureElement element)
    signal finishEditingRequest()
    signal canvasActiveFocusRequest()
}
