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

protected:
    void showEvent(QShowEvent *);

private:
    void initializeFileNameToOpen();
};

#endif // APPWINDOW_H
