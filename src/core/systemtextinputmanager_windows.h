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

#ifndef SYSTEMTEXTINPUTMANAGER_WINDOWS_H
#define SYSTEMTEXTINPUTMANAGER_WINDOWS_H

#include "systemtextinputmanager.h"
#include "Windows.h"

#include <QAbstractNativeEventFilter>

class SystemTextInputManagerBackend_Windows : public AbstractSystemTextInputManagerBackend,
                                              public QAbstractNativeEventFilter
{
public:
    explicit SystemTextInputManagerBackend_Windows(SystemTextInputManager *parent = nullptr);
    ~SystemTextInputManagerBackend_Windows();

    // AbstractSystemTextInputManagerBackend interface
    QList<AbstractSystemTextInputSource *> reloadSources();

    // QAbstractNativeEventFilter interface
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
    bool eventFilter(QObject *object, QEvent *event);
};

class SystemTextInputSource_Windows : public AbstractSystemTextInputSource
{
public:
    explicit SystemTextInputSource_Windows(HKL keyboardLayoutHandle,
                                           SystemTextInputManager *parent = nullptr);
    ~SystemTextInputSource_Windows();

    // AbstractSystemTextInputSource interface
    QString id() const { return m_id; }
    QString displayName() const { return m_displayName; }
    int language() const { return m_language; }
    void select();
    void checkSelection();

private:
    HKL m_hkl = nullptr;
    QString m_id;
    int m_language = -1;
    QString m_displayName;
};

#endif // SYSTEMTEXTINPUTMANAGER_WINDOWS_H
