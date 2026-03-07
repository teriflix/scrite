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

pragma Singleton

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../controls"
import "../helpers"
import "./pdfdialog"

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
