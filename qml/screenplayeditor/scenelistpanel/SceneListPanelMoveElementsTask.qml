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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"

SequentialAnimation {
    id: root

    required property ListView sceneListView

    property int targetIndex: -1

    property ScreenplayElement draggedElement

    signal currentIndexRequest(int index)

    PauseAnimation { duration: 50 }

    ScriptAction {
        script: Scrite.document.screenplay.moveSelectedElements(root.targetIndex)
    }

    PauseAnimation { duration: 50 }

    ScriptAction {
        script: sceneListView.forceLayout()
    }

    PauseAnimation { duration: 50 }

    ScriptAction {
        script: {
            const draggedElement = root.draggedElement
            const targetndex = draggedElement ? Scrite.document.screenplay.indexOfElement(draggedElement) : root.targetIndex

            root.targetIndex = -1
            root.draggedElement = null

            // contentView.positionViewAtIndex(targetndex, ListView.Beginning)
            // privateData.changeCurrentIndexTo(targetndex)
            root.currentIndexRequest(targetIndex)

            sceneListView.forceActiveFocus()
        }
    }

    onTargetIndexChanged: {
        if(targetIndex >= 0)
            start()
    }
}
