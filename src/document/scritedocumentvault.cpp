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

#include "scritedocumentvault.h"

#include "application.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "qobjectserializer.h"

#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtConcurrentRun>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QSettings>

ScriteDocumentVault *ScriteDocumentVault::instance()
{
    static ScriteDocumentVault *theInstance = new ScriteDocumentVault(ScriteDocument::instance());
    return theInstance;
}

ScriteDocumentVault::ScriteDocumentVault(QObject *parent) : QAbstractListModel(parent)
{
    m_document = qobject_cast<ScriteDocument *>(parent);
    connect(m_document, &ScriteDocument::aboutToDelete, this, &ScriteDocumentVault::cleanup);
    connect(Application::instance(), &Application::aboutToQuit, this,
            &ScriteDocumentVault::cleanup);

    const QString settingsPath = Application::instance()->settingsFilePath();
    const QString vault = QStringLiteral("vault");

    QDir dir = QFileInfo(settingsPath).dir();
    if (!dir.cd(vault)) {
        dir.mkdir(vault);
        dir.cd(vault);
    }

    m_folder = dir.absolutePath();
    this->updateModelFromFolder();

    m_folderWatcher = new QFileSystemWatcher(this);
    m_folderWatcher->addPath(m_folder);
    connect(m_folderWatcher, &QFileSystemWatcher::directoryChanged, this,
            &ScriteDocumentVault::updateModelFromFolderLater);

    m_saveToVaultTimer.setInterval(2000);
    m_saveToVaultTimer.setSingleShot(true);
    connect(&m_saveToVaultTimer, &QTimer::timeout, this, &ScriteDocumentVault::saveToVault);

    connect(m_document, &ScriteDocument::documentChanged, this,
            &ScriteDocumentVault::onDocumentChanged);
    connect(m_document, &ScriteDocument::aboutToReset, this,
            &ScriteDocumentVault::onDocumentAboutToReset);
    connect(m_document, &ScriteDocument::justReset, this,
            &ScriteDocumentVault::onDocumentJustReset);
    connect(m_document, &ScriteDocument::justSaved, this,
            &ScriteDocumentVault::onDocumentJustSaved);
    connect(m_document, &ScriteDocument::justLoaded, this,
            &ScriteDocumentVault::onDocumentJustLoaded);
    connect(m_document, &ScriteDocument::documentIdChanged, this,
            &ScriteDocumentVault::prepareModel);

    connect(this, &ScriteDocumentVault::rowsInserted, this,
            &ScriteDocumentVault::documentCountChanged);
    connect(this, &ScriteDocumentVault::rowsRemoved, this,
            &ScriteDocumentVault::documentCountChanged);
    connect(this, &ScriteDocumentVault::modelReset, this,
            &ScriteDocumentVault::documentCountChanged);

    qApp->installEventFilter(this);

    QSettings *settings = Application::instance()->settings();
    m_enabled = settings->value(QStringLiteral("Installation/vaultEnabled"), true).toBool();

    this->pauseSaveToVault();
}

ScriteDocumentVault::~ScriteDocumentVault()
{
    this->cleanup();
}

void ScriteDocumentVault::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    if (val)
        m_saveToVaultTimer.start();
    else
        m_saveToVaultTimer.stop();

    QSettings *settings = Application::instance()->settings();
    settings->setValue(QStringLiteral("Installation/vaultEnabled"), val);

    this->prepareModel();
}

void ScriteDocumentVault::clearAllDocuments()
{
    for (const MetaData &data : qAsConst(m_allMetaDataList))
        QFile::remove(data.fileInfo.absoluteFilePath());

    ++m_nrUnsavedChanges;
    this->updateModelFromFolderLater();
}

int ScriteDocumentVault::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_metaDataList.size();
}

QVariant ScriteDocumentVault::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_metaDataList.size())
        return QVariant();

    const MetaData metaData = m_metaDataList.at(index.row());

    switch (role) {
    case TimestampRole:
        return metaData.fileInfo.lastModified().toMSecsSinceEpoch();
    case TimestampAsStringRole:
        return metaData.fileInfo.lastModified().toString();
    case RelativeTimeRole:
        return Application::relativeTime(metaData.fileInfo.lastModified());
    case FileNameRole:
        return metaData.fileInfo.fileName();
    case FilePathRole:
        return metaData.fileInfo.absoluteFilePath();
    case FileSizeRole:
        return metaData.fileInfo.size();
    case ScreenplayTitleRole:
        return metaData.screenplayTitle;
    case NumberOfScenesRole:
        return metaData.numberOfScenes;
    }

    return QVariant();
}

QHash<int, QByteArray> ScriteDocumentVault::roleNames() const
{
    static const QHash<int, QByteArray> roles(
            { { TimestampRole, QByteArrayLiteral("timestamp") },
              { TimestampAsStringRole, QByteArrayLiteral("timestampAsString") },
              { RelativeTimeRole, QByteArrayLiteral("relativeTime") },
              { FileNameRole, QByteArrayLiteral("fileName") },
              { FilePathRole, QByteArrayLiteral("filePath") },
              { FileSizeRole, QByteArrayLiteral("fileSize") },
              { ScreenplayTitleRole, QByteArrayLiteral("screenplayTitle") },
              { NumberOfScenesRole, QByteArrayLiteral("numberOfScenes") } });
    return roles;
}

void ScriteDocumentVault::onDocumentAboutToReset()
{
    this->saveToVault();
}

void ScriteDocumentVault::onDocumentJustReset()
{
    this->pauseSaveToVault();
}

void ScriteDocumentVault::onDocumentJustSaved()
{
    const QString fileName = this->vaultFilePath();
    QFile::remove(fileName);
    m_saveToVaultTimer.stop();
    this->updateModelFromFolderLater();
}

void ScriteDocumentVault::onDocumentJustLoaded()
{
    this->pauseSaveToVault();
}

void ScriteDocumentVault::onDocumentChanged()
{
    if (m_document != nullptr) {
        ++m_nrUnsavedChanges;
        m_saveToVaultTimer.start();
    } else
        m_saveToVaultTimer.stop();
}

void ScriteDocumentVault::saveToVault()
{
    m_saveToVaultTimer.stop();

    if (m_nrUnsavedChanges <= 0 || !m_enabled)
        return;

    m_nrUnsavedChanges = 0;

    if (m_document == nullptr)
        return;

    if (m_document->isEmpty() || m_document->isFromScriptalay())
        return;

    if (m_document->fileName().isEmpty() || !m_document->isAutoSave()) {
        DocumentFileSystem *dfs = m_document->fileSystem();

        const QString fileName = this->vaultFilePath();
        const QJsonObject json = [=]() {
            QJsonObject ret = QObjectSerializer::toJson(m_document);
            ret.insert(QStringLiteral("$sourceFileName"), m_document->fileName());
            return ret;
        }();
        const QByteArray bytes = QJsonDocument(json).toJson();
        const bool encrypt = m_document->hasCollaborators();
        dfs->setHeader(bytes);
        dfs->save(fileName, encrypt);

        this->updateModelFromFolderLater();
    }
}

void ScriteDocumentVault::cleanup()
{
    if (m_document == nullptr)
        return;

    qApp->removeEventFilter(this);
    this->saveToVault();
    m_document = nullptr;
}

void ScriteDocumentVault::updateModelFromFolder()
{
    auto fetchInfoAboutFilesInVault = [](const QString &currentDocumentId, const QString &folder,
                                         QList<MetaData> oldMetaDataList) -> QList<MetaData> {
        QList<MetaData> ret;
        const QFileInfoList fiList =
                QDir(folder).entryInfoList({ QStringLiteral("*.scrite") }, QDir::Files, QDir::Time);

        for (const QFileInfo &fi : fiList) {
            const int oldIndex = [oldMetaDataList](const QFileInfo &fi) {
                for (int i = 0; i < oldMetaDataList.size(); i++) {
                    const MetaData &md = oldMetaDataList[i];
                    if (md.fileInfo == fi)
                        return i;
                }
                return -1;
            }(fi);

            if (oldIndex >= 0)
                ret.append(oldMetaDataList.takeAt(oldIndex));
            else {
                MetaData metaData;
                metaData.fileInfo = fi;

                DocumentFileSystem dfs;
                if (dfs.load(fi.absoluteFilePath())) {
                    const QJsonDocument jsonDoc = QJsonDocument::fromJson(dfs.header());
                    const QJsonObject docObj = jsonDoc.object();

                    metaData.documentId = docObj.value(QStringLiteral("documentId")).toString();

                    const QJsonObject structure =
                            docObj.value(QStringLiteral("structure")).toObject();
                    metaData.numberOfScenes =
                            structure.value(QStringLiteral("elements")).toArray().size();

                    const QJsonObject screenplay =
                            docObj.value(QStringLiteral("screenplay")).toObject();
                    metaData.screenplayTitle =
                            screenplay.value(QStringLiteral("title")).toString().trimmed();
                    if (metaData.screenplayTitle.isEmpty())
                        metaData.screenplayTitle = QStringLiteral("Untitled Screenplay");
                }

                ret.append(metaData);
            }
        }

        return ret;
    };

    const QString futureWatcherName = QStringLiteral("ScriteDocumentVault::updateModelFromFolder");

    QFutureWatcher<QList<MetaData>> *futureWatcher =
            this->findChild<QFutureWatcher<QList<MetaData>> *>(futureWatcherName,
                                                               Qt::FindDirectChildrenOnly);
    if (futureWatcher) {
        futureWatcher->cancel();
        futureWatcher->deleteLater();
    }

    futureWatcher = new QFutureWatcher<QList<MetaData>>(this);
    connect(futureWatcher, &QFutureWatcher<QList<MetaData>>::finished, this, [=]() {
        m_allMetaDataList = futureWatcher->result();
        this->prepareModel();
        futureWatcher->deleteLater();
    });

    const QString documentId = m_document ? m_document->documentId() : QString();
    const QFuture<QList<MetaData>> future =
            QtConcurrent::run(fetchInfoAboutFilesInVault, documentId, m_folder, m_allMetaDataList);
    futureWatcher->setFuture(future);
}

void ScriteDocumentVault::updateModelFromFolderLater()
{
    ExecLaterTimer::call("updateModelFromFolderLater", this,
                         [=]() { this->updateModelFromFolder(); });
}

QString ScriteDocumentVault::vaultFilePath() const
{
    const QString id = m_document == nullptr ? QStringLiteral("unknown") : m_document->documentId();
    return QDir(m_folder).absoluteFilePath(id + QStringLiteral(".scrite"));
}

void ScriteDocumentVault::pauseSaveToVault(int timeout)
{
    m_nrUnsavedChanges = -100000;
    ExecLaterTimer::call(
            "pauseSaveToVaultTimer", this, [=]() { m_nrUnsavedChanges = 0; }, timeout);
}

void ScriteDocumentVault::prepareModel()
{
    this->beginResetModel();

    m_metaDataList.clear();

    if (m_enabled) {
        const QString currentDocumentId = m_document ? m_document->documentId() : QString();
        std::copy_if(m_allMetaDataList.begin(), m_allMetaDataList.end(),
                     std::back_inserter(m_metaDataList), [currentDocumentId](const MetaData &md) {
                         return md.documentId != currentDocumentId;
                     });
    }

    this->endResetModel();
}

bool ScriteDocumentVault::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::KeyPress:
        if (m_saveToVaultTimer.isActive())
            m_saveToVaultTimer.start();
        break;
    default:
        break;
    }

    return false;
}
