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
import QtQuick.Pdf 5.15
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/modules"

VclDialog {
    id: root

    width: Scrite.window.width - 50
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
