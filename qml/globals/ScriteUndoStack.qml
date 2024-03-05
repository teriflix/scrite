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

UndoStack {
    objectName: "MainUndoStack"

    property bool sceneListPanelActive: false
    property bool screenplayEditorActive: false
    property bool timelineEditorActive: false
    property bool structureEditorActive: false
    property bool sceneEditorActive: false
    property bool notebookActive: false

    active: sceneListPanelActive || screenplayEditorActive || timelineEditorActive || structureEditorActive || sceneEditorActive || notebookActive
}

