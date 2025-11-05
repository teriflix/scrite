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

#ifndef ENUMERATIONMODEL_H
#define ENUMERATIONMODEL_H

#include <QMetaEnum>
#include <QQmlEngine>
#include <QAbstractItemModel>

/*
We need a way to access QMetaEnum like a model from QML code. This class is meant for that.
*/
class EnumerationModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit EnumerationModel(QObject *parent = nullptr);
    explicit EnumerationModel(const QMetaObject *mo, const QString &enumName,
                              QObject *parent = nullptr);
    ~EnumerationModel();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_items.size(); }
    Q_SIGNAL void countChanged();

    Q_PROPERTY(QList<int> ignoreList READ ignoreList WRITE setIgnoreList NOTIFY ignoreListChanged)
    void setIgnoreList(QList<int> val);
    QList<int> ignoreList() const { return m_ignoreList; }
    Q_SIGNAL void ignoreListChanged();

    Q_PROPERTY(QString className READ className WRITE setClassName NOTIFY classNameChanged)
    void setClassName(const QString &val);
    QString className() const;
    Q_SIGNAL void classNameChanged();

    Q_PROPERTY(QObject* object READ object WRITE setObject NOTIFY objectChanged)
    void setObject(QObject *val);
    QObject *object() const { return m_object; }
    Q_SIGNAL void objectChanged();

    Q_PROPERTY(QString enumeration READ enumeration WRITE setEnumeration NOTIFY enumerationChanged)
    void setEnumeration(const QString &val);
    QString enumeration() const { return m_enumeration; }
    Q_SIGNAL void enumerationChanged();

    Q_INVOKABLE QString valueToKey(int value) const;
    Q_INVOKABLE QString valueToIcon(int value) const;
    Q_INVOKABLE int keyToValue(const QString &key) const;
    Q_INVOKABLE int indexOfValue(int value) const;

    // QAbstractItemModel interface
    enum { KeyRole = Qt::UserRole, ValueRole, IconRole };
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    // QQmlParserStatus interface
    void classBegin() { m_componentComplete = false; }
    void componentComplete();

private:
    void resetObject();
    void loadModel();
    void clearModel();

private:
    const QMetaObject *m_metaObject = nullptr;
    QObject *m_object = nullptr;
    QString m_enumeration;

    struct Item
    {
        QString key;
        int value = -1;
        QString icon;
    };
    QList<Item> m_items;
    QMetaEnum m_metaEnum;
    QList<int> m_ignoreList;
    bool m_componentComplete = true;
};

#endif // ENUMERATIONMODEL_H
