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

#ifndef BASICFILEINFO_H
#define BASICFILEINFO_H

#include <QQmlEngine>
#include <QFileInfo>

/**
 * This class is used for two purposes
 *
 * 1. To fetch information about a file based on its file-path
 * 2. To alter various aspects of the file (path, base name, suffix etc) and
 *    compose a full path from it.
 */

class BasicFileInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BasicFileInfo(QObject *parent = nullptr);
    ~BasicFileInfo();

    // clang-format off
    Q_PROPERTY(bool exists
               READ isExists
               NOTIFY fileInfoChanged)
    // clang-format on
    bool isExists() const { return m_fileInfo.exists(); }

    // clang-format off
    Q_PROPERTY(QString absoluteFilePath
               READ absoluteFilePath
               WRITE setAbsoluteFilePath
               NOTIFY fileInfoChanged)
    // clang-format on
    void setAbsoluteFilePath(const QString &val);
    QString absoluteFilePath() const { return m_fileInfo.absoluteFilePath(); }

    // clang-format off
    Q_PROPERTY(QString absolutePath
               READ absolutePath
               WRITE setAbsolutePath
               NOTIFY fileInfoChanged)
    // clang-format on
    void setAbsolutePath(const QString &val);
    QString absolutePath() const { return m_fileInfo.absolutePath(); }

    // clang-format off
    Q_PROPERTY(QString fileName
               READ fileName
               WRITE setFileName
               NOTIFY fileInfoChanged)
    // clang-format on
    void setFileName(const QString &val);
    QString fileName() const { return m_fileInfo.fileName(); }

    // clang-format off
    Q_PROPERTY(QString suffix
               READ suffix
               WRITE setSuffix
               NOTIFY fileInfoChanged)
    // clang-format on
    void setSuffix(const QString &val);
    QString suffix() const { return m_fileInfo.suffix(); }

    // clang-format off
    Q_PROPERTY(QString baseName
               READ baseName
               WRITE setBaseName
               NOTIFY fileInfoChanged)
    // clang-format on
    void setBaseName(const QString &val);
    QString baseName() const { return m_fileInfo.completeBaseName(); }

    QFileInfo fileInfo() const { return m_fileInfo; }
    Q_SIGNAL void fileInfoChanged();

private:
    void setFileInfo(const QFileInfo &val);

private:
    QFileInfo m_fileInfo;
};

#endif // BASICFILEINFO_H
