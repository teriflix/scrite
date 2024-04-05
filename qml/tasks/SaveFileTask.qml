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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

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
        return documentSaveAs.createObject(root, {"callback": callback})
    }

    component AbstractSaveFileTask : Item {
        id: saveFileTaskItem

        property bool silent: false
        property var callback

        function finish(success) {
            if(!errorReportHasError) {
                if((success === undefined || success === true) && callback)
                    callback()
            }

            Qt.callLater(destroy)
        }

        property ErrorReport errorReport: Aggregation.findErrorReport(Scrite.document)
        property bool errorReportHasError: errorReport.hasError
    }

    Component {
        id: documentIsEmptyOrHasNoChanges

        AbstractSaveFileTask {
            id: documentIsEmptyOrHasNoChangesTask

            // Since there is nothing to save, we can finish up by invoking the callback
            // and destroying self.
            Component.onCompleted: {
                finish(true)
            }
        }
    }

    Component {
        id: documentNotEvenSavedOnce

        AbstractSaveFileTask {
            id: documentNotEvenSavedOnceTask

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
                    var saveDlg = saveFileDialog.createObject(root)
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
        id: documentSaveAs

        AbstractSaveFileTask {
            id: documentSaveAsTask

            Component.onCompleted: {
                var saveDlg = saveFileDialog.createObject(root)
                saveDlg.finished.connect(finish)
                saveDlg.finished.connect(saveDlg.destroy)
                saveDlg.open()
            }
        }
    }

    Component {
        id: autoSaveIsEnabled

        AbstractSaveFileTask {
            id: autoSaveIsEnabledItem

            // The document has already been saved to disk, and auto save is enabled.
            // So we simply auto-save the document and move on.
            Component.onCompleted: {
                var fileInfo = Qt.createQmlObject("import io.scrite.components 1.0; BasicFileInfo { }", autoSaveIsEnabledItem)
                fileInfo.absoluteFilePath = Scrite.document.fileName

                Scrite.document.save()

                finish(true)
            }
        }
    }

    Component {
        id: documentSavedButHasChanges

        AbstractSaveFileTask {
            id: documentSavedButHasChangesItem

            property BasicFileInfo fileInfo

            Component.onCompleted: {
                fileInfo = Qt.createQmlObject("import io.scrite.components 1.0; BasicFileInfo { }", documentSavedButHasChangesItem)
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
        id: saveFileDialog

        FileDialog {
            title: "Save Scrite Document As"
            nameFilters: ["Scrite Documents (*.scrite)"]
            selectFolder: false
            selectMultiple: false
            objectName: "Save File Dialog"
            dirUpAction.shortcut: "Ctrl+Shift+U"
            folder: Runtime.workspaceSettings.lastOpenFolderUrl
            onFolderChanged: Runtime.workspaceSettings.lastOpenFolderUrl = folder
            sidebarVisible: true
            selectExisting: false

            signal finished(bool success)

            onAccepted: {
                const path = Scrite.app.urlToLocalFile(fileUrl)
                Scrite.document.saveAs(path)

                const fileInfo = Scrite.app.fileInfo(path)
                Runtime.workspaceSettings.lastOpenFolderUrl = folder

                finished(true)
            }

            onRejected: finished(false)
        }
    }

    QtObject {
        id: _private

        function getTaskComponent() {
            if(Scrite.document.empty || !Scrite.document.modified || Scrite.document.readOnly)
                return documentIsEmptyOrHasNoChanges

            if(Scrite.document.fileName === "")
                return documentNotEvenSavedOnce

            if(Scrite.document.autoSave)
                return autoSaveIsEnabled

            return documentSavedButHasChanges
        }
    }
}

