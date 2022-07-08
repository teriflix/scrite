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

#ifndef OPENFROMLIBRARY_H
#define OPENFROMLIBRARY_H

#include <QJsonArray>
#include <QJsonObject>
#include <QAbstractListModel>

#include "abstractimporter.h"

class Library;

// This is a special case importer, that wont be created through
// the factory. So, we dont have to make it work like other factory
// based importers.
class LibraryService : public AbstractImporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Library")
    QML_ELEMENT

public:
    explicit LibraryService(QObject *parent = nullptr);
    ~LibraryService();

    // This this class cannot be used to import anything from a local file system
    bool canImport(const QString &) const { return false; }

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    bool busy() const;
    Q_SIGNAL void busyChanged();

    Q_PROPERTY(Library* screenplays READ screenplays CONSTANT)
    static Library *screenplays();

    Q_PROPERTY(Library* templates READ templates CONSTANT)
    static Library *templates();

    Q_INVOKABLE void reload();

    Q_INVOKABLE void openScreenplayAt(int index);
    Q_INVOKABLE void openTemplateAt(int index);
    Q_INVOKABLE void openLibraryRecordAt(Library *library, int index);

    // AbstractImporter interface
    bool doImport(QIODevice *device);

signals:
    void importStarted(int index);
    void importFinished(int index);

private:
    bool m_importing = false;
};

class Library : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~Library();

    enum Type { Screenplays, Templates };
    Q_ENUM(Type)

    Q_PROPERTY(Type type READ type CONSTANT)
    Type type() const { return m_type; }

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_records.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE QJsonObject recordAt(int index) const;

    Q_PROPERTY(QUrl baseUrl READ baseUrl NOTIFY baseUrlChanged)
    QUrl baseUrl() const { return m_baseUrl; }
    Q_SIGNAL void baseUrlChanged();

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    // QAbstractItemModel interface
    enum Roles { RecordRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void reload();

private:
    friend class LibraryService;
    Library(Type type, QObject *parent = nullptr);

    void fetchRecords();
    void setRecords(const QJsonArray &array);
    void setBusy(bool val);

private:
    Type m_type = Screenplays;
    bool m_busy = false;
    QJsonArray m_records;
    QUrl m_baseUrl;
};

#endif
