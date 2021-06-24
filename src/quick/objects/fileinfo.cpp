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

#include "fileinfo.h"

#include <QtDebug>
#include <QFileIconProvider>

FileInfo::FileInfo(QObject *parent)
    : QObject(parent)
{

}

FileInfo::~FileInfo()
{

}

void FileInfo::setAbsoluteFilePath(const QString &val)
{
    if(val.isEmpty())
        this->setFileInfo( QFileInfo() );
    else
        this->setFileInfo( QFileInfo(val) );
}

void FileInfo::setAbsolutePath(const QString &val)
{
    if(m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    const QString afp = val + QStringLiteral("/") + m_fileInfo.fileName();
    this->setFileInfo( QFileInfo(afp) );
}

void FileInfo::setFileName(const QString &val)
{
    if(m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + val;
    this->setFileInfo( QFileInfo(afp) );
}

void FileInfo::setSuffix(const QString &val)
{
    if(m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    QString ext = val.trimmed();
    if(ext.isEmpty())
        return;

    if(ext.startsWith(QStringLiteral(".")))
        ext = ext.mid(1);

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + m_fileInfo.baseName() + QStringLiteral(".") + ext;
    this->setFileInfo( QFileInfo(afp) );
}

void FileInfo::setBaseName(const QString &val)
{
    if(m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + val + QStringLiteral(".") + m_fileInfo.suffix();
    this->setFileInfo( QFileInfo(afp) );
}

void FileInfo::setFileInfo(const QFileInfo &val)
{
    m_fileInfo = val;
    emit fileInfoChanged();
}

///////////////////////////////////////////////////////////////////////////////

FileIconProvider::FileIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}

FileIconProvider::~FileIconProvider()
{

}

QPixmap FileIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    const QFileInfo fi(id);
    const QFileIconProvider iconProvider;
    const QIcon icon = iconProvider.icon(fi);

    QSize iconSize(96, 96);
    if(requestedSize.isValid())
        iconSize = requestedSize;

    const QPixmap pixmap = icon.pixmap(iconSize);

    if(size)
        *size = pixmap.size();

    return pixmap;
}
