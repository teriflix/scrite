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

#ifndef WINDOWCAPTURE_H
#define WINDOWCAPTURE_H

#include <QWindow>
#include <QQmlEngine>

#include "automation.h"
#include "qobjectproperty.h"

class WindowCapture : public AbstractAutomationStep
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit WindowCapture(QObject *parent = nullptr);
    ~WindowCapture();

    Q_PROPERTY(QWindow* window READ window WRITE setWindow RESET resetWindow NOTIFY windowChanged)
    void setWindow(QWindow *val);
    QWindow *window() const { return m_window; }
    Q_SIGNAL void windowChanged();

    Q_PROPERTY(QRectF area READ area WRITE setArea NOTIFY areaChanged)
    void setArea(const QRectF &val);
    QRectF area() const { return m_area; }
    Q_SIGNAL void areaChanged();

    Q_PROPERTY(QSizeF maxImageSize READ maxImageSize WRITE setMaxImageSize NOTIFY maxImageSizeChanged)
    void setMaxImageSize(const QSizeF &val);
    QSizeF maxImageSize() const { return m_maxImageSize; }
    Q_SIGNAL void maxImageSizeChanged();

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    void setPath(const QString &val);
    QString path() const { return m_path; }
    Q_SIGNAL void pathChanged();

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    void setFileName(const QString &val);
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    enum Format { PNGFormat, JPGFormat };
    Q_ENUM(Format)
    Q_PROPERTY(Format format READ format WRITE setFormat NOTIFY formatChanged)
    void setFormat(Format val);
    Format format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(int quality READ quality WRITE setQuality NOTIFY qualityChanged)
    void setQuality(int val);
    int quality() const { return m_quality; }
    Q_SIGNAL void qualityChanged();

    Q_PROPERTY(bool forceCounterInFileName READ forceCounterInFileName WRITE setForceCounterInFileName NOTIFY forceCounterInFileNameChanged)
    void setForceCounterInFileName(bool val);
    bool forceCounterInFileName() const { return m_forceCounterInFileName; }
    Q_SIGNAL void forceCounterInFileNameChanged();

    Q_PROPERTY(bool replaceExistingFile READ isReplaceExistingFile WRITE setReplaceExistingFile NOTIFY replaceExistingFileChanged)
    void setReplaceExistingFile(bool val);
    bool isReplaceExistingFile() const { return m_replaceExistingFile; }
    Q_SIGNAL void replaceExistingFileChanged();

    enum CaptureMode { FileOnly, FileAndClipboard };
    Q_ENUM(CaptureMode)
    Q_PROPERTY(CaptureMode captureMode READ captureMode WRITE setCaptureMode NOTIFY captureModeChanged)
    void setCaptureMode(CaptureMode val);
    CaptureMode captureMode() const { return m_captureMode; }
    Q_SIGNAL void captureModeChanged();

    Q_INVOKABLE QString capture();

protected:
    void run();

private:
    void resetWindow();

private:
    QRectF m_area;
    int m_quality = -1;
    QString m_path;
    QString m_fileName;
    QSizeF m_maxImageSize;
    Format m_format = JPGFormat;
    bool m_replaceExistingFile = false;
    CaptureMode m_captureMode = FileOnly;
    QObjectProperty<QWindow> m_window;
    bool m_forceCounterInFileName = false;
};

#endif // WINDOWCAPTURE_H
