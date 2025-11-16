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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs/pdfdialog"

DialogLauncher {
    id: root

    function launch(title, filePath, dlFilePath, pagesPerRow, allowSave) {
        const initialProps = {
            "title": title,
            "source": Url.fromPath(filePath),
            "saveFilePath": dlFilePath,
            "allowFileSave": allowSave,
            "pagesPerRow": pagesPerRow
        }

        return doLaunch(initialProps)
    }

    name: "PdfDialog"
    singleInstanceOnly: true

    dialogComponent: PdfDialogImpl { }
}
