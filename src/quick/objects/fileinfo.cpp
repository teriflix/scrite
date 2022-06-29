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

#include "fileinfo.h"
#include "application.h"
#include "attachments.h"
#include "scritedocument.h"

#include <QIcon>
#include <QPainter>
#include <QMimeDatabase>

FileInfo::FileInfo(QObject *parent) : QObject(parent) { }

FileInfo::~FileInfo() { }

void FileInfo::setAbsoluteFilePath(const QString &val)
{
    if (val.isEmpty())
        this->setFileInfo(QFileInfo());
    else
        this->setFileInfo(QFileInfo(val));
}

void FileInfo::setAbsolutePath(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    const QString afp = val + QStringLiteral("/") + m_fileInfo.fileName();
    this->setFileInfo(QFileInfo(afp));
}

void FileInfo::setFileName(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + val;
    this->setFileInfo(QFileInfo(afp));
}

void FileInfo::setSuffix(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    QString ext = val.trimmed();
    if (ext.isEmpty())
        return;

    if (ext.startsWith(QStringLiteral(".")))
        ext = ext.mid(1);

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + m_fileInfo.baseName()
            + QStringLiteral(".") + ext;
    this->setFileInfo(QFileInfo(afp));
}

void FileInfo::setBaseName(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty())
        return;

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + val + QStringLiteral(".")
            + m_fileInfo.suffix();
    this->setFileInfo(QFileInfo(afp));
}

void FileInfo::setFileInfo(const QFileInfo &val)
{
    m_fileInfo = val;
    emit fileInfoChanged();
}

///////////////////////////////////////////////////////////////////////////////

FileIconProvider::FileIconProvider() : QQuickImageProvider(QQuickImageProvider::Image) { }

FileIconProvider::~FileIconProvider() { }

QImage FileIconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
#if 0
    // This just doesnt work. QFileIconProvider does not provide icons
    // appropriate to the file type.

    const QFileInfo fi(id);
    const QMimeDatabase mimeDb;
    const QMimeType mimeType = mimeDb.mimeTypeForFile(fi);

    const QIcon icon = QIcon::fromTheme(mimeType.name(), QIcon());
    if(icon.isNull())
    {
        const QImage fallback = this->requestImage(fi);
        return fallback;
    }

    QSize iconSize(96, 96);
    if(requestedSize.isValid())
        iconSize = requestedSize;

    const QPixmap pixmap = icon.pixmap(iconSize);

    if(size)
        *size = pixmap.size();

    return pixmap.toImage();
#else
    QImage icon = this->requestImage(QFileInfo(id));

    QSize iconSize(96, 96);
    if (requestedSize.isValid())
        iconSize = requestedSize;

    icon = icon.scaled(iconSize, Qt::KeepAspectRatio);

    if (size)
        *size = icon.size();

    return icon;
#endif
}

QImage FileIconProvider::requestImage(const QFileInfo &fi)
{
    const QString suffix = fi.suffix().toUpper();

    if (m_suffixImageMap.contains(suffix))
        return m_suffixImageMap.value(suffix);

    Attachment::Type type = Attachment::determineType(fi);
    if (type == Attachment::Photo) {
        const QString absFilePath =
                ScriteDocument::instance()->fileSystem()->absolutePath(fi.filePath());
        QImage image(absFilePath);
        image = image.scaled(96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        return image;
    }

    static const QMap<Attachment::Type, QString> typeIconBase = {
        { Attachment::Photo, QStringLiteral("photo") },
        { Attachment::Video, QStringLiteral("video") },
        { Attachment::Audio, QStringLiteral("audio") },
        { Attachment::Document, QStringLiteral("document") },
    };
    const QString baseIconFile =
            QStringLiteral(":/icons/filetype/") + typeIconBase.value(type) + QStringLiteral(".png");

    QImage icon(baseIconFile);
    if (suffix.isEmpty()) {
        m_suffixImageMap[suffix] = icon;
        return icon;
    }

    QPainter paint;
    paint.begin(&icon);

    QFont font = paint.font();
    font.setPointSize(Application::instance()->idealFontPointSize());
    paint.setFont(font);

    const qreal maxTextWidth = icon.width() * 0.8;
    const QFontMetricsF fm = paint.fontMetrics();
    const QRectF sourceRect = fm.boundingRect(suffix);
    QRectF targetRect = sourceRect;
    qreal targetScale = 1.0;
    if (targetRect.width() > maxTextWidth) {
        targetScale = maxTextWidth / targetRect.width();
        targetRect.setWidth(targetRect.width() * targetScale);
        targetRect.setHeight(targetRect.height() * targetScale);
    }

    targetRect.moveCenter(icon.rect().center());
    if (type == Attachment::Photo)
        targetRect.moveTop(icon.rect().top() + icon.height() * 0.25);
    else if (type == Attachment::Video) {
        targetRect.moveTop(targetRect.top() + icon.height() * 0.1);
        targetRect.moveLeft(targetRect.left() + 1);
    } else if (type == Attachment::Document)
        targetRect.moveTop(targetRect.top() + icon.height() * 0.1);

    paint.setBrush(Qt::white);
    paint.setPen(Qt::black);
    paint.drawRect(targetRect.adjusted(-2, -2, 2, 2));

    paint.translate(targetRect.left(), targetRect.top());
    paint.scale(targetScale, targetScale);
    paint.setOpacity(1);
    paint.drawText(QRectF(0, 0, targetRect.width(), targetRect.height()), Qt::AlignCenter, suffix);

    paint.end();

    m_suffixImageMap[suffix] = icon;
    return icon;
}
