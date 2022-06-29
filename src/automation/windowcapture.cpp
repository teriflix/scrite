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

#include "application.h"
#include "windowcapture.h"

#include <QDir>
#include <QScreen>
#include <QClipboard>
#include <QQuickWindow>
#include <QStandardPaths>

WindowCapture::WindowCapture(QObject *parent)
    : AbstractAutomationStep(parent), m_window(this, "window")
{
    m_path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

WindowCapture::~WindowCapture() { }

void WindowCapture::setWindow(QWindow *val)
{
    if (m_window == val)
        return;

    m_window = val;
    emit windowChanged();
}

void WindowCapture::setArea(const QRectF &val)
{
    if (m_area == val)
        return;

    m_area = val;
    emit areaChanged();
}

void WindowCapture::setMaxImageSize(const QSizeF &val)
{
    if (m_maxImageSize == val)
        return;

    m_maxImageSize = val;
    emit maxImageSizeChanged();
}

void WindowCapture::setPath(const QString &val)
{
    if (m_path == val)
        return;

    m_path = val;
    emit pathChanged();
}

void WindowCapture::setFileName(const QString &val)
{
    if (m_fileName == val)
        return;

    m_fileName = val;
    emit fileNameChanged();
}

void WindowCapture::setFormat(WindowCapture::Format val)
{
    if (m_format == val)
        return;

    m_format = val;
    emit formatChanged();
}

void WindowCapture::setQuality(int val)
{
    if (m_quality == val)
        return;

    m_quality = val;
    emit qualityChanged();
}

void WindowCapture::setForceCounterInFileName(bool val)
{
    if (m_forceCounterInFileName == val)
        return;

    m_forceCounterInFileName = val;
    emit forceCounterInFileNameChanged();
}

void WindowCapture::setReplaceExistingFile(bool val)
{
    if (m_replaceExistingFile == val)
        return;

    m_replaceExistingFile = val;
    emit replaceExistingFileChanged();
}

void WindowCapture::setCaptureMode(WindowCapture::CaptureMode val)
{
    if (m_captureMode == val)
        return;

    m_captureMode = val;
    emit captureModeChanged();
}

QString WindowCapture::capture()
{
    this->clearErrorMessage();

    QString absoluteFilePath;

    QDir dir(m_path);
    if (!dir.exists()) {
        QDir().mkpath(m_path);
        dir = QDir(m_path);
    }

    if (!dir.exists()) {
        this->setErrorMessage(QStringLiteral("Cannot switch to directory: ") + m_path);
        return QString();
    }

    if (m_fileName.isEmpty())
        this->setFileName(QStringLiteral("capture"));

    const QFileInfo fi(dir.absoluteFilePath(m_fileName));
    if (m_forceCounterInFileName || (fi.exists() && !m_replaceExistingFile)) {
        int counter = 1;
        while (1) {
            absoluteFilePath = dir.absoluteFilePath(fi.baseName() + QStringLiteral("-")
                                                    + QString::number(counter++)
                                                    + QStringLiteral(".") + fi.suffix());
            if (!QFile::exists(absoluteFilePath))
                break;
        }
    } else
        absoluteFilePath = fi.absoluteFilePath();

    if (absoluteFilePath.isEmpty()) {
        this->setErrorMessage(QStringLiteral("Couldnt figure out file name for saving."));
        return QString();
    }

    QScreen *screen = nullptr;

    if (m_window.isNull())
        screen = qApp->primaryScreen();
    else
        screen = m_window->screen();

    QTransform tx;
    tx.scale(screen->devicePixelRatio(), screen->devicePixelRatio());
    const QRect rect = m_area.isValid() ? tx.map(m_area).boundingRect().toRect() : QRect();

    QPixmap pixmap;
    if (m_window.isNull()) {
        if (rect.isValid())
            pixmap = screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
        else
            pixmap = screen->grabWindow(0);
    } else {
#if 1
        QQuickWindow *qmlWindow = qobject_cast<QQuickWindow *>(m_window);
        if (qmlWindow) {
            QImage windowImage = qmlWindow->grabWindow();
            if (rect.isValid())
                windowImage = windowImage.copy(rect);

            pixmap = QPixmap::fromImage(windowImage);
        } else
#endif
        {
            if (rect.isValid())
                pixmap = screen->grabWindow(m_window->winId(), rect.x(), rect.y(), rect.width(),
                                            rect.height());
            else
                pixmap = screen->grabWindow(m_window->winId());
        }
    }

    if (pixmap.isNull()) {
        this->setErrorMessage(QStringLiteral("No screen grab was available."));
        return QString();
    }

    pixmap.setDevicePixelRatio(screen->devicePixelRatio());
    if (m_maxImageSize.isValid()
        && (pixmap.width() > m_maxImageSize.width() || pixmap.height() > m_maxImageSize.height()))
        pixmap = pixmap.scaled(m_maxImageSize.toSize(), Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);

    const char *format = m_format == JPGFormat ? "JPG" : "PNG";
    const bool result = pixmap.save(absoluteFilePath, format, m_quality);
    if (!result) {
        this->setErrorMessage(QStringLiteral("Error while saving pixmap"));
        return QString();
    }

    if (m_captureMode == FileAndClipboard)
        qApp->clipboard()->setPixmap(pixmap);

    return absoluteFilePath;
}

void WindowCapture::run()
{
    this->capture();
    this->finish();
}

void WindowCapture::resetWindow()
{
    m_window = nullptr;
    emit windowChanged();
}
