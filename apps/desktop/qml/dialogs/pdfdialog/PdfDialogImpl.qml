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

import QtQuick
import QtQuick.Pdf
import QtQuick.Window
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../helpers"
import "../../controls"

VclDialog {
    id: root

    width: Scrite.window.width - 51
    height: Scrite.window.height - 50

    property alias source: pdfView.source
    property alias pagesPerRow: pdfView.pagesPerRow
    property alias saveFilePath: pdfView.saveFilePath
    property alias saveFileName: pdfView.saveFileName
    property alias closable: pdfView.closable
    property alias allowFileSave: pdfView.allowFileSave
    property alias allowFileReveal: pdfView.allowFileReveal
    property alias saveFeatureDisabled: pdfView.saveFeatureDisabled
    property alias displayRefreshButton: pdfView.displayRefreshButton

    signal closeRequest()
    signal refreshRequest()

    contentItem: PdfView {
        id: pdfView

        width: root.width
        height: root.height - root.header.height
    }

    footer: null
}
