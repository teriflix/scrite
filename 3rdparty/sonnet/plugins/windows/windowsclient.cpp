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

#include "windowsclient.h"
#include <QtDebug>
#include <QTimer>

#include <loader_p.h>
#include <settings_p.h>
#include <spellcheck.h>

Q_LOGGING_CATEGORY(SONNET_WINDOWS_ISPELLCHECKER, "SONNET_NSSPELLCHECKER")

namespace Microsoft {
class COM
{
public:
    COM() { }

    ~COM()
    {
        if (this->isValid())
            CoUninitialize();
    }

    void initialize()
    {
        if (hr == S_FALSE)
            this->hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }

    bool isValid() const { return SUCCEEDED(hr); }

private:
    HRESULT hr = S_FALSE;
};

template<class T>
class COMInterface
{
public:
    COMInterface() { }

    COMInterface(T *ptr)
    {
        m_pointer = ptr;
        if (m_pointer != nullptr)
            m_pointer->AddRef();
    }

    ~COMInterface()
    {
        if (m_pointer != nullptr)
            m_pointer->Release();
        m_pointer = nullptr;
    }

    COMInterface &operator=(const COMInterface &other) { return *this = other.m_pointer; }

    COMInterface &operator=(T *ptr)
    {
        if (m_pointer != nullptr)
            m_pointer->Release();
        m_pointer = ptr;
        if (m_pointer != nullptr)
            m_pointer->AddRef();
        return *this;
    }

    bool operator==(const COMInterface &other) const { return m_pointer == other.m_pointer; }

    T *operator->() { return m_pointer; }
    const T *operator->() const { return m_pointer; }

    bool isNull() const { return m_pointer == nullptr; }

private:
    T *m_pointer = nullptr;
};

QStringList toStringList(IEnumString *enumeration, int max = INT_MAX)
{
    QStringList ret;

    HRESULT hr = S_OK;
    while (S_OK == hr && ret.size() < max) {
        LPOLESTR string = nullptr;
        hr = enumeration->Next(1, &string, nullptr);
        if (S_OK == hr) {
            ret << QString::fromWCharArray(string);
            CoTaskMemFree(string);
        }
    }

    return ret;
}
}

struct WindowsClientData
{
    Microsoft::COMInterface<ISpellCheckerFactory> spellCheckerFactory;
    QStringList supportedLanguages;
};

Q_GLOBAL_STATIC(Microsoft::COM, COMSubSystem)

WindowsClient::WindowsClient(QObject *parent) : Sonnet::Client(parent)
{
    d = new WindowsClientData;

    ISpellCheckerFactory *spellCheckerFactory = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&spellCheckerFactory));
    if (hr == CO_E_NOTINITIALIZED) {
        ::COMSubSystem->initialize();
        hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&spellCheckerFactory));
    }

    if (SUCCEEDED(hr)) {
        d->spellCheckerFactory = spellCheckerFactory;

        IEnumString *enumLanguages = nullptr;
        hr = spellCheckerFactory->get_SupportedLanguages(&enumLanguages);
        if (SUCCEEDED(hr)) {
            d->supportedLanguages = Microsoft::toStringList(enumLanguages);
            enumLanguages->Release();
        }

        spellCheckerFactory->Release();
    }
}

WindowsClient::~WindowsClient()
{
    delete d;
}

int WindowsClient::reliability() const
{
    return qEnvironmentVariableIsSet("SONNET_PREFER_NSSPELLCHECKER") ? 9999 : 30;
}

Sonnet::SpellerPlugin *WindowsClient::createSpeller(const QString &language)
{
    const int languageIndex = d->supportedLanguages.contains(language);
    if (languageIndex < 0)
        return nullptr;

    return new WindowsSpellerPlugin(language, d);
}

QStringList WindowsClient::languages() const
{
    return d->supportedLanguages;
}

QString WindowsClient::defaultEnglishLanguage() const
{
    // Pick an English dictionary in the given order.
    const QStringList engLangs = QStringList()
            << QStringLiteral("en-IN") << QStringLiteral("en-US") << QStringLiteral("en-GB");
    for (QString engLang : engLangs) {
        if (d->supportedLanguages.contains(engLang))
            return engLang;
    }

    // Pick any en- language.
    for (QString lang : d->supportedLanguages) {
        if (lang.startsWith(QStringLiteral("en-")))
            return lang;
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////

struct WindowsSpellerPluginData
{
    QString language;
    Microsoft::COMInterface<ISpellChecker> spellChecker;
    QSet<QString> sessionWords;
    WindowsClientData *clientData = nullptr;
    QStringList customWords;
};

WindowsSpellerPlugin::WindowsSpellerPlugin(const QString &language, WindowsClientData *clientData)
    : Sonnet::SpellerPlugin(language)
{
    d = new WindowsSpellerPluginData;
    d->language = language;
    d->clientData = clientData;

    if (!clientData->spellCheckerFactory.isNull()) {
        ISpellChecker *spellChecker = nullptr;

        const std::wstring wideString = language.toStdWString();
        HRESULT hr = clientData->spellCheckerFactory->CreateSpellChecker(wideString.data(),
                                                                         &spellChecker);
        if (SUCCEEDED(hr)) {
            d->spellChecker = spellChecker;
            spellChecker->Release();
        }
    }
}

WindowsSpellerPlugin::~WindowsSpellerPlugin()
{
    delete d;
}

bool WindowsSpellerPlugin::isCorrect(const QString &word) const
{
    return this->suggest(word, 1).isEmpty();
}

QStringList WindowsSpellerPlugin::suggest(const QString &word) const
{
    return this->suggest(word, INT_MAX);
}

bool WindowsSpellerPlugin::checkAndSuggest(const QString &word, QStringList &suggestions) const
{
    return Sonnet::SpellerPlugin::checkAndSuggest(word, suggestions);
}

bool WindowsSpellerPlugin::storeReplacement(const QString &bad, const QString &good)
{
    qCDebug(SONNET_WINDOWS_ISPELLCHECKER) << "Not storing replacement" << good << "for" << bad;
    return false;
}

bool WindowsSpellerPlugin::addToPersonal(const QString &word)
{
    if (d->spellChecker.isNull())
        return false;

    const std::wstring word2 = word.toStdWString();
    const HRESULT hr = d->spellChecker->Add(word2.data());
    return (hr == S_OK);
}

bool WindowsSpellerPlugin::addToSession(const QString &word)
{
    qCDebug(SONNET_WINDOWS_ISPELLCHECKER) << "Not storing" << word << "in the session dictionary";
    return false;
}

QStringList WindowsSpellerPlugin::suggest(const QString &word, int atMost) const
{
    QStringList ret;
    if (d->spellChecker.isNull())
        return ret;

    IEnumSpellingError *enumSpellingError = nullptr;
    const std::wstring wideWord = word.toStdWString();
    HRESULT hr = d->spellChecker->Check(wideWord.data(), &enumSpellingError);
    if (hr == S_OK) {
        ISpellingError *spellingError = nullptr;
        hr = enumSpellingError->Next(&spellingError);
        if (hr == S_OK) {
            CORRECTIVE_ACTION correctiveAction = CORRECTIVE_ACTION_NONE;
            hr = spellingError->get_CorrectiveAction(&correctiveAction);

            if (SUCCEEDED(hr)) {
                switch (correctiveAction) {
                case CORRECTIVE_ACTION_GET_SUGGESTIONS: {
                    IEnumString *enumSuggestions = nullptr;
                    hr = d->spellChecker->Suggest(wideWord.data(), &enumSuggestions);
                    if (SUCCEEDED(hr)) {
                        ret = Microsoft::toStringList(enumSuggestions, atMost);
                        enumSuggestions->Release();
                    }
                } break;
                case CORRECTIVE_ACTION_REPLACE: {
                    PWSTR replacement = nullptr;
                    hr = spellingError->get_Replacement(&replacement);
                    if (SUCCEEDED(hr)) {
                        ret << QString::fromWCharArray(replacement);
                        CoTaskMemFree(replacement);
                    }
                } break;
                case CORRECTIVE_ACTION_DELETE: {
                    if (atMost == 1)
                        ret << QString();
                } break;
                default:
                    break;
                }
            }

            spellingError->Release();
        }

        enumSpellingError->Release();
    }

    return ret;
}
