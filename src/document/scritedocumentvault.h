/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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

    Q_PROPERTY(QString folder READ folder CONSTANT)
    QString folder() const { return m_folder; }

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(int documentCount READ documentCount NOTIFY documentCountChanged)
    int documentCount() const { return m_metaDataList.size(); }
    Q_SIGNAL void documentCountChanged();

    Q_INVOKABLE void clearAllDocuments();

    // QAbstractItemModel interface
    enum Roles {
        TimestampRole = Qt::UserRole,
        TimestampAsStringRole,
        RelativeTimeRole,
        FileNameRole,
        FilePathRole,
        FileSizeRole,
        ScreenplayTitleRole,
        NumberOfScenesRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    bool eventFilter(QObject *watched, QEvent *event);

private:
    ScriteDocumentVault(QObject *parent = nullptr);

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
    bool m_enabled = true;
    QString m_folder;
    QTimer m_saveToVaultTimer;
    int m_nrUnsavedChanges = 0;
    ScriteDocument *m_document = nullptr;
    QFileSystemWatcher *m_folderWatcher = nullptr;

    struct MetaData
    {
        QString documentId;
        QFileInfo fileInfo;
        QString screenplayTitle;
        int numberOfScenes = 0;
    };
    QList<MetaData> m_allMetaDataList; // including current document
    QList<MetaData> m_metaDataList; // excluding current document
};

#endif // SCRITEDOCUMENTVAULT_H
