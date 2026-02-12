/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        padding: 20
        text: "Ooops! We have no idea what to show here."
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
    }
}
