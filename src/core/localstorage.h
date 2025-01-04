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

#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <QString>
#include <QVariant>
#include <QQmlEngine>
#include <QJsonObject>

class LocalStorage
{
public:
    static void store(const QString &key, const QVariant &value);
    static QVariant load(const QString &key, const QVariant &defaultValue = QVariant());
    static void reset();

    static QJsonObject compile(const QJsonObject &object);
};

class Session : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    Session(QObject *parent = nullptr);
    ~Session();

    Q_INVOKABLE static void set(const QString &name, const QVariant &value);
    Q_INVOKABLE static QVariant get(const QString &name);
    Q_INVOKABLE static void unset(const QString &name);

    Q_SIGNAL void changed(const QString &name, const QVariant &value);
};

#endif // LOCALSTORAGE_H
