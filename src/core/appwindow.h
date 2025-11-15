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

#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QQuickView>
#include <QQmlEngine>

class AppWindow : public QQuickView
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static AppWindow *instance();

    explicit AppWindow();
    ~AppWindow();

    // By default, this is true and close button on the title bar is visible.
    // When set to false, it becomes invisible or disabled. Please ensure
    // that the flag is turned back to true as soon as the utility of this
    // flag is over.
    // clang-format off
    Q_PROPERTY(bool closeButtonVisible
               READ isCloseButtonVisible
               WRITE setCloseButtonVisible
               NOTIFY closeButtonVisibleChanged)
    // clang-format on
    void setCloseButtonVisible(bool val);
    bool isCloseButtonVisible() const { return m_closeButtonVisible; }
    Q_SIGNAL void closeButtonVisibleChanged();

private:
    void initializeFileNameToOpen();

private:
    bool m_closeButtonVisible = true;
    Qt::WindowFlags m_defaultWindowFlags;
};

#endif // APPWINDOW_H
