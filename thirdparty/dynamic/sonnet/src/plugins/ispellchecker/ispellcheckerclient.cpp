/*
    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ispellcheckerclient.h"
#include "ispellcheckerdebug.h"
#include "ispellcheckerdict.h"

using namespace Sonnet;

ISpellCheckerClient::ISpellCheckerClient(QObject *parent)
    : Client(parent)
{
    qCDebug(SONNET_ISPELLCHECKER) << " ISpellCheckerClient::ISpellCheckerClient";

    // init com if needed, use same variant as e.g. Qt in qtbase/src/corelib/io/qfilesystemengine_win.cpp
    CoInitialize(nullptr);

    // get factory & collect all known languages + instantiate the spell checkers for them
    ISpellCheckerFactory *spellCheckerFactory = nullptr;
    if (SUCCEEDED(CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spellCheckerFactory))) && spellCheckerFactory) {
        // if we have a factory, cache the language names
        IEnumString *enumLanguages = nullptr;
        if (SUCCEEDED(spellCheckerFactory->get_SupportedLanguages(&enumLanguages))) {
            HRESULT hr = S_OK;
            while (S_OK == hr) {
                LPOLESTR string = nullptr;
                hr = enumLanguages->Next(1, &string, nullptr);
                if (S_OK == hr) {
                    ISpellChecker *spellChecker = nullptr;
                    if (SUCCEEDED(spellCheckerFactory->CreateSpellChecker(string, &spellChecker)) && spellChecker) {
                        m_languages.insert(QString::fromWCharArray(string), spellChecker);
                    }
                    CoTaskMemFree(string);
                }
            }
            enumLanguages->Release();
        }
        spellCheckerFactory->Release();
    }
}

ISpellCheckerClient::~ISpellCheckerClient()
{
    // FIXME: we at the moment leak all checkers as sonnet does the cleanup to late for proper com cleanup :/
}

SpellerPlugin *ISpellCheckerClient::createSpeller(const QString &language)
{
    // create requested spellchecker if we know the language
    qCDebug(SONNET_ISPELLCHECKER) << " SpellerPlugin *ISpellCheckerClient::createSpeller(const QString &language) ;" << language;
    const auto it = m_languages.find(language);
    if (it != m_languages.end()) {
        return new ISpellCheckerDict(it.value(), language);
    }
    return nullptr;
}

QStringList ISpellCheckerClient::languages() const
{
    return m_languages.keys();
}

#include "moc_ispellcheckerclient.cpp"
