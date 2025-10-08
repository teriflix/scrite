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

#include "platformtransliterator_macos.h"

#import <Carbon/Carbon.h>

struct TextInputSource
{
    QString id;
    int languageCode = -1;
    QString displayName;
    TISInputSourceRef inputSource;

    bool operator==(const TextInputSource &other) const
    {
        return this->id == other.id && this->languageCode == other.languageCode
                && this->displayName == other.displayName && this->inputSource == other.inputSource;
    }
};

static void macOSNotificationHandler(CFNotificationCenterRef center, void *observer,
                                     CFStringRef name, const void *object, CFDictionaryRef userInfo)
{
    Q_UNUSED(object);
    Q_UNUSED(userInfo);

    if (center != CFNotificationCenterGetDistributedCenter())
        return;

    const QString qname = QString::fromCFString(name);

    MacOSBackend *macOSBackend = static_cast<MacOSBackend *>(observer);
    if (macOSBackend) {
        if (qname == QString::fromCFString(kTISNotifyEnabledKeyboardInputSourcesChanged))
            macOSBackend->reload();
        /*else if (qname == QString::fromCFString(kTISNotifySelectedKeyboardInputSourceChanged))

        */
    }
}

struct MacOSBackendData
{
    QList<TextInputSource> textInputSources;
};

MacOSBackend::MacOSBackend(QObject *parent) : QObject(parent), d(new MacOSBackendData)
{
    CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(), this,
                                    &macOSNotificationHandler,
                                    kTISNotifyEnabledKeyboardInputSourcesChanged, NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);

    /*CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(), this,
                                    &macOSNotificationHandler,
                                    kTISNotifySelectedKeyboardInputSourceChanged, NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);*/

    this->reload();
}

MacOSBackend::~MacOSBackend()
{
    delete d;
}

QList<TransliterationOption>
MacOSBackend::options(int lang, const PlatformTransliterationEngine *transliterator) const
{
    QList<TransliterationOption> ret;

    for (const TextInputSource &tis : qAsConst(d->textInputSources)) {
        if (tis.languageCode == lang)
            ret << TransliterationOption(
                    { (QObject *)transliterator, lang, tis.id, tis.displayName, false });
    }

    return ret;
}

bool MacOSBackend::canActivate(const TransliterationOption &option,
                               PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(transliterator)

    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [option](const TextInputSource &tis) { return option.id == tis.id; });
    return (it != d->textInputSources.end());
}

bool MacOSBackend::activate(const TransliterationOption &option,
                            PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(transliterator)

    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [option](const TextInputSource &tis) { return option.id == tis.id; });

    if (it != d->textInputSources.end()) {
        const bool alreadyActive = (CFBooleanRef)TISGetInputSourceProperty(it->inputSource, kTISPropertyInputSourceIsSelected) == kCFBooleanTrue;
        if(alreadyActive)
            return true;

        TISInputSourceRef inputSource = (TISInputSourceRef)it->inputSource;
        return TISSelectInputSource(inputSource) == noErr;
    }

    return false;
}

bool MacOSBackend::release(const TransliterationOption &option,
                           PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(option)
    Q_UNUSED(transliterator)
    return true;
}

bool MacOSBackend::reload()
{
    QList<TextInputSource> textInputSources;

    CFArrayRef sourceList = TISCreateInputSourceList(NULL, false);
    const int nrSources = CFArrayGetCount(sourceList);

    for (int i = 0; i < nrSources; i++) {
        TextInputSource tis;

        tis.inputSource = (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, i);
        const void *inputSourceType =
                TISGetInputSourceProperty(tis.inputSource, kTISPropertyInputSourceType);
        if (kTISTypeKeyboardInputMode != inputSourceType
            && kTISTypeKeyboardLayout != inputSourceType)
            continue;

        tis.id = QString::fromCFString(
                (CFStringRef)TISGetInputSourceProperty(tis.inputSource, kTISPropertyInputSourceID));
        tis.displayName = QString::fromCFString(
                (CFStringRef)TISGetInputSourceProperty(tis.inputSource, kTISPropertyLocalizedName));

        CFArrayRef languages = (CFArrayRef)TISGetInputSourceProperty(
                tis.inputSource, kTISPropertyInputSourceLanguages);
        const int nrLanguages = CFArrayGetCount(languages);
        if (nrLanguages >= 1) {
            const QString primaryLanguage =
                    QString::fromCFString((CFStringRef)CFArrayGetValueAtIndex(languages, 0))
                            .left(2);
            const QLocale keyboardLocale(primaryLanguage);
            tis.languageCode = keyboardLocale.language();

            textInputSources << tis;
        }
    }

    if (d->textInputSources != textInputSources) {
        d->textInputSources = textInputSources;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

Q_GLOBAL_STATIC(MacOSBackend, Backend)

PlatformTransliterationEngine::PlatformTransliterationEngine(QObject *parent)
    : AbstractTransliterationEngine(parent)
{
    connect(::Backend, &MacOSBackend::textInputSourcesChanged, this,
            &PlatformTransliterationEngine::capacityChanged);
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("macOS");
}

QList<TransliterationOption> PlatformTransliterationEngine::options(int lang) const
{
    return ::Backend->options(lang, this);
}

bool PlatformTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return option.transliteratorObject == this && ::Backend->canActivate(option, this);
}

bool PlatformTransliterationEngine::activate(const TransliterationOption &option)
{
    return ::Backend->activate(option, this);
}

void PlatformTransliterationEngine::release(const TransliterationOption &option)
{
    ::Backend->release(option, this);
}

QString PlatformTransliterationEngine::transliterateWord(const QString &word,
                                                         const TransliterationOption &option) const
{
    // No need to implement this, because platform transliterators don't offer in-app
    // transliterations.
    Q_UNUSED(option);
    return word;
}
