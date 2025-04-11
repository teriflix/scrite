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

#include "documentfilesystem.h"

#include <QDir>
#include <QtDebug>
#include <QDateTime>
#include <QDataStream>
#include <QTemporaryDir>
#include <QFutureWatcher>
#include <QStandardPaths>
#include <QtConcurrentRun>

#include "scrite.h"
#include "quazip.h"
#include "quazipfile.h"
#include "simplecrypt.h"
#include "restapikey/restapikey.h"

struct DocumentFileSystemData
{
    QByteArray header;
    QList<DocumentFile *> files;
    QMutex folderMutex;
    QScopedPointer<QTemporaryDir> folder;
    qint64 fileNameCounter = 0;

    static const QString normalHeaderFile;
    static const QString encryptedHeaderFile;

    void pack(QDataStream &ds, const QString &path);

    QStringList filePaths() const
    {
        QStringList ret;
        this->filePaths(ret, folder->path());
        return ret;
    }

private:
    void filePaths(QStringList &paths, const QString &dirPath) const;
};

const QString DocumentFileSystemData::normalHeaderFile = QStringLiteral("_header.json");
const QString DocumentFileSystemData::encryptedHeaderFile =
        QStringLiteral("_header.json_encrypted");

void DocumentFileSystemData::pack(QDataStream &ds, const QString &path)
{
    const QFileInfo fi(path);
    if (fi.isDir()) {
        const QFileInfoList fiList = QDir(path).entryInfoList(
                QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::DirsLast);
        for (const QFileInfo &fi2 : fiList)
            this->pack(ds, fi2.absoluteFilePath());
    } else {
        const QString filePath = fi.absoluteFilePath();

        QDir fsDir(this->folder->path());
        ds << fsDir.relativeFilePath(filePath);

        QFile file(filePath);
        if (file.open(QFile::ReadOnly)) {
            const qint64 fileSize = file.size();
            const qint64 fileSizeFieldPosition = ds.device()->pos();
            ds << fileSize;

            qint64 bytesWritten = 0;
            const int bufferSize = 65535;
            char buffer[bufferSize];
            while (!file.atEnd() && bytesWritten < fileSize) {
                const int bytesRead = int(file.read(buffer, qint64(bufferSize)));
                bytesWritten += qint64(ds.writeRawData(buffer, bytesRead));
            }

            if (bytesWritten != fileSize) {
                const qint64 devicePosition = ds.device()->pos();
                ds.device()->seek(fileSizeFieldPosition);
                ds << bytesWritten;
                ds.device()->seek(devicePosition);
            }
        } else
            ds << qint64(0);
    }
}

void DocumentFileSystemData::filePaths(QStringList &paths, const QString &dirPath) const
{
    QDir fsDir(this->folder->path());

    QFileInfo fi(dirPath);
    if (fi.isDir()) {
        const QFileInfoList fiList = QDir(dirPath).entryInfoList(
                QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::DirsLast);
        for (const QFileInfo &fi2 : fiList)
            this->filePaths(paths, fi2.absoluteFilePath());
    } else
        paths.append(fsDir.relativeFilePath(fi.absoluteFilePath()));
}

Q_GLOBAL_STATIC(QByteArray, DocumentFileSystemMaker)

void DocumentFileSystem::setMarker(const QByteArray &marker)
{
    if (::DocumentFileSystemMaker->isEmpty())
        *::DocumentFileSystemMaker = marker;
}

DocumentFileSystem::DocumentFileSystem(QObject *parent)
    : QObject(parent), d(new DocumentFileSystemData)
{
    this->reset();
}

DocumentFileSystem::~DocumentFileSystem()
{
    delete d;
}

void DocumentFileSystem::hardReset()
{
    QMutexLocker mutexLocker(&d->folderMutex);
    this->reset();
}

void DocumentFileSystem::reset()
{
    d->header.clear();
    d->fileNameCounter = QDateTime::currentMSecsSinceEpoch();

    while (!d->files.isEmpty()) {
        DocumentFile *file = d->files.first();
        file->close();
    }

    d->folder.reset(new QTemporaryDir);

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: " << d->folder->path();
#endif
}

bool DocumentFileSystem::load(const QString &fileName, Format *format)
{
    QMutexLocker mutexLocker(&d->folderMutex);

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: DocumentFileSystem.Load " << fileName;
#endif

    this->reset();
    if (format)
        *format = UnknownFormat;

    if (fileName.isEmpty())
        return false;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    const int markerLength = ::DocumentFileSystemMaker->length();
    const QByteArray marker = file.read(markerLength);
    if (marker == *::DocumentFileSystemMaker) {
        QDataStream ds(&file);
        const bool ret = this->unpack(ds);
        if (format)
            *format = ScriteFormat;
        return ret;
    }

    // If we are here, then we can use a QuaZip to unpack the Scrite
    // document as a ZIP file.
    file.close();

    if (Scrite::doUnzip(QFileInfo(fileName), *d->folder)) {
        QString headerPath;

        const QString normalPath = d->folder->filePath(DocumentFileSystemData::normalHeaderFile);
        const QString encryptedPath =
                d->folder->filePath(DocumentFileSystemData::encryptedHeaderFile);
        if (QFile::exists(normalPath))
            headerPath = normalPath;
        else if (QFile::exists(encryptedPath))
            headerPath = encryptedPath;
        else
            return false;

        QFile headerFile(headerPath);

        const QByteArray headerData =
                headerFile.open(QFile::ReadOnly) ? headerFile.readAll() : QByteArray();
        if (headerPath == encryptedPath) {
            SimpleCrypt sc(REST_CRYPT_KEY);
            d->header = sc.decryptToByteArray(headerData);
        } else
            d->header = headerData;

        if (format)
            *format = ZipFormat;
    }

    return !d->header.isEmpty();
}

bool saveTask(const QByteArray &header, bool encrypt, const QDir &folder,
              const QString &targetFileName, QMutex *mutex)
{
    QMutexLocker mutexLocker(mutex);

    QByteArray headerData = header;
    if (encrypt) {
        SimpleCrypt sc(REST_CRYPT_KEY);
        headerData = sc.encryptToByteArray(headerData);
    }

    const QString headerFileName =
            folder.filePath(encrypt ? DocumentFileSystemData::encryptedHeaderFile
                                    : DocumentFileSystemData::normalHeaderFile);
    QSaveFile headerFile(headerFileName);
    if (!headerFile.open(QFile::WriteOnly))
        return false;

    headerFile.write(headerData);
    if (!headerFile.commit())
        return false;

    const QString tmpFileName = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
            + QStringLiteral("/scrite_") + QString::number(QDateTime::currentMSecsSinceEpoch())
            + QStringLiteral("_temp.scrite");

    const QFileInfo fileInfo(tmpFileName);
    bool success = Scrite::doZip(fileInfo, folder);

    if (success && QFile::exists(tmpFileName) && QFileInfo(tmpFileName).size() > 0) {
        if (QFile::exists(targetFileName))
            success &= QFile::remove(targetFileName);
        if (success)
            success &= QFile::copy(tmpFileName, targetFileName);
        QFile::remove(tmpFileName);
    }

    return success;
}

bool DocumentFileSystem::save(const QString &fileName, bool encrypt, SaveMode mode)
{
    if (fileName.isEmpty())
        return false;

    // Ensure that unwanted files are no longer in the DFS folder
    this->cleanup();

#if 0
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
#else
    // Starting with 0.5.5 Scrite documents are basically ZIP files.
    if (mode == NonBlockingSaveMode) {
        /**
         * NonBlockingSaveMode implementation is not perfect. A lot of assumptions are
         * built into this code.
         *
         * 1. We assume that when non-blocking-save is triggered, no other attachment is
         *    added to DFS until the save finishes.
         * 2. We also assume that while a non-blocking-save is underway, another request
         *    won't come. If it does, then the second request will block until the first
         *    one is completed, even if its results get discarded soon after.
         * 3. We assume that save never takes more than 30 seconds on any computer, which
         *    is the least interval for auto-save.
         *
         * So the advise is to use blocking-save for when the user actually clicks on the
         * save button. Use the non-blocking save, for auto-save purposes.
         */
        emit saveStarted();

        const QString saveTaskWatcher = QStringLiteral("saveTaskWatcher");
        QFutureWatcher<bool> *watcher = this->findChild<QFutureWatcher<bool> *>(
                saveTaskWatcher, Qt::FindDirectChildrenOnly);
        if (watcher) {
            disconnect(watcher, &QFutureWatcher<bool>::finished, this,
                       &DocumentFileSystem::saveTaskFinished);
            watcher->deleteLater();
        }

        watcher = new QFutureWatcher<bool>(this);
        watcher->setObjectName(saveTaskWatcher);
        connect(watcher, &QFutureWatcher<bool>::finished, this,
                &DocumentFileSystem::saveTaskFinished);
        watcher->setFuture(QtConcurrent::run(saveTask, d->header, encrypt, QDir(d->folder->path()),
                                             fileName, &d->folderMutex));

        return true;
    }

    const bool ret =
            saveTask(d->header, encrypt, QDir(d->folder->path()), fileName, &d->folderMutex);
    return ret;
#endif
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
    if (path.isEmpty())
        return nullptr;

    const QString completePath = this->absolutePath(path, true);
    if (!QFile::exists(completePath) && mode == QIODevice::ReadOnly)
        return nullptr;

    DocumentFile *file = new DocumentFile(completePath, this);
    if (!file->open(mode)) {
        delete file;
        return nullptr;
    }

    return file;
}

QByteArray DocumentFileSystem::read(const QString &path)
{
    QByteArray ret;

    if (path.isEmpty())
        return ret;

    const QString completePath = this->absolutePath(path);
    if (!QFile::exists(completePath))
        return ret;

    DocumentFile file(completePath, this);
    if (!file.open(QFile::ReadOnly))
        return ret;

    ret = file.readAll();
    file.close();

    return ret;
}

bool DocumentFileSystem::write(const QString &path, const QByteArray &bytes)
{
    if (path.isEmpty() || bytes.isEmpty())
        return false;

    const QString completePath = this->absolutePath(path, true);
    DocumentFile file(completePath, this);
    if (!file.open(QFile::WriteOnly))
        return false;

    file.write(bytes);
    return true;
}

QString DocumentFileSystem::add(const QString &fileName, const QString &ns)
{
    if (fileName.isEmpty())
        return QString();

    if (this->contains(fileName))
        return this->absolutePath(fileName);

    const QFileInfo fi(fileName);
    if (!fi.exists() || !fi.isFile())
        return QString();

    const QString suffix = fi.suffix().toLower();
    const QString path = ns + "/" + QString::number(d->fileNameCounter++) + "." + suffix;
    const QString absPath = this->absolutePath(path, true);
    if (QFile::copy(fileName, absPath)) {
        QFile copiedFile(absPath);
        copiedFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                  | QFileDevice::ReadUser | QFileDevice::WriteUser
                                  | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                                  | QFileDevice::ReadOther | QFileDevice::WriteOther);
        return path;
    }

    QFile::remove(absPath);
    return QString();
}

QString DocumentFileSystem::duplicate(const QString &fileName, const QString &ns)
{
    if (fileName.isEmpty())
        return QString();

    if (!this->contains(fileName))
        return QString();

    const QFileInfo fi(this->absolutePath(fileName));
    if (!fi.exists() || !fi.isFile())
        return QString();

    const QString path = ns + "/" + QString::number(d->fileNameCounter++) + "." + fi.suffix();
    const QString absPath = this->absolutePath(path, true);
    if (QFile::copy(fi.absoluteFilePath(), absPath)) {
        QFile copiedFile(absPath);
        copiedFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                  | QFileDevice::ReadUser | QFileDevice::WriteUser
                                  | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                                  | QFileDevice::ReadOther | QFileDevice::WriteOther);
        return path;
    }

    QFile::remove(absPath);
    return QString();
}

bool DocumentFileSystem::remove(const QString &path)
{
    if (path.isEmpty())
        return false;

    const QString completePath = this->absolutePath(path);
    return QFile::remove(completePath);
}

QString DocumentFileSystem::absolutePath(const QString &path, bool mkpath) const
{
    if (path.isEmpty())
        return QString();

    if (QDir::isAbsolutePath(path)) {
        if (path.startsWith(d->folder->path()))
            return path;

        return QString();
    }

    const QString ret = d->folder->filePath(path);
    const QFileInfo fi(ret);
    if (!fi.exists() && mkpath) {
        if (!QDir().mkpath(fi.absolutePath()))
            return QString();
    }

    return ret;
}

QString DocumentFileSystem::relativePath(const QString &path) const
{
    if (path.isEmpty())
        return QString();

    return QDir(d->folder->path()).relativeFilePath(path);
}

bool DocumentFileSystem::contains(const QString &path) const
{
    if (path.isEmpty())
        return false;

    QFileInfo fi(path);
    if (fi.isRelative())
        return this->exists(path);

    return fi.absoluteFilePath().startsWith(d->folder->path());
}

bool DocumentFileSystem::exists(const QString &path) const
{
    if (path.isEmpty())
        return false;

    const QString completePath = this->absolutePath(path);
    return QFile::exists(completePath);
}

QFileInfo DocumentFileSystem::fileInfo(const QString &path) const
{
    if (path.isEmpty())
        return QFileInfo();

    const QString completePath = this->absolutePath(path);
    return QFileInfo(completePath);
}

QString DocumentFileSystem::addFile(const QString &srcFile, const QString &dstPath,
                                    bool replaceIfExists)
{
    // Verify that srcFile isnt already a part of the document file system.
    if (this->contains(srcFile))
        return QDir::isAbsolutePath(srcFile) ? this->relativePath(srcFile)
                                             : this->absolutePath(srcFile);

    // Verify that the srcFile exists.
    if (!QFile::exists(srcFile))
        return QString();

    // Verify that dstPath is relative path.
    if (QDir::isAbsolutePath(dstPath))
        return QString();

    // Compose absolute path for destination, make sure it is a file.
    QString absDstPath = this->absolutePath(dstPath, true);
    if (QFileInfo(absDstPath).isDir())
        return QString();

    // Delete previous file, if replacement is requested
    if (QFile::exists(absDstPath)) {
        if (!replaceIfExists)
            return QString();

        QFile::remove(absDstPath);
    }

    // Copy the file into the DFS.
    if (!QFile::copy(srcFile, dstPath))
        return QString();

    // That's it
    return this->relativePath(absDstPath);
}

QString DocumentFileSystem::addImage(const QString &srcFile, const QString &dstPath,
                                     const QSize &scaleTo, bool replaceIfExists)
{
    // Verify that srcFile isnt already a part of the document file system.
    if (this->contains(srcFile))
        return QDir::isAbsolutePath(srcFile) ? this->relativePath(srcFile)
                                             : this->absolutePath(srcFile);

    const QImage image(srcFile);
    return this->addImage(image, dstPath, scaleTo, replaceIfExists);
}

QString DocumentFileSystem::addImage(const QImage &srcImage, const QString &dstPath,
                                     const QSize &scaleTo, bool replaceIfExists)
{
    // Verify that dstPath is relative path.
    if (QDir::isAbsolutePath(dstPath))
        return QString();

    // Compose absolute path for destination, make sure it is a file.
    QString absDstPath = this->absolutePath(dstPath, true);
    if (QFileInfo(absDstPath).isDir())
        return QString();

    // If the image passed to this function is empty, we just have
    // to delete a previously existing file.
    if (srcImage.isNull()) {
        if (QFile::exists(absDstPath))
            QFile::remove(absDstPath);

        return QString();
    }

    // Delete previous file, if replacement is requested
    if (QFile::exists(absDstPath)) {
        if (!replaceIfExists)
            return QString();

        QFile::remove(absDstPath);
    }

    QImage imageToSave = srcImage;
    if (!scaleTo.isEmpty()) {
        if (imageToSave.width() > scaleTo.width() || imageToSave.height() > scaleTo.height())
            imageToSave =
                    imageToSave.scaled(scaleTo, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    const QString suffix = QFileInfo(absDstPath).suffix().toUpper();
    const bool ret = imageToSave.save(absDstPath, qPrintable(suffix));
    return ret ? this->relativePath(absDstPath) : QString();
}

void DocumentFileSystem::cleanup()
{
    const QStringList filePaths = d->filePaths();
    for (const QString &filePath : filePaths) {
        int claims = 0;
        emit auction(filePath, &claims);
        if (claims == 0)
            this->remove(filePath);
    }
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

    while (!ds.atEnd()) {
        QString relativeFilePath;
        ds >> relativeFilePath;

        qint64 fileSize = 0;
        ds >> fileSize;

        if (fileSize == 0)
            continue;

        const QString absoluteFilePath = folderPath.absoluteFilePath(relativeFilePath);
        const QFileInfo fi(absoluteFilePath);

        if (!QDir().mkpath(fi.absolutePath()))
            return false;

        QFile file(absoluteFilePath);
        if (!file.open(QFile::WriteOnly))
            return false;

        qint64 bytesRead = 0;
        const int bufferSize = 65535;
        char buffer[bufferSize];

        while (bytesRead < fileSize) {
            const int rawDataLen =
                    ds.readRawData(buffer, qMin(int(fileSize - bytesRead), bufferSize));
            file.write(buffer, rawDataLen);
            bytesRead += qint64(rawDataLen);
        }
    }

    return true;
}

void DocumentFileSystem::saveTaskFinished()
{
    if (this->sender() && this->sender()->objectName() == QStringLiteral("saveTaskWatcher")
        && this->sender()->inherits("QFutureWatcherBase")) {
        QFutureWatcher<bool> *watcher = dynamic_cast<QFutureWatcher<bool> *>(this->sender());
        if (watcher) {
            emit saveFinished(watcher->result());
            watcher->deleteLater();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

DocumentFile::DocumentFile(const QString &filePath, DocumentFileSystem *parent)
    : QFile(filePath, parent), m_fileSystem(parent)
{
    if (m_fileSystem != nullptr) {
        m_fileSystem->d->files.append(this);
        connect(this, &QIODevice::aboutToClose, this, &DocumentFile::onAboutToClose);
    }
}

DocumentFile::~DocumentFile()
{
    if (m_fileSystem != nullptr) {
        m_fileSystem->d->files.removeOne(this);
        m_fileSystem = nullptr;
    }
}

void DocumentFile::onAboutToClose()
{
    if (m_fileSystem != nullptr) {
        m_fileSystem->d->files.removeOne(this);
        m_fileSystem = nullptr;
    }
}
