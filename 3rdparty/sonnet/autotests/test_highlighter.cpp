// krazy:excludeall=spelling
/**
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "highlighter.h"
#include "speller.h"

#include <QObject>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTest>

using namespace Sonnet;

class HighlighterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testEnglish();
    void testFrench();
    void testMultipleLanguages();
    void testForceLanguage();
};

void HighlighterTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    Speller speller(QStringLiteral("en_US"));
    if (!speller.availableBackends().contains(QLatin1String("ASpell"))) {
        QSKIP("ASpell not available");
    }
    // Doing this here affects all the highlighters created later on, due to the weird hidden "Settings" class saving stuff behind the API's back...
    speller.setDefaultClient(QStringLiteral("ASpell"));
    if (!speller.availableLanguages().contains(QLatin1String("en"))) {
        QSKIP("'en' not available");
    }

    // How weird to have to do this here and not with the Highlighter API....
    speller.setAttribute(Speller::AutoDetectLanguage, true);
}

static const char s_englishSentence[] = "Hello helo this is the highlighter test enviroment guvernment"; // words from test_suggest.cpp

void HighlighterTest::testEnglish()
{
    // GIVEN
    QPlainTextEdit textEdit;
    textEdit.setPlainText(QString::fromLatin1(s_englishSentence));
    Sonnet::Highlighter highlighter(&textEdit);
    highlighter.setCurrentLanguage(QStringLiteral("en"));
    if (!highlighter.spellCheckerFound()) {
        QSKIP("'en' not available");
    }
    highlighter.rehighlight();
    QTextCursor cursor(textEdit.document());

    // WHEN
    cursor.setPosition(6);
    const QStringList suggestionsForHelo = highlighter.suggestionsForWord(QStringLiteral("helo"), cursor);
    const QStringList unlimitedSuggestions = highlighter.suggestionsForWord(QStringLiteral("helo"), cursor, -1);
    cursor.setPosition(40);
    const QStringList suggestionsForEnviroment = highlighter.suggestionsForWord(QStringLiteral("enviroment"), cursor);

    // THEN
    QCOMPARE(suggestionsForHelo.count(), 10);
    QVERIFY2(suggestionsForHelo.contains(QLatin1String("hello")), qPrintable(suggestionsForHelo.join(QLatin1Char(','))));
    QVERIFY2(suggestionsForEnviroment.contains(QLatin1String("environment")), qPrintable(suggestionsForEnviroment.join(QLatin1Char(','))));
    QVERIFY(unlimitedSuggestions.count() > 10);
}

static const char s_frenchSentence[] = "Bnjour est un bon mot pour tester le dictionnare.";

void HighlighterTest::testFrench()
{
    // GIVEN
    QPlainTextEdit textEdit;
    textEdit.setPlainText(QString::fromLatin1(s_frenchSentence));
    Sonnet::Highlighter highlighter(&textEdit);
    highlighter.setCurrentLanguage(QStringLiteral("fr_FR"));
    if (!highlighter.spellCheckerFound()) {
        QSKIP("'fr_FR' not available");
    }
    highlighter.rehighlight();
    QTextCursor cursor(textEdit.document());

    // WHEN
    cursor.setPosition(0);
    const QStringList suggestionsForBnjour = highlighter.suggestionsForWord(QStringLiteral("Bnjour"), cursor);
    cursor.setPosition(37);
    const QStringList suggestionsForDict = highlighter.suggestionsForWord(QStringLiteral("dictionnare"), cursor);

    // THEN
    QVERIFY2(suggestionsForBnjour.contains(QLatin1String("Bonjour")), qPrintable(suggestionsForBnjour.join(QLatin1Char(','))));
    QVERIFY2(suggestionsForDict.contains(QLatin1String("dictionnaire")), qPrintable(suggestionsForDict.join(QLatin1Char(','))));
}

void HighlighterTest::testForceLanguage()
{
    // GIVEN
    QPlainTextEdit textEdit;
    textEdit.setPlainText(QString::fromLatin1(s_frenchSentence));
    Sonnet::Highlighter highlighter(&textEdit);
    highlighter.setCurrentLanguage(QStringLiteral("en"));
    highlighter.setAutoDetectLanguageDisabled(true);
    QVERIFY(highlighter.spellCheckerFound());
    QVERIFY(highlighter.autoDetectLanguageDisabled());
    highlighter.rehighlight();
    QCOMPARE(highlighter.currentLanguage(), QStringLiteral("en"));
    QTextCursor cursor(textEdit.document());

    // WHEN
    cursor.setPosition(0);
    const QStringList suggestionsForBnjour = highlighter.suggestionsForWord(QStringLiteral("Bnjour"), cursor);
    cursor.setPosition(37);
    const QStringList suggestionsForDict = highlighter.suggestionsForWord(QStringLiteral("dictionnare"), cursor);

    // THEN
    QVERIFY2(!suggestionsForBnjour.contains(QLatin1String("Bonjour")), qPrintable(suggestionsForBnjour.join(QLatin1Char(','))));
    QVERIFY2(!suggestionsForDict.contains(QLatin1String("dictionnaire")), qPrintable(suggestionsForDict.join(QLatin1Char(','))));
}

void HighlighterTest::testMultipleLanguages()
{
    // GIVEN
    QPlainTextEdit textEdit;
    const QString englishSentence = QString::fromLatin1(s_englishSentence) + QLatin1Char('\n');
    textEdit.setPlainText(englishSentence + QString::fromLatin1(s_frenchSentence));
    Sonnet::Highlighter highlighter(&textEdit);
    highlighter.rehighlight();
    QTextCursor cursor(textEdit.document());

    // create Speller to check if we have the language dictionaries available otherwise
    // this will just keep failing
    Sonnet::Speller speller;
    const auto availableLangs = speller.availableLanguages();
    bool isFrAvailable = availableLangs.indexOf(QRegularExpression(QStringLiteral("fr"))) != -1;
    bool isEnAvailable = availableLangs.indexOf(QRegularExpression(QStringLiteral("en"))) != -1;

    // WHEN
    cursor.setPosition(6);
    const QStringList suggestionsForHelo = highlighter.suggestionsForWord(QStringLiteral("helo"), cursor);
    cursor.setPosition(40);
    const QStringList suggestionsForEnviroment = highlighter.suggestionsForWord(QStringLiteral("enviroment"), cursor);
    cursor.setPosition(englishSentence.size());
    const QStringList suggestionsForBnjour = highlighter.suggestionsForWord(QStringLiteral("Bnjour"), cursor);
    cursor.setPosition(englishSentence.size() + 37);
    const QStringList suggestionsForDict = highlighter.suggestionsForWord(QStringLiteral("dictionnare"), cursor);

    // THEN
    if (isEnAvailable) {
        QVERIFY2(suggestionsForHelo.contains(QLatin1String("hello")), qPrintable(suggestionsForHelo.join(QLatin1Char(','))));
        QVERIFY2(suggestionsForEnviroment.contains(QLatin1String("environment")), qPrintable(suggestionsForEnviroment.join(QLatin1Char(','))));
    }
    if (isFrAvailable) {
        QVERIFY2(suggestionsForBnjour.contains(QLatin1String("Bonjour")), qPrintable(suggestionsForBnjour.join(QLatin1Char(','))));
        QVERIFY2(suggestionsForDict.contains(QLatin1String("dictionnaire")), qPrintable(suggestionsForDict.join(QLatin1Char(','))));
    }
}

QTEST_MAIN(HighlighterTest)

#include "test_highlighter.moc"
