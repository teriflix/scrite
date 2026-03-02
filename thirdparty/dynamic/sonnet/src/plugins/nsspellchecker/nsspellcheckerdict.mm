/*
 * nsspellcheckerdict.mm
 *
 * SPDX-FileCopyrightText: 2015 Nick Shaforostoff <shaforostoff@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "nsspellcheckerdict.h"
#include "nsspellcheckerdebug.h"

#import <AppKit/AppKit.h>

using namespace Sonnet;

NSSpellCheckerDict::NSSpellCheckerDict(const QString &lang)
    : SpellerPlugin(lang)
    , m_langCode([lang.toNSString() retain])
{
    NSSpellChecker *checker = [NSSpellChecker sharedSpellChecker];
    if ([checker setLanguage:m_langCode]) {
        qCDebug(SONNET_NSSPELLCHECKER) << "Loading dictionary for" << lang;
        [checker updatePanels];
    } else {
        qCWarning(SONNET_NSSPELLCHECKER) << "Loading dictionary for unsupported language" << lang;
    }
}

NSSpellCheckerDict::~NSSpellCheckerDict()
{
    [m_langCode release];
}

bool NSSpellCheckerDict::isCorrect(const QString &word) const
{
    NSString *nsWord = word.toNSString();
    NSSpellChecker *checker = [NSSpellChecker sharedSpellChecker];
    NSRange range = [checker checkSpellingOfString:nsWord
        startingAt:0 language:m_langCode
        wrap:NO inSpellDocumentWithTag:0 wordCount:nullptr];
    if (range.length == 0) {
        // Check if the user configured a replacement text for this string. Sadly
        // we can only signal an error if that's the case, Sonnet has no other way
        // to take such substitutions into account.
        if (NSDictionary *replacements = [checker userReplacementsDictionary]) {
            return [replacements objectForKey:nsWord] == nil;
        } else {
            return true;
        }
    }
    return false;
}

QStringList NSSpellCheckerDict::suggest(const QString &word) const
{
    NSString *nsWord = word.toNSString();
    NSSpellChecker *checker = [NSSpellChecker sharedSpellChecker];
    NSArray *suggestions = [checker guessesForWordRange:NSMakeRange(0, word.length())
        inString:nsWord language:m_langCode inSpellDocumentWithTag:0];
    QStringList lst;
    NSDictionary *replacements = [checker userReplacementsDictionary];
    QString replacement;
    if ([replacements objectForKey:nsWord]) {
        // return the replacement text from the userReplacementsDictionary first.
        replacement = QString::fromNSString([replacements valueForKey:nsWord]);
        lst << replacement;
    }
    for (NSString *suggestion in suggestions) {
        // the replacement text from the userReplacementsDictionary will be in
        // the suggestions list; don't add it again.
        QString str = QString::fromNSString(suggestion);
        if (str != replacement) {
            lst << str;
        }
    }
    return lst;
}

bool NSSpellCheckerDict::storeReplacement(const QString &bad,
                                    const QString &good)
{
    qCDebug(SONNET_NSSPELLCHECKER) << "Not storing replacement" << good << "for" << bad;
    return false;
}

bool NSSpellCheckerDict::addToPersonal(const QString &word)
{
    NSString *nsWord = word.toNSString();
    NSSpellChecker *checker = [NSSpellChecker sharedSpellChecker];
    if (![checker hasLearnedWord:nsWord]) {
        [checker learnWord:nsWord];
        [checker updatePanels];
    }
    return true;
}

bool NSSpellCheckerDict::addToSession(const QString &word)
{
    qCDebug(SONNET_NSSPELLCHECKER) << "Not storing" << word << "in the session dictionary";
    return false;
}
