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

#include "languageengine.h"
#include "platformtransliterator_windows.h"

#include <QGuiApplication>
#include <QAbstractNativeEventFilter>

Q_GLOBAL_STATIC(WindowsBackend, Backend)

///////////////////////////////////////////////////////////////////////////////

PlatformTransliterationEngine::PlatformTransliterationEngine(QObject *parent)
    : AbstractTransliterationEngine(parent)
{
    connect(::Backend, &WindowsBackend::textInputSourcesChanged, this,
            &PlatformTransliterationEngine::capacityChanged);
    connect(::Backend, &MacOSBackend::textInputSourcesChanged, this,
            &PlatformTransliterationEngine::defaultLanguageChanged);
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("Windows");
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

///////////////////////////////////////////////////////////////////////////////

#include "Windows.h"

struct TextInputSource
{
    HKL hkl = nullptr;
    int languageCode = -1;
    QString id;
    QString displayName;
    bool isDefault = false;

    bool operator==(const TextInputSource &other) const { return this->hkl == other.hkl; }
};

struct WindowsBackendData
{
    QList<TextInputSource> textInputSources;
};

WindowsBackend::WindowsBackend(QObject *parent) : QObject(parent), d(new WindowsBackendData)
{
    qApp->installEventFilter(this);

    this->reload();
}

WindowsBackend::~WindowsBackend()
{
    delete d;
}

int WindowsBackend::defaultLanguage() const
{
    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [](const TextInputSource &tis) { return tis.isDefault; });
    if (it != d->textInputSources.end())
        return it->languageCode;

    return -1;
}

int WindowsBackend::activateDefaultLanguage() const
{
    // Find the default keyboard layout
    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [](const TextInputSource &tis) { return tis.isDefault; });

    // If found, then activate it.
    if (it != d->textInputSources.end()) {
        ActivateKeyboardLayout(it->hkl, KLF_SETFORPROCESS);
        return GetKeyboardLayout(0) == it->hkl ? it->languageCode : -1;
    }

    // Report error otherwise
    return -1;
}

QList<TransliterationOption>
WindowsBackend::options(int lang, const PlatformTransliterationEngine *transliterator) const
{
    QList<TransliterationOption> ret;

    for (const TextInputSource &tis : qAsConst(d->textInputSources)) {
        if (tis.languageCode == lang)
            ret << TransliterationOption(
                    { (QObject *)transliterator, lang, tis.id, tis.displayName, false });
    }

    return ret;
}

bool WindowsBackend::canActivate(const TransliterationOption &option,
                                 PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(transliterator)

    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [option](const TextInputSource &tis) { return option.id == tis.id; });
    return (it != d->textInputSources.end());
}

bool WindowsBackend::activate(const TransliterationOption &option,
                              PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(transliterator)

    auto it = std::find_if(d->textInputSources.begin(), d->textInputSources.end(),
                           [option](const TextInputSource &tis) { return option.id == tis.id; });

    if (it != d->textInputSources.end()) {
        if (GetKeyboardLayout(0) == it->hkl)
            return true;

        ActivateKeyboardLayout(it->hkl, KLF_SETFORPROCESS);
        return GetKeyboardLayout(0) == it->hkl;
    }

    return false;
}

bool WindowsBackend::release(const TransliterationOption &option,
                             PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(option)
    Q_UNUSED(transliterator)
    return true;
}

bool WindowsBackend::eventFilter(QObject *object, QEvent *event)
{
    /**
      Windows doesn't seem to have a WM_ notification for when the set of input sources
      change. But what's clear is that the user will have to switch away from our application
      into Control Panel (or Settings on Windows 10) to add/remove input sources.

       So, everytime our application is activated; we reload all input sources just to make sure
       that our list of input-sources is current. This will result in unfortunate reloads for
       when the user has not added or removed input sources between the time our application
       lost focus and got it back, but its a price we have to pay for Windows not giving us any
       WM_ message to notify us.
       */
    if (event->type() == QEvent::ApplicationStateChange && object == qApp
        && qApp->applicationState() == Qt::ApplicationActive) {
        if (this->reload())
            emit textInputSourcesChanged();
    }

    return false;
}

bool WindowsBackend::reload()
{
    int nrKeyboards = GetKeyboardLayoutList(0, nullptr);
    if (nrKeyboards > 0) {
        QList<TextInputSource> textInputSources;

        HKL defaultHkl = nullptr;
        SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &defaultHkl, 0);

        QVector<HKL> keyboards(nrKeyboards, nullptr);
        nrKeyboards = GetKeyboardLayoutList(nrKeyboards, keyboards.data());
        for (int i = 0; i < nrKeyboards; i++) {
            HKL keyboard = keyboards.at(i);

            // HKL is a 4-byte number where last 2 bytes is language code and first 2 bytes is
            // layout type
            LANGID languageId = LANGID(quint32(keyboard) & 0x0000FFFF);
            int layoutType = (quint32(keyboard) & 0xFFFF0000) >> 16;
            auto toHex = [](const int number) {
                QString ret = QString::number(number, 16).toUpper();
                if (ret.length() < 4)
                    ret.prepend(QString(4 - ret.length(), QChar('0')));
                return ret;
            };

            LCID locale = MAKELCID(languageId, SORT_DEFAULT);
            wchar_t name[256];
            memset(name, 0, sizeof(name));
            GetLocaleInfoW(locale, LOCALE_SLANGUAGE, name, 256);
            const QString id = toHex(languageId) + QStringLiteral("-") + toHex(layoutType);
            const QString displayName = QString::fromWCharArray(name);

            memset(name, 0, sizeof(name));
            GetLocaleInfoW(locale, LOCALE_SNAME, name, 256);
            const QLocale keyboardLocale(QString::fromWCharArray(name).left(2));

            textInputSources << TextInputSource({ keyboard, keyboardLocale.language(), id,
                                                  displayName, defaultHkl == keyboard });
        }

        if (d->textInputSources != textInputSources) {
            d->textInputSources = textInputSources;
            emit textInputSourcesChanged();
            return true;
        }
    }

    return false;
}
