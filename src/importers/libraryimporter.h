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

#ifndef LIBRARYIMPORTER_H
#define LIBRARYIMPORTER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QAbstractListModel>

#include "abstractimporter.h"

class Library;

// This is a special case importer, that wont be created through
// the factory. So, we dont have to make it work like other factory
// based importers.
class LibraryImporter : public AbstractImporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Library")

public:
    LibraryImporter(QObject *parent=nullptr);
    ~LibraryImporter();

    Q_PROPERTY(Library* library READ library CONSTANT)
    Library* library() const;

    Q_INVOKABLE void importLibraryRecordAt(int index);

    // AbstractImporter interface
    bool doImport(QIODevice *device);

signals:
    void imported(int index);

private:
    bool m_importing = false;
};

class Library : public QAbstractListModel
{
    Q_OBJECT

private:
    Library(QObject *parent=nullptr);

public:
    static Library *instance();
    ~Library();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_records.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE QJsonObject recordAt(int index) const;

    Q_PROPERTY(QUrl baseUrl READ baseUrl CONSTANT)
    QUrl baseUrl() const { return m_baseUrl; }

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    // QAbstractItemModel interface
    enum Roles { RecordRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    Q_INVOKABLE void reload();

private:
    void fetchRecords();
    void setRecords(const QJsonArray &array);
    void setBusy(bool val);

private:
    bool m_busy = false;
    QJsonArray m_records;
    const QUrl m_baseUrl = QUrl( QStringLiteral("http://www.teriflix.in/scrite/library/") );
};

#endif
