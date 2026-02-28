/*
 * kspell_hunspelldict.cpp
 *
 * SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "hunspelldict.h"

#include "config-hunspell.h"
#include "hunspelldebug.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

using namespace Sonnet;

HunspellDict::HunspellDict(const QString &lang, const std::shared_ptr<Hunspell> &speller)
    : SpellerPlugin(lang)
{
    if (!speller) {
        qCWarning(SONNET_HUNSPELL) << "Can't create a client without a speller";
        return;
    }
    m_decoder = QStringDecoder(speller->get_dic_encoding());
    if (!m_decoder.isValid()) {
        qCWarning(SONNET_HUNSPELL) << "Failed to find a text codec for name" << speller->get_dic_encoding() << "defaulting to locale text codec";
        m_decoder = QStringDecoder(QStringDecoder::System);
        Q_ASSERT(m_decoder.isValid());
    }
    m_encoder = QStringEncoder(speller->get_dic_encoding());
    if (!m_encoder.isValid()) {
        qCWarning(SONNET_HUNSPELL) << "Failed to find a text codec for name" << speller->get_dic_encoding() << "defaulting to locale text codec";
        m_encoder = QStringEncoder(QStringEncoder::System);
        Q_ASSERT(m_encoder.isValid());
    }
    m_speller = speller;

    const QString userDic = QDir::home().filePath(QLatin1String(".hunspell_") % lang);
    QFile userDicFile(userDic);
    if (userDicFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCDebug(SONNET_HUNSPELL) << "Load a user dictionary" << userDic;
        QTextStream userDicIn(&userDicFile);
        while (!userDicIn.atEnd()) {
            const QString word = userDicIn.readLine();
            if (word.isEmpty()) {
                continue;
            }

            if (word.contains(QLatin1Char('/'))) {
                QStringList wordParts = word.split(QLatin1Char('/'));
                speller->add_with_affix(toDictEncoding(wordParts.at(0)).constData(), toDictEncoding(wordParts.at(1)).constData());
            }
            if (word.at(0) == QLatin1Char('*')) {
                speller->remove(toDictEncoding(word.mid(1)).constData());
            } else {
                speller->add(toDictEncoding(word).constData());
            }
        }
        userDicFile.close();
    }
}

std::shared_ptr<Hunspell> HunspellDict::createHunspell(const QString &lang, QString path)
{
    qCDebug(SONNET_HUNSPELL) << "Loading dictionary for" << lang << "from" << path;

    if (!path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
    }
    path += lang;
    QString dictionary = path + QStringLiteral(".dic");
    QString aff = path + QStringLiteral(".aff");

    if (!QFileInfo::exists(dictionary) || !QFileInfo::exists(aff)) {
        qCWarning(SONNET_HUNSPELL) << "Unable to find dictionary for" << lang << "in path" << path;
        return nullptr;
    }

    std::shared_ptr<Hunspell> speller = std::make_shared<Hunspell>(aff.toLocal8Bit().constData(), dictionary.toLocal8Bit().constData());
    qCDebug(SONNET_HUNSPELL) << "Created " << speller.get();

    return speller;
}

HunspellDict::~HunspellDict()
{
}

QByteArray HunspellDict::toDictEncoding(const QString &word) const
{
    if (m_encoder.isValid()) {
        return m_encoder.encode(word);
    }
    return {};
}

bool HunspellDict::isCorrect(const QString &word) const
{
    qCDebug(SONNET_HUNSPELL) << " isCorrect :" << word;
    if (!m_speller) {
        return false;
    }

#if USE_OLD_HUNSPELL_API
    int result = m_speller->spell(toDictEncoding(word).constData());
    qCDebug(SONNET_HUNSPELL) << " result :" << result;
    return result != 0;
#else
    bool result = m_speller->spell(toDictEncoding(word).toStdString());
    qCDebug(SONNET_HUNSPELL) << " result :" << result;
    return result;
#endif
}

QStringList HunspellDict::suggest(const QString &word) const
{
    if (!m_speller) {
        return QStringList();
    }

    QStringList lst;
#if USE_OLD_HUNSPELL_API
    char **selection;
    int nbWord = m_speller->suggest(&selection, toDictEncoding(word).constData());
    for (int i = 0; i < nbWord; ++i) {
        lst << m_decoder.decode(selection[i]);
    }
    m_speller->free_list(&selection, nbWord);
#else
    const auto suggestions = m_speller->suggest(toDictEncoding(word).toStdString());
    for_each(suggestions.begin(), suggestions.end(), [this, &lst](const std::string &suggestion) {
        lst << m_decoder.decode(suggestion.c_str());
    });
#endif

    return lst;
}

bool HunspellDict::storeReplacement(const QString &bad, const QString &good)
{
    Q_UNUSED(bad);
    Q_UNUSED(good);
    if (!m_speller) {
        return false;
    }
    qCDebug(SONNET_HUNSPELL) << "HunspellDict::storeReplacement not implemented";
    return false;
}

bool HunspellDict::addToPersonal(const QString &word)
{
    if (!m_speller) {
        return false;
    }
    m_speller->add(toDictEncoding(word).constData());
    const QString userDic = QDir::home().filePath(QLatin1String(".hunspell_") % language());
    QFile userDicFile(userDic);
    if (userDicFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&userDicFile);
        out << word << '\n';
        userDicFile.close();
        return true;
    }

    return false;
}

bool HunspellDict::addToSession(const QString &word)
{
    if (!m_speller) {
        return false;
    }
    int r = m_speller->add(toDictEncoding(word).constData());
    return r == 0;
}
