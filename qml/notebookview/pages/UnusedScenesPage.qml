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

import "qrc:/qml/controls"

AbstractNotebookPage {
    id: root

    VclLabel {
        anchors.fill: parent
        anchors.margins: 20

        text: "<b><font size=\"+2\">Unused Scenes</font></b><br/><br/>Unused scenes are those that are placed on structure but are not yet dragged into the screenplay (or timeline). Click on any of the unused scenes in the tree to the left to view their notes."
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }
}
