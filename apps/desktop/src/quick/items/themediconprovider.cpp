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

#include "themediconprovider.h"
#include "utils.h"

ThemedIconProvider::ThemedIconProvider() : QQuickImageProvider(QQuickImageProvider::Image) { }

ThemedIconProvider::~ThemedIconProvider() { }

QString ThemedIconProvider::name()
{
    return QStringLiteral("icon");
}

QImage ThemedIconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // id is "dark/qrc:/icons/action/add_episode.png" or "light/qrc:/icons/..."
    const int slashPos = id.indexOf(QLatin1Char('/'));
    if (slashPos < 0)
        return QImage();

    const QString theme = id.left(slashPos); // "dark" or "light"
    QString imagePath = id.mid(slashPos + 1); // "qrc:/icons/action/add_episode.png"

    // Strip "qrc:" prefix so QImage can locate the resource as ":/icons/..."
    if (imagePath.startsWith("qrc:"))
        imagePath = imagePath.mid(3);

    QImage image(imagePath);
    if (image.isNull())
        return QImage();

    if (requestedSize.isValid() && requestedSize != image.size())
        image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (size)
        *size = image.size();

    if (theme == QLatin1String("dark")) {
        const int dotPos = imagePath.lastIndexOf(QLatin1Char('.'));
        const QString darkImagePath = dotPos >= 0
                ? imagePath.left(dotPos) + QStringLiteral("_darkmode") + imagePath.mid(dotPos)
                : imagePath + QStringLiteral("_darkmode");

        const QImage darkImage(darkImagePath);
        if (!darkImage.isNull()) {
            image = darkImage;
            if (requestedSize.isValid() && requestedSize != image.size())
                image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (size)
                *size = image.size();
        } else {
            image = image.convertToFormat(QImage::Format_ARGB32);
            image.invertPixels(QImage::InvertRgb); // invert RGB, preserve alpha
        }
    }

    return image;
}
