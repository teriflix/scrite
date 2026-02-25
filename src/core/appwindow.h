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

#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QQmlEngine>
#include <QQuickWindow>

class AppWindow : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(AppWindow)

public:
    static QQuickWindow *instance();
    ~AppWindow();

    static AppWindow *qmlAttachedProperties(QObject *object);

    QQuickWindow *window() const { return m_window; }

    // clang-format off
    Q_PROPERTY(bool closeButtonVisible
               READ isCloseButtonVisible
               WRITE setCloseButtonVisible
               NOTIFY closeButtonVisibleChanged)
    // clang-format on
    void setCloseButtonVisible(bool val);
    bool isCloseButtonVisible() const { return m_closeButtonVisible; }
    Q_SIGNAL void closeButtonVisibleChanged();

signals:
    void initialize();

private:
    explicit AppWindow(QQuickWindow *window);

    void initializeFileNameToOpen();

private:
    QQuickWindow *m_window = nullptr;
    bool m_closeButtonVisible = true;
    Qt::WindowFlags m_defaultWindowFlags;
};

#endif // APPWINDOW_H
