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
    bool isDefault = false;

    bool operator==(const TextInputSource &other) const
    {
        return this->inputSource == other.inputSource;
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

int MacOSBackend::defaultLanguage() const
{
    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [](const TextInputSource &tis) { return tis.isDefault; });
    if(it != d->textInputSources.end())
        return it->languageCode;

    return -1;
}

int MacOSBackend::activateDefaultLanguage() const
{
    // Find the default keyboard layout
    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [](const TextInputSource &tis) { return tis.isDefault; });

    // If found, then activate it.
    if (it != d->textInputSources.end()) {
        const bool alreadyActive = (CFBooleanRef)TISGetInputSourceProperty(it->inputSource, kTISPropertyInputSourceIsSelected) == kCFBooleanTrue;
        if(alreadyActive)
            return it->languageCode;

        TISInputSourceRef inputSource = (TISInputSourceRef)it->inputSource;
        if(TISSelectInputSource(inputSource) == noErr)
            return it->languageCode;
    }

    // Report error otherwise
    return -1;
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
    TISInputSourceRef defaultSource = (nrSources > 0)
            ? (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, 0)
            : NULL;

    for (int i = 0; i < nrSources; i++) {
        TextInputSource tis;

        tis.inputSource = (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, i);
        const void *inputSourceType =
                TISGetInputSourceProperty(tis.inputSource, kTISPropertyInputSourceType);
        if (kTISTypeKeyboardInputMode != inputSourceType
            && kTISTypeKeyboardLayout != inputSourceType)
            continue;

        tis.isDefault = (tis.inputSource == defaultSource);
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

    CFRelease(sourceList);

    if (d->textInputSources != textInputSources) {
        d->textInputSources = textInputSources;
        emit textInputSourcesChanged();
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
    connect(::Backend, &MacOSBackend::textInputSourcesChanged, this,
            &PlatformTransliterationEngine::defaultLanguageChanged);
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("macOS");
}

int PlatformTransliterationEngine::defaultLanguage() const
{
    return ::Backend->defaultLanguage();
}

int PlatformTransliterationEngine::activateDefaultLanguage()
{
    // Ask the backend to activate the default language
    int code = ::Backend->activateDefaultLanguage();
    if (code >= 0)
        return code;

           // If it couldn't then fallback to English as default language
    const QList<TransliterationOption> englishOptions = this->options(QLocale::English);
    if (englishOptions.isEmpty())
        return -1; // No clue what to do now.

    if (this->activate(englishOptions.first()))
        return QLocale::English;

    return -1;
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

QString PlatformTransliterationEngine::transliterateWord(const QString &word,
                                                         const TransliterationOption &option) const
{
    // No need to implement this, because platform transliterators don't offer in-app
    // transliterations.
    Q_UNUSED(option);
    return word;
}
