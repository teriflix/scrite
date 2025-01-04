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

#include "application.h"
#include "simplecrypt.h"
#include "localstorage.h"

#include "restapikey/restapikey.h"

#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QSettings>

class EncryptedDataStore
{
public:
    EncryptedDataStore();
    ~EncryptedDataStore();

    QVariantMap data;

    void save();
};

EncryptedDataStore::EncryptedDataStore()
{
    const QString appDataFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFile file(QDir(appDataFolder).absoluteFilePath("localstore.db"));
    if (!file.open(QFile::ReadOnly))
        return;

    const QByteArray encryptedBytes = file.readAll();
    if (encryptedBytes.isEmpty())
        return;

    SimpleCrypt sc(REST_CRYPT_KEY);

    const QByteArray decryptedBytes = sc.decryptToByteArray(encryptedBytes);
    if (decryptedBytes.isEmpty())
        return;

    QDataStream ds(decryptedBytes);
    ds >> this->data;
}

EncryptedDataStore::~EncryptedDataStore() { }

void EncryptedDataStore::save()
{
    if (this->data.isEmpty())
        return;

    const QString appDataFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFile file(QDir(appDataFolder).absoluteFilePath("localstore.db"));
    if (!file.open(QFile::WriteOnly))
        return;

    const QByteArray decryptedBytes = [=]() {
        QVariantMap copy = this->data;
        copy.remove("sessionToken");

        QByteArray ret;
        QDataStream ds(&ret, QIODevice::WriteOnly);
        ds << copy;
        return ret;
    }();

    SimpleCrypt sc(REST_CRYPT_KEY);

    const QByteArray encryptedBytes = sc.encryptToByteArray(decryptedBytes);

    file.write(encryptedBytes);
}

Q_GLOBAL_STATIC(EncryptedDataStore, DataStore)

void LocalStorage::store(const QString &key, const QVariant &value)
{
    QVariantMap &data = ::DataStore->data;
    if (value.isValid())
        data.insert(key, value);
    else
        data.remove(key);

    ::DataStore->save();
}

QVariant LocalStorage::load(const QString &key, const QVariant &defaultValue)
{
    QVariantMap &data = ::DataStore->data;
    return data.value(key, defaultValue);
}

void LocalStorage::reset()
{
    ::DataStore->data.clear();
}

QJsonObject LocalStorage::compile(const QJsonObject &object)
{
    QJsonObject ret;
    if (object.isEmpty())
        return ret;

    QVariantMap &data = ::DataStore->data;

    QJsonObject::const_iterator it = object.constBegin();
    QJsonObject::const_iterator end = object.constEnd();
    while (it != end) {
        const QString key = it.key();

        QJsonValue value = it.value();
        if (value.isString()) {
            QString svalue = value.toString();
            if (svalue.startsWith('$')) {
                svalue = svalue.mid(1);
                value = QJsonValue::fromVariant(data.value(svalue));
            }
        }

        ret.insert(key, value);

        ++it;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

Q_GLOBAL_STATIC(QVariantMap, SessionVariables);
Q_GLOBAL_STATIC(QList<Session *>, Sessions);

Session::Session(QObject *parent) : QObject(parent)
{
    Sessions->append(this);
}

Session::~Session()
{
    Sessions->removeOne(this);
}

void Session::set(const QString &name, const QVariant &value)
{
    if (SessionVariables->value(name) != value) {
        if (value.isValid())
            SessionVariables->insert(name, value);
        else
            SessionVariables->remove(name);

        for (Session *session : *Sessions)
            emit session->changed(name, value);
    }
}

QVariant Session::get(const QString &name)
{
    return SessionVariables->value(name);
}

void Session::unset(const QString &name)
{
    Session::set(name, QVariant());
}
