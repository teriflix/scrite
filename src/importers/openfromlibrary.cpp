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

#include "user.h"
#include "restapicall.h"
#include "openfromlibrary.h"
#include "networkaccessmanager.h"

#include <QApplication>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QNetworkRequest>

QNetworkAccessManager &LibraryNetworkAccess()
{
    return *NetworkAccessManager::instance();
}

LibraryService::LibraryService(QObject *parent) : AbstractImporter(parent)
{
    connect(this->screenplays(), &Library::busyChanged, this, &LibraryService::busyChanged);
    connect(this->templates(), &Library::busyChanged, this, &LibraryService::busyChanged);
    connect(this, &LibraryService::importStarted, this, &LibraryService::busyChanged);
    connect(this, &LibraryService::importFinished, this, &LibraryService::busyChanged);
}

LibraryService::~LibraryService() { }

bool LibraryService::busy() const
{
    return m_importing || this->screenplays()->isBusy() || this->templates()->isBusy();
}

Library *LibraryService::screenplays()
{
    static Library *theInstance = new Library(Library::Screenplays, qApp);
    return theInstance;
}

Library *LibraryService::templates()
{
    static Library *theInstance = new Library(Library::Templates, qApp);
    return theInstance;
}

void LibraryService::reload()
{
    this->screenplays()->reload();
    this->templates()->reload();
}

void LibraryService::openScreenplayAt(int index)
{
    this->openLibraryRecordAt(this->screenplays(), index);
}

void LibraryService::openTemplateAt(int index)
{
    this->openLibraryRecordAt(this->templates(), index);
}

class LibraryServiceOpenRecordTask : public QObject
{
public:
    LibraryServiceOpenRecordTask(Library *library, int index, LibraryService *parent = nullptr)
        : QObject(parent), m_index(index), m_library(library), m_parent(parent)
    {
        QTimer::singleShot(0, this, &LibraryServiceOpenRecordTask::start);
    }
    ~LibraryServiceOpenRecordTask() { }

private:
    void start();
    void openRecord();
    void complete();

    void recordFetched(const QString &name, const QByteArray &bytes);

private:
    int m_index = -1;
    Library *m_library = nullptr;
    LibraryService *m_parent = nullptr;
};

void LibraryService::openLibraryRecordAt(Library *library, int index)
{
    if (m_importing || library == nullptr)
        return;

    if (library != this->templates() && library != this->screenplays())
        return;

    new LibraryServiceOpenRecordTask(library, index, this);
}

bool LibraryService::doImport(QIODevice *device)
{
    Q_UNUSED(device);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

Library::Library(Library::Type type, QObject *parent) : QAbstractListModel(parent), m_type(type)
{
    this->reload();

    connect(RestApi::instance(), &RestApi::sessionTokenAvailable, this, &Library::reload);
}

Library::~Library() { }

QJsonObject Library::recordAt(int index) const
{
    if (index < 0 || index > m_records.size())
        return QJsonObject();

    return m_records.at(index).toObject();
}

int Library::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

QVariant Library::data(const QModelIndex &index, int role) const
{
    if (role == RecordRole)
        return this->recordAt(index.row());

    return QVariant();
}

QHash<int, QByteArray> Library::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[RecordRole] = "record";
    return roles;
}

void Library::reload()
{
    this->setRecords(QJsonArray());
    this->fetchRecords();
}

void Library::fetchRecords()
{
    if (m_busy || !User::instance()->isLoggedIn()
        || !RestApi::instance()->isSessionTokenAvailable())
        return;

    this->setBusy(true);

    AbstractScriptalayRestApiCall *call = m_type == Screenplays
            ? (AbstractScriptalayRestApiCall *)(new ScriptalayScreenplaysRestApiCall(this))
            : (AbstractScriptalayRestApiCall *)(new ScriptalayTemplatesRestApiCall(this));

    connect(call, &RestApiCall::finished, this, [=]() {
        if (call->hasError() || !call->hasResponse()) {
            this->setBusy(false);
            return;
        }

        m_baseUrl = call->baseUrl();
        emit baseUrlChanged();

        this->setRecords(call->records());
        this->setBusy(false);
    });

    if (!call->call()) {
        call->deleteLater();
        this->setBusy(false);
    }
}

void Library::setRecords(const QJsonArray &array)
{
    this->beginResetModel();
    m_records = array;
    if (m_type == Templates) {
        QJsonObject defaultTemplate;
        defaultTemplate.insert(QStringLiteral("name"), "Blank Document");
        defaultTemplate.insert(QStringLiteral("authors"), QStringLiteral("Scrite"));
        defaultTemplate.insert(QStringLiteral("poster"),
                               QStringLiteral("qrc:/icons/filetype/document.png"));
        defaultTemplate.insert(QStringLiteral("url_kind"), QStringLiteral("local"));
        defaultTemplate.insert(
                QStringLiteral("description"),
                QStringLiteral("A crisp and clean new document to write your next blockbuster!"));
        defaultTemplate.insert(QStringLiteral("more_info"), QLatin1String());
        m_records.prepend(defaultTemplate);
    }
    this->endResetModel();

    emit countChanged();
}

void Library::setBusy(bool val)
{
    if (m_busy == val)
        return;

    m_busy = val;

    if (val)
        qApp->setOverrideCursor(Qt::WaitCursor);
    else
        qApp->restoreOverrideCursor();

    emit busyChanged();
}

void LibraryServiceOpenRecordTask::start()
{
    m_parent->error()->clear();
    m_parent->progress()->start();

    m_parent->m_importing = true;

    emit m_parent->importStarted(m_index);

    QTimer::singleShot(100, this, &LibraryServiceOpenRecordTask::openRecord);
}

void LibraryServiceOpenRecordTask::openRecord()
{
    if (m_library == m_parent->templates() && m_index == 0) {
        // Blank document is being requested from templates lit
        ScriteDocument::instance()->reset();
        QTimer::singleShot(100, this, &LibraryServiceOpenRecordTask::complete);
        return;
    }

    QNetworkAccessManager &nam = ::LibraryNetworkAccess();

    const QJsonObject record = m_library->recordAt(m_index);
    const QString name = record.value("name").toString();

    QUrl url;
    if (record.value("url_kind").toString() == "relative")
        url = QUrl(m_library->baseUrl().toString() + "/" + record.value("url").toString());
    else
        url = QUrl(record.value("url").toString());

    const QNetworkRequest request(url);

    m_parent->progress()->setProgressText(QStringLiteral("Downloading \"") + name
                                          + QStringLiteral("\" from library..."));
    qApp->setOverrideCursor(Qt::WaitCursor);

    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        this->recordFetched(name, reply->readAll());
        reply->deleteLater();
    });

    const QString activity = m_library == m_parent->templates() ? QStringLiteral("template")
                                                                : QStringLiteral("scriptalay");
    User::instance()->logActivity2(activity, name);
}

void LibraryServiceOpenRecordTask::complete()
{
    m_parent->progress()->finish();

    m_parent->m_importing = false;

    emit m_parent->importFinished(m_index);

    qApp->restoreOverrideCursor();

    this->deleteLater();
}

void LibraryServiceOpenRecordTask::recordFetched(const QString &name, const QByteArray &bytes)
{
    if (bytes.isEmpty()) {
        m_parent->error()->setErrorMessage(QStringLiteral("Error downloading ") + name
                                           + QStringLiteral(". Please try again later."));
        QTimer::singleShot(0, this, &LibraryServiceOpenRecordTask::complete);
        return;
    }

    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    tmpFile.open();
    tmpFile.write(bytes);
    tmpFile.close();

    ScriteDocument::instance()->openAnonymously(tmpFile.fileName());
    ScriteDocument::instance()->screenplay()->setCurrentElementIndex(-1);

    if (m_library->type() == Library::Templates) {
        ScriteDocument::instance()->structure()->setCurrentElementIndex(-1);
        ScriteDocument::instance()->screenplay()->setCurrentElementIndex(
                ScriteDocument::instance()->screenplay()->firstSceneElementIndex());
        ScriteDocument::instance()->structure()->setForceBeatBoardLayout(true);

        ScriteDocument::instance()->printFormat()->resetToUserDefaults();
        ScriteDocument::instance()->formatting()->resetToUserDefaults();
    }

    if (m_library->type() == Library::Screenplays)
        ScriteDocument::instance()->setFromScriptalay(true);

    QTimer::singleShot(100, this, &LibraryServiceOpenRecordTask::complete);
}
