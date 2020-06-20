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

#include "windowsclient.h"
#include <QtDebug>

#include <spellcheck.h>

namespace Microsoft
{
    class COM
    {
    public:
        COM() {}

        ~COM() {
            if(this->isValid())
                CoUninitialize();
        }

        void initialize() {
            if(hr == S_FALSE)
                this->hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        }

        bool isValid() const {
            return SUCCEEDED(hr);
        }

    private:
        HRESULT hr = S_FALSE;
    };

    template <class T>
    class COMInterface
    {
    public:
        COMInterface() { }

        COMInterface(T *ptr) {
            m_pointer = ptr;
            if(m_pointer != nullptr)
                m_pointer->AddRef();
        }
        ~COMInterface() {
            if(m_pointer != nullptr)
                m_pointer->Release();
        }

        COMInterface &operator = (const COMInterface &other) {
            if(m_pointer != nullptr)
                m_pointer->Release();
            m_pointer = other.m_pointer;
            if(m_pointer != nullptr)
                m_pointer->AddRef();
            return *this;
        }

        COMInterface &operator = (T *ptr) {
            if(m_pointer != nullptr)
                m_pointer->Release();
            m_pointer = ptr;
            if(m_pointer != nullptr)
                m_pointer->AddRef();
            return *this;
        }

        bool operator == (const COMInterface &other) const {
            return m_pointer == other.m_pointer;
        }

        T *operator -> () { return m_pointer; }
        const T *operator -> () const { return m_pointer; }

        bool isNull() const { return m_pointer == nullptr; }

    private:
        T *m_pointer = nullptr;
    };
}

struct WindowsClientData
{
    Microsoft::COMInterface<ISpellCheckerFactory> spellCheckerFactory;
    QStringList supportedLanguages;
};

Q_GLOBAL_STATIC(Microsoft::COM, COMSubSystem)

WindowsClient::WindowsClient(QObject *parent)
    : Sonnet::Client(parent)
{
    d = new WindowsClientData;

    ISpellCheckerFactory* spellCheckerFactory = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spellCheckerFactory));
    if(hr == CO_E_NOTINITIALIZED)
    {
        ::COMSubSystem->initialize();
        hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spellCheckerFactory));
    }

    if (SUCCEEDED(hr))
    {
        d->spellCheckerFactory = spellCheckerFactory;

        IEnumString* enumLanguages = nullptr;
        hr = spellCheckerFactory->get_SupportedLanguages(&enumLanguages);
        if (SUCCEEDED(hr))
        {
            HRESULT hr = S_OK;
            while (S_OK == hr)
            {
                LPOLESTR string = nullptr;
                hr = enumLanguages->Next(1, &string, nullptr);
                if (S_OK == hr)
                {
                    d->supportedLanguages << QString::fromWCharArray(string);
                    CoTaskMemFree(string);
                }
            }
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
    if(languageIndex < 0)
        return new WindowsSpellerPlugin("en-US", d);

    return new WindowsSpellerPlugin(language, d);
}

QStringList WindowsClient::languages() const
{
    return d->supportedLanguages;
}

///////////////////////////////////////////////////////////////////////////////

struct WindowsSpellerPluginData
{
    Microsoft::COMInterface<ISpellChecker> spellChecker;
};

WindowsSpellerPlugin::WindowsSpellerPlugin(const QString &language, WindowsClientData *clientData)
    : Sonnet::SpellerPlugin(language)
{
    d = new WindowsSpellerPluginData;

    if(!clientData->spellCheckerFactory.isNull())
    {
        ISpellChecker *spellChecker = nullptr;

        const std::wstring wideString = language.toStdWString();
        HRESULT hr = clientData->spellCheckerFactory->CreateSpellChecker(wideString.data(), &spellChecker);
        if( SUCCEEDED(hr) )
        {
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
    if(d->spellChecker.isNull())
        return true;

    bool ret = true;
    IEnumSpellingError* enumSpellingError = nullptr;
    const std::wstring wideWord = word.toStdWString();
    HRESULT hr = d->spellChecker->Check(wideWord.data(), &enumSpellingError);
    if( SUCCEEDED(hr) )
    {
        ISpellingError* spellingError = nullptr;
        hr = enumSpellingError->Next(&spellingError);
        if(hr == S_OK)
        {
            spellingError->Release();
            ret = false;
        }

        enumSpellingError->Release();
    }

    return ret;
}

QStringList WindowsSpellerPlugin::suggest(const QString &word) const
{
    Q_UNUSED(word)
    return QStringList();
}

bool WindowsSpellerPlugin::checkAndSuggest(const QString &word, QStringList &suggestions) const
{
    Q_UNUSED(word)
    Q_UNUSED(suggestions)
    return true;
}

bool WindowsSpellerPlugin::storeReplacement(const QString &bad, const QString &good)
{
    Q_UNUSED(bad)
    Q_UNUSED(good)
    return true;
}

bool WindowsSpellerPlugin::addToPersonal(const QString &word)
{
    Q_UNUSED(word)
    return true;
}

bool WindowsSpellerPlugin::addToSession(const QString &word)
{
    Q_UNUSED(word)
    return true;
}
