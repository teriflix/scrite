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

#ifndef SCRITEDOCUMENTVAULT_H
#define SCRITEDOCUMENTVAULT_H

#include <QTimer>
#include <QQmlEngine>
#include <QFileInfoList>
#include <QAbstractItemModel>

#include "scritefileinfo.h"

class ScriteDocument;
class QFileSystemWatcher;

class ScriteDocumentVault : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static ScriteDocumentVault *instance();
    ~ScriteDocumentVault();

    // clang-format off
    Q_PROPERTY(bool busy
               READ busy
               NOTIFY busyChanged)
    // clang-format on
    bool busy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    // clang-format off
    Q_PROPERTY(QString folder
               READ folder
               CONSTANT )
    // clang-format on
    QString folder() const { return m_folder; }

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // clang-format off
    Q_PROPERTY(int documentCount
               READ documentCount
               NOTIFY documentCountChanged)
    // clang-format on
    int documentCount() const { return m_fileInfoList.size(); }
    Q_SIGNAL void documentCountChanged();

    Q_INVOKABLE void clearAllDocuments();

    // QAbstractItemModel interface
    enum Roles {
        TimestampRole = Qt::UserRole,
        TimestampAsStringRole,
        RelativeTimeRole,
        FileInfoRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    bool eventFilter(QObject *watched, QEvent *event);

private:
    ScriteDocumentVault(QObject *parent = nullptr);

    void setBusy(bool val);

    void onDocumentAboutToReset();
    void onDocumentJustReset();
    void onDocumentJustSaved();
    void onDocumentJustLoaded();
    void onDocumentChanged();
    void saveToVault();
    void cleanup();
    void updateModelFromFolder();
    void updateModelFromFolderLater();

    QString vaultFilePath() const;
    void pauseSaveToVault(int timeout = 2100);

    void prepareModel();

private:
    bool m_busy = false;
    bool m_enabled = true;
    QString m_folder;
    QTimer m_saveToVaultTimer;
    int m_nrUnsavedChanges = 0;
    ScriteDocument *m_document = nullptr;
    QFileSystemWatcher *m_folderWatcher = nullptr;

    QList<ScriteFileInfo> m_allFileInfoList; // including current document
    QList<ScriteFileInfo> m_fileInfoList; // excluding current document
};

#endif // SCRITEDOCUMENTVAULT_H
