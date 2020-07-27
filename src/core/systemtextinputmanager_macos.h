/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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
    SystemTextInputManagerBackend_macOS(SystemTextInputManager *parent=nullptr);
    ~SystemTextInputManagerBackend_macOS();

    // AbstractSystemTextInputManagerBackend interface
    void reloadSources();
    void determineSelectedInputSource();

    // Hook to receive notifications from macOS
    void registerNotificationHooks();
    void unregisterNotificationHooks();

    // For handling kTISNotifySelectedKeyboardInputSourceChanged
    void handleInputSourceChangedNotification();
};

class SystemTextInputSource_macOS : public AbstractSystemTextInputSource
{
    Q_OBJECT

public:
    SystemTextInputSource_macOS(TISInputSourceRef inputSource, SystemTextInputManager *parent=nullptr);
    ~SystemTextInputSource_macOS();

    bool equals(TISInputSourceRef source) const { return m_inputSource == source; }

    // AbstractSystemTextInputSource interface
    QString id() const { return m_id; }
    QString displayName() const { return m_displayName; }
    void select();

private:
    friend class SystemTextInputManagerBackend_macOS;
    void updateSelectionStatus();

private:
    QString m_id;
    QString m_displayName;
    TISInputSourceRef m_inputSource;
};

#endif // SYSTEMTEXTINPUTMANAGER_MACOS_H
