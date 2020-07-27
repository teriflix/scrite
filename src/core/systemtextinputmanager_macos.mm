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

#include "systemtextinputmanager_macos.h"
#include <QtDebug>

SystemTextInputManagerBackend_macOS::SystemTextInputManagerBackend_macOS(SystemTextInputManager *parent)
    : AbstractSystemTextInputManagerBackend(parent)
{
    this->registerNotificationHooks();
}

SystemTextInputManagerBackend_macOS::~SystemTextInputManagerBackend_macOS()
{
    this->unregisterNotificationHooks();
}

void SystemTextInputManagerBackend_macOS::reloadSources()
{
    CFArrayRef sourceList = TISCreateInputSourceList(NULL, false);
    const int nrSources = CFArrayGetCount(sourceList);

    for(int i=0; i<nrSources; i++)
    {
        TISInputSourceRef inputSource = (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, i);
        const void *inputSourceType = TISGetInputSourceProperty(inputSource, kTISPropertyInputSourceType);
        if( kTISTypeKeyboardInputMode != inputSourceType && kTISTypeKeyboardLayout != inputSourceType )
            continue;

        new SystemTextInputSource_macOS(inputSource, this->inputManager());
    }
}

void SystemTextInputManagerBackend_macOS::determineSelectedInputSource()
{
    const int nrInputSources = this->inputManager()->count();
    if(nrInputSources == 0)
        return;

    for(int i=0; i<nrInputSources; i++)
    {
        SystemTextInputSource_macOS *inputSource = static_cast<SystemTextInputSource_macOS*>(this->inputManager()->sourceAt(i));
        if(inputSource)
            inputSource->updateSelectionStatus();
    }
}

static void macOSNotificationHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo)
{
    Q_UNUSED(object);
    Q_UNUSED(userInfo);

    if(center != CFNotificationCenterGetDistributedCenter())
        return;

    const QString qname = QString::fromCFString(name);

    SystemTextInputManagerBackend_macOS *macOSBackend = static_cast<SystemTextInputManagerBackend_macOS*>(observer);
    if(macOSBackend)
    {
        if(qname == QString::fromCFString(kTISNotifySelectedKeyboardInputSourceChanged))
            macOSBackend->handleInputSourceChangedNotification();
    }
}

void SystemTextInputManagerBackend_macOS::registerNotificationHooks()
{
    CFNotificationCenterAddObserver(
            CFNotificationCenterGetDistributedCenter(),
            this,
            &macOSNotificationHandler,
            kTISNotifySelectedKeyboardInputSourceChanged,
            NULL,
            CFNotificationSuspensionBehaviorDeliverImmediately
        );
}

void SystemTextInputManagerBackend_macOS::unregisterNotificationHooks()
{
    CFNotificationCenterRemoveEveryObserver(
            CFNotificationCenterGetDistributedCenter(),
            this
        );
}

void SystemTextInputManagerBackend_macOS::handleInputSourceChangedNotification()
{
    this->determineSelectedInputSource();
}

///////////////////////////////////////////////////////////////////////////////

SystemTextInputSource_macOS::SystemTextInputSource_macOS(TISInputSourceRef inputSource, SystemTextInputManager *parent)
    : AbstractSystemTextInputSource(parent), m_inputSource(inputSource)
{
    m_id = QString::fromCFString( (CFStringRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceID) );
    m_displayName = QString::fromCFString( (CFStringRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyLocalizedName) );
    this->setSelected( (CFBooleanRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceIsSelected) == kCFBooleanTrue );
}

SystemTextInputSource_macOS::~SystemTextInputSource_macOS()
{

}

void SystemTextInputSource_macOS::select()
{
    TISInputSourceRef inputSource = (TISInputSourceRef)m_inputSource;
    TISSelectInputSource(inputSource);
    this->updateSelectionStatus();
}

void SystemTextInputSource_macOS::updateSelectionStatus()
{
    const bool flag = (CFBooleanRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceIsSelected) == kCFBooleanTrue;
    this->setSelected(flag);
}
