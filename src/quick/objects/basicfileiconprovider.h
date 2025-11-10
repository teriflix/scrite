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

#ifndef BASICFILEICONPROVIDER_H
#define BASICFILEICONPROVIDER_H

#include <QFileInfo>
#include <QQuickImageProvider>

/**
 * The purpose of this image provider is to provide a basic icon for file based
 * on its type (inferenced by its file name extension).
 *
 * As such it returns a distinct icon for four types of files
 * - Audio
 * - Video
 * - Picture
 * - Document
 *
 * The original idea was to use QFileIconProvider. But that did not work for
 * some reason. So, I have settled for this until I figure out a way to get
 * QFileIconProvider to actually work.
 */

class BasicFileIconProvider : public QQuickImageProvider
{
public:
    static QString name();

    explicit BasicFileIconProvider();
    ~BasicFileIconProvider();

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

    static QImage requestImage(const QFileInfo &fi);
};

#endif // BASICFILEICONPROVIDER_H
