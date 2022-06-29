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

BorderImage {
    source: "../icons/content/shadow.png"
    horizontalTileMode: BorderImage.Stretch
    verticalTileMode: BorderImage.Stretch
    anchors { leftMargin: -9; topMargin: -9; rightMargin: -9; bottomMargin: -9 }
    border { left: 22; top: 22; right: 22; bottom: 22 }
    smooth: true
    opacity: 0.75
}
