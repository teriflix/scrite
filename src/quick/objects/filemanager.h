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

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QQmlEngine>

class FileManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit FileManager(QObject *parent = nullptr);
    ~FileManager();

    Q_INVOKABLE static QString generateUniqueTemporaryFileName(const QString &ext);

    Q_PROPERTY(QStringList autoDeleteList READ autoDeleteList WRITE setAutoDeleteList NOTIFY autoDeleteListChanged)
    void setAutoDeleteList(const QStringList &val);
    QStringList autoDeleteList() const { return m_autoDeleteList; }
    Q_SIGNAL void autoDeleteListChanged();

    Q_INVOKABLE void removeFilesInAutoDeleteList();
    Q_INVOKABLE void addToAutoDeleteList(const QString &filePath);
    Q_INVOKABLE void removeFromAutoDeleteList(const QString &filePath);
    Q_INVOKABLE void clearAutoDeleteList() { this->setAutoDeleteList(QStringList()); }

private:
    QStringList m_autoDeleteList;
};

#endif // FILEMANAGER_H
