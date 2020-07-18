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

#include "libraryimporter.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

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

QNetworkAccessManager & LibraryNetworkAccess()
{
    static QNetworkAccessManager nam;
    return nam;
}

LibraryImporter::LibraryImporter(QObject *parent)
    : AbstractImporter(parent)
{

}

LibraryImporter::~LibraryImporter()
{

}

Library *LibraryImporter::library() const
{
    return Library::instance();
}

void LibraryImporter::importLibraryRecordAt(int index)
{
    if(m_importing)
        return;

    this->error()->clear();
    this->progress()->start();

    QNetworkAccessManager &nam = ::LibraryNetworkAccess();

    const QJsonObject record = Library::instance()->recordAt(index);
    const QString name = record.value("name").toString();

    QUrl url;
    if(record.value("url_kind").toString() == "relative")
        url = QUrl( Library::instance()->baseUrl().toString() + "/" + record.value("url").toString() );
    else
        url = QUrl(record.value("url").toString());

    const QNetworkRequest request(url);

    this->progress()->setProgressText( QStringLiteral("Downloading screenplay of \"") + name + QStringLiteral("\" from library...") );

    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        const QByteArray bytes = reply->readAll();
        reply->deleteLater();
        this->progress()->finish();

        if(bytes.isEmpty()) {
            this->error()->setErrorMessage( QStringLiteral("Error downloading ") + name + QStringLiteral(". Please try again later.") );
            return;
        }

        QTemporaryFile tmpFile;
        tmpFile.setAutoRemove(true);
        tmpFile.open();
        tmpFile.write(bytes);
        tmpFile.close();

        ScriteDocument::instance()->openAnonymously(tmpFile.fileName());
        ScriteDocument::instance()->screenplay()->setCurrentElementIndex(-1);

        emit imported(index);
    });
}

bool LibraryImporter::doImport(QIODevice *device)
{
    Q_UNUSED(device);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

Library *Library::instance()
{
    static Library *theInstance = new Library(qApp);
    return theInstance;
}

Library::Library(QObject *parent)
        :QAbstractListModel(parent)
{
    this->fetchRecords();
}

Library::~Library()
{

}

QJsonObject Library::recordAt(int index) const
{
    if(index < 0 || index > m_records.size())
        return QJsonObject();

    return m_records.at(index).toObject();
}

int Library::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

QVariant Library::data(const QModelIndex &index, int role) const
{
    if(role == RecordRole)
        return this->recordAt(index.row());

    return QVariant();
}

QHash<int, QByteArray> Library::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[RecordRole] = "record";
    return roles;
}

void Library::reload()
{
    this->setRecords( QJsonArray() );
    this->fetchRecords();
}

void Library::fetchRecords()
{
    if(m_busy)
        return;

    QNetworkAccessManager &nam = ::LibraryNetworkAccess();
    const QUrl url = QUrl( m_baseUrl.toString() + QStringLiteral("/records.json") );

    this->setBusy(true);

    const QNetworkRequest request(url);
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        const QByteArray bytes = reply->readAll();
        reply->deleteLater();
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
        if(error.error == QJsonParseError::NoError) {
            const QJsonObject object = doc.object();
            this->setRecords(object.value("records").toArray());
        }
        this->setBusy(false);
    });
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
#endif
    this->endResetModel();

    emit countChanged();
}

void Library::setBusy(bool val)
{
    if(m_busy == val)
        return;

    m_busy = val;
    emit busyChanged();
}

