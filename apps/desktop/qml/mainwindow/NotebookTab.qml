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
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import io.scrite.components

import ".."
import "../globals"
import "../helpers"
import "../controls"
import "../notifications"

Item {
    id: root

    RowLayout {
        anchors.fill: parent

        VerticalToolBar {
            Layout.fillHeight: true

            actions: ActionHub.notebookOperations
        }

        NotebookView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            enabled: Runtime.appFeatures.notebook.enabled
        }
    }


    DisabledFeatureNotice {
        anchors.fill: parent

        visible: !Runtime.appFeatures.notebook.enabled

        featureName: "Notebook"
    }
}
