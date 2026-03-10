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
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components


import "../globals"
import "../helpers"
import "../dialogs"
import "../controls"
import "../notifications"

Item {
    id: root

    parent: Scrite.window.contentItem

    function save(callback) {
        var taskComp = _private.getTaskComponent()
        return taskComp.createObject(root, {"callback": callback})
    }

    function saveSilently(callback) {
        var taskComp = _private.getTaskComponent()
        return taskComp.createObject(root, {"callback": callback, "silent": true})
    }

    function saveAs(callback) {
        if(!ActionHub.isOperationAllowedByUser("Save As operation")) {
            return false
        }

        return _documentSaveAs.createObject(root, {"callback": callback})
    }

    component AbstractSaveFileTask : Item {
        id: _saveFileTaskItem

        property bool silent: false
        property var callback

        function finish(success) {
            if(!errorReportHasError) {
                if((success === undefined || success === true) && callback)
                    callback()
            }

            Qt.callLater(destroy)
        }

        property ErrorReport errorReport: Aggregation.errorReport(Scrite.document)
        property bool errorReportHasError: errorReport.hasError
    }

    Component {
        id: _documentIsEmptyOrHasNoChanges

        AbstractSaveFileTask {
            id: _documentIsEmptyOrHasNoChangesTask

            // Since there is nothing to save, we can finish up by invoking the callback
            // and destroying self.
            Component.onCompleted: {
                finish(true)
            }
        }
    }

    Component {
        id: _documentNotEvenSavedOnce

        AbstractSaveFileTask {
            id: _documentNotEvenSavedOnceTask

            // The document has not been saved even once, so we will need to save it now.
            Component.onCompleted: {
                if(silent)
                    questionAnswered("Yes")
                else
                    MessageBox.question("Save Confirmation",
                                        "The current document is not saved to disk. Do you want to save it now?",
                                        ["Yes", "No", "Cancel"],
                                        questionAnswered);
            }

            function questionAnswered(answer) {
                if(answer === "Yes") {
                    var saveDlg = _saveFileDialog.createObject(root)
                    saveDlg.finished.connect(finish)
                    saveDlg.finished.connect(saveDlg.destroy)
                    saveDlg.open()
                } else if(answer === "No") {
                    finish(true)
                } else
                    finish(false)
            }
        }
    }

    Component {
        id: _documentSaveAs

        AbstractSaveFileTask {
            id: _documentSaveAsTask

            Component.onCompleted: {
                var saveDlg = _saveFileDialog.createObject(root)
                saveDlg.finished.connect(finish)
                saveDlg.finished.connect(saveDlg.destroy)
                saveDlg.open()
            }
        }
    }

    Component {
        id: _autoSaveIsEnabled

        AbstractSaveFileTask {
            id: _autoSaveIsEnabledItem

            // The document has already been saved to disk, and auto save is enabled.
            // So we simply auto-save the document and move on.
            Component.onCompleted: {
                Scrite.document.save()

                finish(true)
            }
        }
    }

    Component {
        id: _documentSavedButHasChanges

        AbstractSaveFileTask {
            id: _documentSavedButHasChangesItem

            property BasicFileInfo fileInfo

            Component.onCompleted: {
                fileInfo = Qt.createQmlObject("import io.scrite.components 1.0; BasicFileInfo { }", _documentSavedButHasChangesItem)
                fileInfo.absoluteFilePath = Scrite.document.fileName

                if(silent)
                    questionAnswered("Yes")
                else
                    MessageBox.question("Save Confirmation",
                                        "Do you want to save changes to '" + fileInfo.baseName + "'?",
                                        ["Yes", "No", "Cancel"],
                                        questionAnswered);
            }

            function questionAnswered(answer) {
                if(answer === "Yes") {
                    Scrite.document.save()
                    finish(true)
                } else if(answer === "No") {
                    finish(true)
                } else
                    finish(false)
            }
        }
    }

    Component {
        id: _saveFileDialog

        VclFileDialog {
            title: "Save Scrite Document As"
            nameFilters: ["Scrite Documents (*.scrite)"]
            objectName: "Save File Dialog"
            
            currentFolder: Runtime.workspaceSettings.lastOpenFolderUrl
            onCurrentFolderChanged: Runtime.workspaceSettings.lastOpenFolderUrl = folder

            signal finished(bool success)

            onAccepted: {
                const path = Url.toPath(selectedFile)
                if(Scrite.document.canBeBackupFileName(path)) {
                    _private.reportSaveAsBackupNotPossible()
                    finished(false)
                    return
                }

                Scrite.document.saveAs(path)

                const fileInfo = File.info(path)
                Runtime.workspaceSettings.lastOpenFolderUrl = folder

                finished(true)

                Runtime.showHelpTip("syncFiles")
            }

            onRejected: finished(false)
        }
    }

    QtObject {
        id: _private

        function getTaskComponent() {
            if(Scrite.document.empty || !Scrite.document.modified || Scrite.document.readOnly)
                return _documentIsEmptyOrHasNoChanges

            if(Scrite.document.fileName === "")
                return _documentNotEvenSavedOnce

            if(Scrite.document.autoSave)
                return _autoSaveIsEnabled

            return _documentSavedButHasChanges
        }

        function reportSaveAsBackupNotPossible() {
            MessageBox.information("Save Error", "Cannot save as a backup file. Please choose another path or file name.", () => {
                                        Runtime.execLater(root, 100, root.saveAs)
                                   })
        }
    }
}

