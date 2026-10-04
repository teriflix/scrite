/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DOCUMENTFILESYSTEMENTRY_H
#define DOCUMENTFILESYSTEMENTRY_H

#include <QObject>
#include <QQmlEngine>

/**
 * Claims one file in the current document's file system, so that it survives the
 * cleanup that happens before every save, and is packed into the .scrite file.
 *
 * The file always lives under the "extensions/" folder of the document file system.
 * When no live entry claims a file there, it is removed before the next save.
 */
class DocumentFileSystemEntry : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DocumentFileSystemEntry(QObject *parent = nullptr);
    ~DocumentFileSystemEntry() override;

    // Relative to "extensions/". Absolute paths and paths that climb out with ".." are rejected.
    // clang-format off
    Q_PROPERTY(QString path
               READ path
               WRITE setPath
               NOTIFY pathChanged)
    // clang-format on
    void setPath(const QString &val);
    QString path() const { return m_path; }
    Q_SIGNAL void pathChanged();

    // Changes when path changes, and whenever the document is reset or loaded,
    // because each document gets its own temporary folder.
    // clang-format off
    Q_PROPERTY(QString absoluteFilePath
               READ absoluteFilePath
               NOTIFY absoluteFilePathChanged)
    // clang-format on
    QString absoluteFilePath() const;
    Q_SIGNAL void absoluteFilePathChanged();

    Q_INVOKABLE bool exists() const;
    Q_INVOKABLE bool remove();

signals:
    // Relayed from ScriteDocument. Close any open handle on the file in response to
    // aboutToSave() and aboutToReset(); it is safe to reopen it after justSaved().
    void aboutToSave();
    void justSaved();
    void aboutToReset();

private:
    QString dfsPath() const;
    void onDfsAuction(const QString &filePath, int *claims);

private:
    QString m_path;
};

#endif // DOCUMENTFILESYSTEMENTRY_H
