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
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("Windows");
}

QList<TransliterationOption> PlatformTransliterationEngine::options(int lang) const
{
    return ::Backend->options(lang, this);
}

bool PlatformTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return option.transliteratorObject == this && ::Backend->canActivate(option, this);
}

void PlatformTransliterationEngine::activate(const TransliterationOption &option)
{
    ::Backend->activate(option, this);
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

///////////////////////////////////////////////////////////////////////////////

#include "Windows.h"

struct TextInputSource
{
    HKL hkl = nullptr;
    int languageCode = -1;
    QString id;
    QString displayName;

    bool operator==(const TextInputSource &other) const
    {
        return this->hkl == other.hkl && this->languageCode == other.languageCode
                && this->id == other.id && this->displayName == other.displayName;
    }
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
        ActivateKeyboardLayout(it->hkl, 0);
        return true;
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

            textInputSources << TextInputSource(
                    { keyboard, keyboardLocale.language(), id, displayName });
        }

        if (d->textInputSources != textInputSources) {
            d->textInputSources = textInputSources;
            return true;
        }
    }

    return false;
}
