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

#include "colorimageprovider.h"

#include <QtMath>
#include <QPainter>

ColorImageProvider::ColorImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) { }

ColorImageProvider::~ColorImageProvider() { }

QImage ColorImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const QStringList fields = id.split(QStringLiteral("/"), Qt::SkipEmptyParts);

    const QSize defaultSize(64, 64);
    QImage image(requestedSize.isEmpty() ? defaultSize : requestedSize, QImage::Format_ARGB32);
    if (size)
        *size = defaultSize;

    image.fill(QColor(fields.first()).rgba());

    if (fields.size() == 1)
        return image;

    const qreal pw = fields.last().toDouble();
    if (qFuzzyIsNull(pw) || pw < 0)
        return image;

    const qreal hpw = qCeil(pw / 2);
    const QRectF rect = image.rect().adjusted(hpw, hpw, -hpw, -hpw);

    QPainter paint(&image);
    paint.setBrush(Qt::NoBrush);
    paint.setPen(QPen(Qt::black, pw));
    paint.drawRect(rect);
    paint.end();

    return image;
}
