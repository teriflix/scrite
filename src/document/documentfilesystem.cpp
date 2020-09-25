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

#include "documentfilesystem.h"

#include <QDir>
#include <QtDebug>
#include <QDateTime>
#include <QDataStream>
#include <QTemporaryDir>

struct DocumentFileSystemData
{
    QScopedPointer<QTemporaryDir> folder;
    QByteArray header;
    QList<DocumentFile*> files;

    void pack(QDataStream &ds, const QString &path);
};

void DocumentFileSystemData::pack(QDataStream &ds, const QString &path)
{
    const QFileInfo fi(path);
    if( fi.isDir() )
    {
        const QFileInfoList fiList = QDir(path).entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsLast);
        Q_FOREACH(QFileInfo fi2, fiList)
            this->pack(ds, fi2.absoluteFilePath());
    }
    else
    {
        const QString filePath = fi.absoluteFilePath();

        QDir fsDir(this->folder->path());
        ds << fsDir.relativeFilePath(filePath);

        QFile file(filePath);
        if(file.open(QFile::ReadOnly))
        {
            const qint64 fileSize = file.size();
            const qint64 fileSizeFieldPosition = ds.device()->pos();
            ds << fileSize;

            qint64 bytesWritten = 0;
            const int bufferSize = 65535;
            char buffer[bufferSize];
            while(!file.atEnd() && bytesWritten < fileSize)
            {
                const int bytesRead = int(file.read(buffer, qint64(bufferSize)));
                bytesWritten += qint64(ds.writeRawData(buffer, bytesRead));
            }

            if(bytesWritten != fileSize)
            {
                const qint64 devicePosition = ds.device()->pos();
                ds.device()->seek(fileSizeFieldPosition);
                ds << bytesWritten;
                ds.device()->seek(devicePosition);
            }
        }
        else
            ds << qint64(0);
    }
}

Q_GLOBAL_STATIC(QByteArray, DocumentFileSystemMaker)

void DocumentFileSystem::setMarker(const QByteArray &marker)
{
    if(::DocumentFileSystemMaker->isEmpty())
        *::DocumentFileSystemMaker = marker;
}

DocumentFileSystem::DocumentFileSystem(QObject *parent)
    : QObject(parent),
      d(new DocumentFileSystemData)
{
    this->reset();
}

DocumentFileSystem::~DocumentFileSystem()
{
    delete d;
}

void DocumentFileSystem::reset()
{
    d->header.clear();

    while(!d->files.isEmpty())
    {
        DocumentFile *file = d->files.first();
        file->close();
    }

    d->folder.reset(new QTemporaryDir);

    qDebug() << "PA: " << d->folder->path();
}

bool DocumentFileSystem::load(const QString &fileName)
{
    this->reset();

    if(fileName.isEmpty())
        return false;

    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
        return false;

    const int markerLength = ::DocumentFileSystemMaker->length();
    const QByteArray marker = file.read(markerLength);
    if(marker != *::DocumentFileSystemMaker)
        return false;

    QDataStream ds(&file);
    return this->unpack(ds);
}

bool DocumentFileSystem::save(const QString &fileName)
{
    if(fileName.isEmpty())
        return false;

    QFile file(fileName);
    if( !file.open(QFile::WriteOnly) )
        return false;

    file.write(*::DocumentFileSystemMaker);

    QDataStream ds(&file);
    if( !this->pack(ds) )
    {
        file.close();
        QFile::remove(fileName);
        return false;
    }

    file.close();
    return true;
}

void DocumentFileSystem::setHeader(const QByteArray &header)
{
    d->header = header;
}

QByteArray DocumentFileSystem::header() const
{
    return d->header;
}

QFile *DocumentFileSystem::open(const QString &path, QFile::OpenMode mode)
{
    if(path.isEmpty())
        return nullptr;

    const QString completePath = this->absolutePath(path, true);
    if( !QFile::exists(completePath) && mode == QIODevice::ReadOnly )
        return nullptr;

    DocumentFile *file = new DocumentFile(completePath, this);
    if( !file->open(mode) )
    {
        delete file;
        return nullptr;
    }

    return  file;
}

QByteArray DocumentFileSystem::read(const QString &path)
{
    QByteArray ret;

    if(path.isEmpty())
        return ret;

    const QString completePath = this->absolutePath(path);
    if( !QFile::exists(completePath) )
        return ret;

    DocumentFile file(completePath, this);
    if( !file.open(QFile::ReadOnly) )
        return ret;

    ret = file.readAll();
    file.close();

    return ret;
}

bool DocumentFileSystem::write(const QString &path, const QByteArray &bytes)
{
    if(path.isEmpty() || bytes.isEmpty())
        return false;

    const QString completePath = this->absolutePath(path, true);
    DocumentFile file(completePath, this);
    if( !file.open(QFile::WriteOnly) )
        return false;

    file.write(bytes);
    return true;
}

QString DocumentFileSystem::add(const QString &fileName, const QString &ns)
{
    if(fileName.isEmpty())
        return QString();

    if(this->contains(fileName))
        return this->absolutePath(fileName);

    const QFileInfo fi(fileName);
    if(!fi.exists() || !fi.isFile())
        return QString();

    const QString suffix = fi.suffix().toLower();
    const QString path = ns + "/" + QString::number(QDateTime::currentSecsSinceEpoch()) + "." + suffix;
    const QString absPath = this->absolutePath(path, true);
    if( QFile::copy(fileName, absPath) )
    {
        QFile copiedFile(absPath);
        copiedFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
        return path;
    }

    QFile::remove(absPath);
    return QString();
}

QString DocumentFileSystem::duplicate(const QString &fileName, const QString &ns)
{
    if(fileName.isEmpty())
        return QString();

    if(!this->contains(fileName))
        return QString();

    const QFileInfo fi( this->absolutePath(fileName) );
    if(!fi.exists() || !fi.isFile())
        return QString();

    const QString path = ns + "/" + QString::number(QDateTime::currentSecsSinceEpoch()) + "." + fi.suffix();
    const QString absPath = this->absolutePath(path, true);
    if( QFile::copy(fi.absoluteFilePath(), absPath) )
    {
        QFile copiedFile(absPath);
        copiedFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
        return path;
    }

    QFile::remove(absPath);
    return QString();
}

bool DocumentFileSystem::remove(const QString &path)
{
    if(path.isEmpty())
        return false;

    const QString completePath = this->absolutePath(path);
    return QFile::remove(completePath);
}

QString DocumentFileSystem::absolutePath(const QString &path, bool mkpath) const
{
    if(path.isEmpty())
        return QString();

    if(QDir::isAbsolutePath(path))
    {
        if( path.startsWith(d->folder->path()) )
            return path;

        return QString();
    }

    const QString ret = d->folder->filePath(path);
    const QFileInfo fi(ret);
    if(!fi.exists() && mkpath)
    {
        if( !QDir().mkpath(fi.absolutePath()) )
            return QString();
    }

    return ret;
}

QString DocumentFileSystem::relativePath(const QString &path) const
{
    if(path.isEmpty())
        return QString();

    return QDir(d->folder->path()).relativeFilePath(path);
}

bool DocumentFileSystem::contains(const QString &path) const
{
    if(path.isEmpty())
        return false;

    QFileInfo fi(path);
    if(fi.isRelative())
        return this->exists(path);

    return fi.absoluteFilePath().startsWith( d->folder->path() );
}

bool DocumentFileSystem::exists(const QString &path) const
{
    if(path.isEmpty())
        return false;

    const QString completePath = this->absolutePath(path);
    return QFile::exists(completePath);
}

QFileInfo DocumentFileSystem::fileInfo(const QString &path) const
{
    if(path.isEmpty())
        return QFileInfo();

    const QString completePath = this->absolutePath(path);
    return QFileInfo(completePath);
}

QString DocumentFileSystem::addFile(const QString &srcFile, const QString &dstPath, bool replaceIfExists)
{
    // Verify that srcFile isnt already a part of the document file system.
    if( this->contains(srcFile) )
        return QDir::isAbsolutePath(srcFile) ? this->relativePath(srcFile) : this->absolutePath(srcFile);

    // Verify that the srcFile exists.
    if( !QFile::exists(srcFile) )
        return QString();

    // Verify that dstPath is relative path.
    if( QDir::isAbsolutePath(dstPath) )
        return QString();

    // Compose absolute path for destination, make sure it is a file.
    QString absDstPath = this->absolutePath(dstPath, true);
    if( QFileInfo(absDstPath).isDir() )
        return QString();

    // Delete previous file, if replacement is requested
    if( QFile::exists(absDstPath) )
    {
        if(!replaceIfExists)
            return QString();

        QFile::remove(absDstPath);
    }

    // Copy the file into the DFS.
    if( !QFile::copy(srcFile, dstPath) )
        return QString();

    // That's it
    return this->relativePath(absDstPath);
}

QString DocumentFileSystem::addImage(const QString &srcFile, const QString &dstPath, const QSize &scaleTo, bool replaceIfExists)
{
    // Verify that srcFile isnt already a part of the document file system.
    if( this->contains(srcFile) )
        return QDir::isAbsolutePath(srcFile) ? this->relativePath(srcFile) : this->absolutePath(srcFile);

    const QImage image(srcFile);
    return this->addImage(image, dstPath, scaleTo, replaceIfExists);
}

QString DocumentFileSystem::addImage(const QImage &srcImage, const QString &dstPath, const QSize &scaleTo, bool replaceIfExists)
{
    // Verify that dstPath is relative path.
    if( QDir::isAbsolutePath(dstPath) )
        return QString();

    // Compose absolute path for destination, make sure it is a file.
    QString absDstPath = this->absolutePath(dstPath, true);
    if( QFileInfo(absDstPath).isDir() )
        return QString();

    // If the image passed to this function is empty, we just have
    // to delete a previously existing file.
    if(srcImage.isNull())
    {
        if( QFile::exists(absDstPath) )
            QFile::remove(absDstPath);

        return QString();
    }

    // Delete previous file, if replacement is requested
    if( QFile::exists(absDstPath) )
    {
        if(!replaceIfExists)
            return QString();

        QFile::remove(absDstPath);
    }

    QImage imageToSave = srcImage;
    if(!scaleTo.isEmpty())
    {
        if(imageToSave.width() > scaleTo.width() || imageToSave.height() > scaleTo.height())
            imageToSave = imageToSave.scaled(scaleTo, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    const QString suffix = QFileInfo(absDstPath).suffix().toUpper();
    const bool ret = imageToSave.save(absDstPath, qPrintable(suffix));
    return ret ? this->relativePath(absDstPath) : QString();
}

bool DocumentFileSystem::pack(QDataStream &ds)
{
    const QByteArray compressedHeader = d->header.isEmpty() ? d->header : qCompress(d->header);

    ds << compressedHeader;
    d->pack(ds, d->folder->path());

    return true;
}

bool DocumentFileSystem::unpack(QDataStream &ds)
{
    QByteArray compressedHeader;
    ds >> compressedHeader;

    d->header = compressedHeader.isEmpty() ? compressedHeader : qUncompress(compressedHeader);

    const QDir folderPath(d->folder->path());

    while(!ds.atEnd())
    {
        QString relativeFilePath;
        ds >> relativeFilePath;

        qint64 fileSize = 0;
        ds >> fileSize;

        if(fileSize == 0)
            continue;

        const QString absoluteFilePath = folderPath.absoluteFilePath(relativeFilePath);
        const QFileInfo fi(absoluteFilePath);

        if( !QDir().mkpath(fi.absolutePath()) )
            return false;

        QFile file(absoluteFilePath);
        if( !file.open(QFile::WriteOnly) )
            return false;

        qint64 bytesRead = 0;
        const int bufferSize = 65535;
        char buffer[bufferSize];

        while(bytesRead < fileSize)
        {
            const int rawDataLen = ds.readRawData(buffer, qMin(int(fileSize-bytesRead),bufferSize));
            file.write(buffer, rawDataLen);
            bytesRead += qint64(rawDataLen);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

DocumentFile::DocumentFile(const QString &filePath, DocumentFileSystem *parent)
    : QFile(filePath, parent),
      m_fileSystem(parent)
{
    if(m_fileSystem != nullptr)
    {
        m_fileSystem->d->files.append(this);
        connect(this, &QIODevice::aboutToClose, this, &DocumentFile::onAboutToClose);
    }
}

DocumentFile::~DocumentFile()
{
    if(m_fileSystem != nullptr)
    {
        m_fileSystem->d->files.removeOne(this);
        m_fileSystem = nullptr;
    }
}

void DocumentFile::onAboutToClose()
{
    if(m_fileSystem != nullptr)
    {
        m_fileSystem->d->files.removeOne(this);
        m_fileSystem = nullptr;
    }
}

