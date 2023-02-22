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

#ifndef DOCUMENTFILESYSTEM_H
#define DOCUMENTFILESYSTEM_H

#include <QObject>

#include <QFile>
#include <QSize>
#include <QImage>
#include <QFileInfo>

class DocumentFile;

struct DocumentFileSystemData;
class DocumentFileSystem : public QObject
{
    Q_OBJECT

public:
    static void setMarker(const QByteArray &marker);

    explicit DocumentFileSystem(QObject *parent = nullptr);
    ~DocumentFileSystem();

    void hardReset();

    enum Format { UnknownFormat, ScriteFormat, ZipFormat };
    bool load(const QString &fileName, Format *format = nullptr);

    enum SaveMode { BlockingSaveMode, NonBlockingSaveMode };
    bool save(const QString &fileName, bool encrypt = false, SaveMode mode = BlockingSaveMode);

    void setHeader(const QByteArray &header);
    QByteArray header() const;

    QFile *open(const QString &path, QFile::OpenMode mode = QFile::ReadOnly);

    QByteArray read(const QString &path);
    bool write(const QString &path, const QByteArray &bytes);

    QString add(const QString &fileName, const QString &ns = QString());
    QString duplicate(const QString &fileName, const QString &ns = QString());
    bool remove(const QString &path);

    QString absolutePath(const QString &path, bool mkpath = false) const;
    QString relativePath(const QString &path) const;
    bool contains(const QString &path) const;

    bool exists(const QString &path) const;
    QFileInfo fileInfo(const QString &path) const;

    // API to add/replace/remove an external file into the DFS under a specific path/name
    QString addFile(const QString &srcFile, const QString &dstPath, bool replaceIfExists = true);
    QString addImage(const QString &srcFile, const QString &dstPath, const QSize &scaleTo = QSize(),
                     bool replaceIfExists = true);
    QString addImage(const QImage &srcImage, const QString &dstPath, const QSize &scaleTo = QSize(),
                     bool replaceIfExists = true);

    // API to cleanup unreferenced files that may be lying around.
    Q_SIGNAL void auction(const QString &path, int *claims);

signals:
    void saveStarted();
    void saveFinished(bool success);

private:
    void reset();
    void cleanup();
    bool pack(QDataStream &ds);
    bool unpack(QDataStream &ds);
    void saveTaskFinished();

private:
    friend class DocumentFile;
    DocumentFileSystemData *d;
};

class DocumentFile : public QFile
{
public:
    ~DocumentFile();

private:
    explicit DocumentFile(const QString &filePath, DocumentFileSystem *parent = nullptr);
    void onAboutToClose();

private:
    friend class DocumentFileSystem;
    DocumentFileSystem *m_fileSystem = nullptr;
};

#endif // DOCUMENTFILESYSTEM_H
