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

#include "user.h"
#include "openfromlibrary.h"
#include "networkaccessmanager.h"

#include <QApplication>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QNetworkRequest>

/*
We will host a JSON file at http://www.teriflix.in/scrite/library.json.
This file will be of the form..

{
    "records": [
        {
            "name": "...",
            "authors": "...",
            "revision": "...",
            "poster": "....",
            "url": "http://....",
            "source": "...",
            "copyright": "....",
            "contributed_by": "....",
            "logline": "....",
            "version": "x.y.z"
        },
        ...
    ]
}

All documents must be available under a plain http access (not https). Lets just avoid
all SSL certificate validation bits for now. The data transferred is not mission critical
anyway.
*/

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

void LibraryService::openLibraryRecordAt(Library *library, int index)
{
    if (m_importing || library == nullptr)
        return;

    if (library != this->templates() && library != this->screenplays())
        return;

    this->error()->clear();
    this->progress()->start();

    m_importing = true;
    emit importStarted(index);

    if (library == this->templates() && index == 0) {
        ScriteDocument::instance()->reset();
        this->progress()->finish();

        m_importing = false;
        emit importFinished(0);
        return;
    }

    QNetworkAccessManager &nam = ::LibraryNetworkAccess();

    const QJsonObject record = library->recordAt(index);
    const QString name = record.value("name").toString();

    QUrl url;
    if (record.value("url_kind").toString() == "relative")
        url = QUrl(library->baseUrl().toString() + "/" + record.value("url").toString());
    else
        url = QUrl(record.value("url").toString());

    const QNetworkRequest request(url);

    this->progress()->setProgressText(QStringLiteral("Downloading \"") + name
                                      + QStringLiteral("\" from library..."));
    qApp->setOverrideCursor(Qt::WaitCursor);

    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        const QByteArray bytes = reply->readAll();
        reply->deleteLater();

        if (bytes.isEmpty()) {
            this->progress()->finish();
            qApp->restoreOverrideCursor();
            this->error()->setErrorMessage(QStringLiteral("Error downloading ") + name
                                           + QStringLiteral(". Please try again later."));
            return;
        }

        QTemporaryFile tmpFile;
        tmpFile.setAutoRemove(true);
        tmpFile.open();
        tmpFile.write(bytes);
        tmpFile.close();

        ScriteDocument::instance()->openAnonymously(tmpFile.fileName());
        ScriteDocument::instance()->screenplay()->setCurrentElementIndex(-1);

        if (library->type() == Library::Templates) {
            ScriteDocument::instance()->structure()->setCurrentElementIndex(-1);
            ScriteDocument::instance()->screenplay()->setCurrentElementIndex(
                    ScriteDocument::instance()->screenplay()->firstSceneIndex());
            ScriteDocument::instance()->structure()->setForceBeatBoardLayout(true);

            ScriteDocument::instance()->printFormat()->resetToUserDefaults();
            ScriteDocument::instance()->formatting()->resetToUserDefaults();
        }

        if (library == this->screenplays())
            ScriteDocument::instance()->setFromScriptalay(true);

        this->progress()->finish();
        qApp->restoreOverrideCursor();

        m_importing = false;
        emit importFinished(index);
    });

    const QString activity = library == this->templates() ? QStringLiteral("template")
                                                          : QStringLiteral("scriptalay");
    User::instance()->logActivity2(activity, name);
}

bool LibraryService::doImport(QIODevice *device)
{
    Q_UNUSED(device);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

Library::Library(Library::Type type, QObject *parent) : QAbstractListModel(parent), m_type(type)
{
    this->setRecords(QJsonArray());
    this->fetchRecords();
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
    if (m_busy)
        return;

    const QString path = m_type == Screenplays ? QStringLiteral("/records.hexdb")
                                               : QStringLiteral("/templates.hexdb");

    QNetworkAccessManager &nam = ::LibraryNetworkAccess();
    const QUrl url = QUrl(m_baseUrl.toString() + path);

    this->setBusy(true);

    const QNetworkRequest request(url);
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        const QByteArray bytes = reply->readAll();
        reply->deleteLater();
        this->loadDatabase(bytes);
        this->setBusy(false);
    });
}

void Library::loadDatabase(const QByteArray &bytes)
{
    const QByteArray bson = qUncompress(QByteArray::fromHex(bytes));

    const QJsonDocument doc = QJsonDocument::fromBinaryData(bson);
    if (doc.isNull())
        return;

    const QJsonObject object = doc.object();
    const QJsonArray records = object.value("records").toArray();
    this->setRecords(records);
}

void Library::setRecords(const QJsonArray &array)
{
    this->beginResetModel();
#if 0
    for(int j=0; j<10; j++)
    {
        for(int i=0; i<array.size(); i++)
            m_records.append( array.at(i) );
    }
#else
    m_records = array;
    if (m_type == Templates) {
        QJsonObject defaultTemplate;
        defaultTemplate.insert(QStringLiteral("name"), "Blank Document");
        defaultTemplate.insert(QStringLiteral("authors"), QStringLiteral("Scrite"));
        defaultTemplate.insert(QStringLiteral("poster"),
                               QStringLiteral("qrc:/images/blank_document.png"));
        defaultTemplate.insert(QStringLiteral("url_kind"), QStringLiteral("local"));
        defaultTemplate.insert(QStringLiteral("description"),
                               QStringLiteral("An empty Scrite document."));
        defaultTemplate.insert(QStringLiteral("more_info"), QLatin1String());
        m_records.prepend(defaultTemplate);
    }
#endif
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
