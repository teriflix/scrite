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

#include "basicfileiconprovider.h"

#include "application.h"
#include "attachments.h"
#include "scritedocument.h"

#include <QPainter>

QString BasicFileIconProvider::name()
{
    return QStringLiteral("fileIcon");
}

BasicFileIconProvider::BasicFileIconProvider() : QQuickImageProvider(QQuickImageProvider::Image) { }

BasicFileIconProvider::~BasicFileIconProvider() { }

QImage BasicFileIconProvider::requestImage(const QString &id, QSize *size,
                                           const QSize &requestedSize)
{
    QImage icon = BasicFileIconProvider::requestImage(QFileInfo(id));

    QSize iconSize(96, 96);
    if (requestedSize.isValid())
        iconSize = requestedSize;

    icon = icon.scaled(iconSize, Qt::KeepAspectRatio);

    if (size)
        *size = icon.size();

    return icon;
}

QImage BasicFileIconProvider::requestImage(const QFileInfo &fi)
{
    static QMap<QString, QImage> globalSuffixImageMap;

    const QString suffix = fi.suffix().toUpper();

    if (globalSuffixImageMap.contains(suffix))
        return globalSuffixImageMap.value(suffix);

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
        globalSuffixImageMap[suffix] = icon;
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

    globalSuffixImageMap[suffix] = icon;
    return icon;
}
