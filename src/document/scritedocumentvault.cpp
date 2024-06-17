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

#include "scritedocumentvault.h"

#include "callgraph.h"
#include "application.h"
#include "timeprofiler.h"
#include "scritefileinfo.h"
#include "scritedocument.h"
#include "qobjectserializer.h"

#include <QDir>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFutureWatcher>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <QFileSystemWatcher>

ScriteDocumentVault *ScriteDocumentVault::instance()
{
    // CAPTURE_FIRST_CALL_GRAPH;
    static ScriteDocumentVault *theInstance = new ScriteDocumentVault(ScriteDocument::instance());
    return theInstance;
}

ScriteDocumentVault::ScriteDocumentVault(QObject *parent) : QAbstractListModel(parent)
{
    // CAPTURE_CALL_GRAPH;
    m_document = qobject_cast<ScriteDocument *>(parent);
    connect(m_document, &ScriteDocument::aboutToDelete, this, &ScriteDocumentVault::cleanup);
    connect(Application::instance(), &Application::aboutToQuit, this,
            &ScriteDocumentVault::cleanup);

    const QString vault = QStringLiteral("vault");
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
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
    for (const ScriteFileInfo &sfi : qAsConst(m_allFileInfoList))
        QFile::remove(sfi.fileInfo.absoluteFilePath());

    ++m_nrUnsavedChanges;
    this->updateModelFromFolderLater();
}

int ScriteDocumentVault::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_fileInfoList.size();
}

QVariant ScriteDocumentVault::data(const QModelIndex &index, int role) const
{
    const QVariant none;
    if (index.row() < 0 || index.row() >= m_fileInfoList.size())
        return none;

    const ScriteFileInfo sfi = m_fileInfoList.at(index.row());

    switch (role) {
    case TimestampRole:
        return sfi.fileInfo.lastModified().toMSecsSinceEpoch();
    case TimestampAsStringRole:
        return sfi.fileInfo.lastModified().toString(QStringLiteral("dd MMM yyyy @ hh:mm:ss"));
    case RelativeTimeRole:
        return Application::relativeTime(sfi.fileInfo.lastModified());
    case FileInfoRole:
        return QVariant::fromValue<ScriteFileInfo>(sfi);
    }

    return none;
}

QHash<int, QByteArray> ScriteDocumentVault::roleNames() const
{
    return { { TimestampRole, QByteArrayLiteral("timestamp") },
             { TimestampAsStringRole, QByteArrayLiteral("timestampAsString") },
             { RelativeTimeRole, QByteArrayLiteral("relativeTime") },
             { FileInfoRole, QByteArrayLiteral("fileInfo") } };
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
                                         QList<ScriteFileInfo> oldList) -> QList<ScriteFileInfo> {
        Q_UNUSED(currentDocumentId);

        QList<ScriteFileInfo> ret;
        const QFileInfoList fiList =
                QDir(folder).entryInfoList({ QStringLiteral("*.scrite") }, QDir::Files, QDir::Time);

        for (const QFileInfo &fi : fiList) {
            const int oldIndex = [oldList](const QFileInfo &fi) {
                for (int i = 0; i < oldList.size(); i++) {
                    const ScriteFileInfo &sfi = oldList[i];
                    if (sfi.fileInfo == fi)
                        return i;
                }
                return -1;
            }(fi);

            if (oldIndex >= 0)
                ret.append(oldList.takeAt(oldIndex));
            else {
                ScriteFileInfo sfi = ScriteFileInfo::load(fi.absoluteFilePath());
                if (sfi.title.isEmpty())
                    sfi.title = QStringLiteral("Untitled Screenplay");

                ret.append(sfi);
            }
        }

        return ret;
    };

    const QString futureWatcherName = QStringLiteral("ScriteDocumentVault::updateModelFromFolder");

    QFutureWatcher<QList<ScriteFileInfo>> *futureWatcher =
            this->findChild<QFutureWatcher<QList<ScriteFileInfo>> *>(futureWatcherName,
                                                                     Qt::FindDirectChildrenOnly);
    if (futureWatcher) {
        futureWatcher->cancel();
        futureWatcher->deleteLater();
    }

    futureWatcher = new QFutureWatcher<QList<ScriteFileInfo>>(this);
    connect(futureWatcher, &QFutureWatcher<QList<ScriteFileInfo>>::finished, this, [=]() {
        m_allFileInfoList = futureWatcher->result();
        this->prepareModel();
        futureWatcher->deleteLater();
    });

    const QString documentId = m_document ? m_document->documentId() : QString();
    const QFuture<QList<ScriteFileInfo>> future =
            QtConcurrent::run(fetchInfoAboutFilesInVault, documentId, m_folder, m_allFileInfoList);
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

    m_fileInfoList.clear();

    if (m_enabled) {
        const QString currentDocumentId = m_document ? m_document->documentId() : QString();
        std::copy_if(m_allFileInfoList.begin(), m_allFileInfoList.end(),
                     std::back_inserter(m_fileInfoList),
                     [currentDocumentId](const ScriteFileInfo &sfi) {
                         return sfi.documentId != currentDocumentId;
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
