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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "../js/utils.js" as Utils
import "./globals"

/**
Save workflow
================

This workflow can be initiated from anywhere to execute an "operation" after
taking the user through a series of UI screens helping them save the
"current document".

For using this workflow, there are two important objects
- Current Document: Implicitly assumed as Scrite.document
- Operation: Must be explicitly provided as a callback function

The way the save workflow works is different depending on whether the current
document is saved or not.

If the current document is empty or has no changes, then
    - execute operation

If the current document is not saved even once, then we ask if user wants to save
    - yes => show save dialog box -> save the file -> execute operation
    - no => discard changes -> execute operation
    - cancel => discard changes -> abort operation

If the current document is already saved, but has changes
    - if auto save is enabled => save changes -> execute operation
    - if auto save is disabled, then we ask if user wants to save changes
        - yes => show save dialog box -> save the file -> execute operation
        - no => discard changes -> execute operation
        - cancel => discard changes -> abort operation
  */

Item {
    id: saveWorkflow

    property var operation: function() { } // must be supplied by the caller
    signal done()

    QtObject {
        id: _private

        function execute() {
            if(saveWorkflow.operation)
                saveWorkflow.operation()
            Qt.callLater(endWorkflow)
        }

        function abort() {
            endWorkflow()
        }

        function endWorkflow() {
            workflowLoader.active = false
            saveWorkflow.done()
        }

        property real maxTextWidth: Math.min(300, saveWorkflow.width*0.5)
    }

    Rectangle {
        anchors.fill: parent
        color: ScriteRuntime.colors.primary.windowColor
        opacity: 0.9
        visible: workflowLoader.active

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true
            propagateComposedEvents: false
        }
    }

    BoxShadow {
        anchors.fill: workflowLoaderBackdrop
        visible: workflowLoader.active
    }

    Rectangle {
        id: workflowLoaderBackdrop
        anchors.fill: workflowLoader
        anchors.margins: -40
        border.width: 2
        border.color: ScriteRuntime.colors.primary.borderColor
        color: ScriteRuntime.colors.primary.c100.background
        visible: workflowLoader.active
    }

    Loader {
        id: workflowLoader
        anchors.centerIn: parent
        active: false
        sourceComponent: {
            if(Scrite.document.empty || !Scrite.document.modified || Scrite.document.readOnly)
                return documentIsEmptyOrHasNoChanges

            if(Scrite.document.fileName === "")
                return documentNotEvenSavedOnce

            if(Scrite.document.autoSave)
                return autoSaveIsEnabled

            return documentSavedButHasChanges
        }
        Component.onCompleted: Qt.callLater( () => { active = true } )

        property ErrorReport errorReport: Aggregation.findErrorReport(Scrite.document)
        Notification.title: "Saving Scrite Document"
        Notification.text: errorReport.errorMessage
        Notification.active: errorReport.hasError
        Notification.autoClose: false
        Notification.onDismissed: {
            if(errorReport.details && errorReport.details.revealOnDesktopRequest)
                Scrite.app.revealFileOnDesktop(errorReport.details.revealOnDesktopRequest)
            errorReport.clear()
        }
    }

    Component {
        id: documentNotEvenSavedOnce

        ColumnLayout {
            spacing: 40

            Text {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                width: _private.maxTextWidth
                wrapMode: Text.WordWrap
                text: "Do you want to save this document first?"
            }

            YesNoCancel {
                Layout.alignment: Qt.AlignHCenter
                onYesClicked: saveFileDialog.open()
                onNoClicked: _private.execute()
                onCancelClicked: _private.abort()
            }
        }
    }

    Component {
        id: documentSavedButHasChanges

        ColumnLayout {
            spacing: 40

            BasicFileInfo {
                id: scriteFileInfo
                absoluteFilePath: Scrite.document.fileName
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                width: _private.maxTextWidth
                wrapMode: Text.WordWrap
                text: "Do you want to save changes to '" + scriteFileInfo.baseName + "'?"
            }

            YesNoCancel {
                Layout.alignment: Qt.AlignHCenter
                onYesClicked: {
                    ScriteRuntime.recentFiles.add(scriteFileInfo.absoluteFilePath)
                    Scrite.document.save()
                    _private.execute()
                }
                onNoClicked: _private.execute()
                onCancelClicked: _private.abort()
            }
        }
    }

    Component {
        id: autoSaveIsEnabled

        Item {
            BasicFileInfo {
                id: scriteFileInfo
                absoluteFilePath: Scrite.document.fileName
            }

            Component.onCompleted: {
                Scrite.document.save()
                ScriteRuntime.recentFiles.add(scriteFileInfo.absoluteFilePath)
                _private.execute()
            }
        }
    }

    Component {
        id: documentIsEmptyOrHasNoChanges

        Item {
            Component.onCompleted: {
                _private.execute()
            }
        }
    }

    component YesNoCancel : RowLayout {
        id: _yesNoCancel
        spacing: 20

        signal yesClicked()
        signal noClicked()
        signal cancelClicked()

        Button2 {
            text: "Yes"
            onClicked: _yesNoCancel.yesClicked()
        }

        Button2 {
            text: "No"
            onClicked: _yesNoCancel.noClicked()
        }

        Button2 {
            text: "Cancel"
            onClicked: _yesNoCancel.cancelClicked()
        }
    }

    FileDialog {
        id: saveFileDialog

        title: "Save Scrite Document As"
        nameFilters: ["Scrite Documents (*.scrite)"]
        selectFolder: false
        selectMultiple: false
        objectName: "Save File Dialog"
        dirUpAction.shortcut: "Ctrl+Shift+U"
        folder: ScriteRuntime.workspaceSettings.lastOpenFolderUrl
        onFolderChanged: ScriteRuntime.workspaceSettings.lastOpenFolderUrl = folder
        sidebarVisible: true
        selectExisting: false

        onAccepted: {
            const path = Scrite.app.urlToLocalFile(fileUrl)
            Scrite.document.saveAs(path)

            ScriteRuntime.recentFiles.add(path)

            const fileInfo = Scrite.app.fileInfo(path)
            ScriteRuntime.workspaceSettings.lastOpenFolderUrl = folder

            _private.execute()
        }

        onRejected: {
            _private.abort()
        }
    }
}

