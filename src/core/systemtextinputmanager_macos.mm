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

QList<AbstractSystemTextInputSource *> SystemTextInputManagerBackend_macOS::reloadSources()
{
    QList<AbstractSystemTextInputSource *> ret;
    CFArrayRef sourceList = TISCreateInputSourceList(NULL, false);
    const int nrSources = CFArrayGetCount(sourceList);

    for(int i=0; i<nrSources; i++)
    {
        TISInputSourceRef inputSource = (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, i);
        const void *inputSourceType = TISGetInputSourceProperty(inputSource, kTISPropertyInputSourceType);
        if( kTISTypeKeyboardInputMode != inputSourceType && kTISTypeKeyboardLayout != inputSourceType )
            continue;

        ret << new SystemTextInputSource_macOS(inputSource, this->inputManager());
    }

    return ret;
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
        else if(qname == QString::fromCFString(kTISNotifyEnabledKeyboardInputSourcesChanged))
            macOSBackend->handleInputSourcesChangedNotification();
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

    CFNotificationCenterAddObserver(
            CFNotificationCenterGetDistributedCenter(),
            this,
            &macOSNotificationHandler,
            kTISNotifyEnabledKeyboardInputSourcesChanged,
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

void SystemTextInputManagerBackend_macOS::handleInputSourcesChangedNotification()
{
    this->inputManager()->reload();
}

///////////////////////////////////////////////////////////////////////////////

SystemTextInputSource_macOS::SystemTextInputSource_macOS(TISInputSourceRef inputSource, SystemTextInputManager *parent)
    : AbstractSystemTextInputSource(parent), m_inputSource(inputSource)
{
    m_id = QString::fromCFString( (CFStringRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceID) );
    m_displayName = QString::fromCFString( (CFStringRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyLocalizedName) );

    CFArrayRef languages = (CFArrayRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceLanguages);
    const int nrLanguages = CFArrayGetCount(languages);
    if(nrLanguages >= 1)
    {
        const QString primaryLanguage = QString::fromCFString( (CFStringRef)CFArrayGetValueAtIndex(languages, 0) ).left(2);
        static const QStringList languageCodes = QStringList() << QStringLiteral("en") << QStringLiteral("bn") <<
            QStringLiteral("gu") << QStringLiteral("hi") << QStringLiteral("kn") << QStringLiteral("ml") <<
            QStringLiteral("mr") << QStringLiteral("or") << QStringLiteral("pa") << QStringLiteral("sa") <<
            QStringLiteral("ta") << QStringLiteral("te");
        const int languageIndex = languageCodes.indexOf(primaryLanguage);
        m_language = languageIndex;
    }

    this->setSelected( (CFBooleanRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceIsSelected) == kCFBooleanTrue );
}

SystemTextInputSource_macOS::~SystemTextInputSource_macOS()
{

}

void SystemTextInputSource_macOS::select()
{
    TISInputSourceRef inputSource = (TISInputSourceRef)m_inputSource;
    TISSelectInputSource(inputSource);
    this->checkSelection();
}

void SystemTextInputSource_macOS::checkSelection()
{
    const bool flag = (CFBooleanRef)TISGetInputSourceProperty(m_inputSource, kTISPropertyInputSourceIsSelected) == kCFBooleanTrue;
    this->setSelected(flag);
}
