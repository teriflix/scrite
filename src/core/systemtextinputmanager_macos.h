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

#ifndef SYSTEMTEXTINPUTMANAGER_MACOS_H
#define SYSTEMTEXTINPUTMANAGER_MACOS_H

#include "systemtextinputmanager.h"

#import <Carbon/Carbon.h>

class SystemTextInputManagerBackend_macOS : public AbstractSystemTextInputManagerBackend
{
    Q_OBJECT

public:
    explicit SystemTextInputManagerBackend_macOS(SystemTextInputManager *parent = nullptr);
    ~SystemTextInputManagerBackend_macOS();

    // AbstractSystemTextInputManagerBackend interface
    QList<AbstractSystemTextInputSource *> reloadSources() override;

    // Hook to receive notifications from macOS
    void registerNotificationHooks();
    void unregisterNotificationHooks();

    // For handling kTISNotifySelectedKeyboardInputSourceChanged
    void handleInputSourceChangedNotification();

    // For handling kTISNotifyEnabledKeyboardInputSourcesChanged
    void handleInputSourcesChangedNotification();
};

class SystemTextInputSource_macOS : public AbstractSystemTextInputSource
{
    Q_OBJECT

public:
    explicit SystemTextInputSource_macOS(TISInputSourceRef inputSource,
                                         SystemTextInputManager *parent = nullptr);
    ~SystemTextInputSource_macOS();

    bool equals(TISInputSourceRef source) const { return m_inputSource == source; }

    // AbstractSystemTextInputSource interface
    QString id() const { return m_id; }
    QString displayName() const { return m_displayName; }
    int language() const { return m_language; }
    void select();
    void checkSelection();

private:
    QString m_id;
    int m_language = -1;
    QString m_displayName;
    TISInputSourceRef m_inputSource;
};

#endif // SYSTEMTEXTINPUTMANAGER_MACOS_H
