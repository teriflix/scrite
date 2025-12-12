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

#include "spellcheckservice.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "languageengine.h"
#include "utils.h"

#include <QFuture>
#include <QJsonObject>
#include <QScopeGuard>
#include <QTimerEvent>
#include <QFutureWatcher>
#include <QThreadStorage>
#include <QtConcurrentRun>
#include <QRandomGenerator>
#include <QCoreApplication>

#include "3rdparty/sonnet/sonnet/src/core/speller.h"
#include "3rdparty/sonnet/sonnet/src/core/loader_p.h"
#include "3rdparty/sonnet/sonnet/src/core/textbreaks_p.h"

#ifdef Q_OS_MAC
#include "3rdparty/sonnet/sonnet/src/plugins/nsspellchecker/nsspellcheckerclient.h"
#endif

class EnglishLanguageSpeller : public Sonnet::Speller
{
public:
    static const QString languageName;

    EnglishLanguageSpeller() : Sonnet::Speller(EnglishLanguageSpeller::languageName) { }
    ~EnglishLanguageSpeller() { }
};

#ifdef Q_OS_MAC
const QString EnglishLanguageSpeller::languageName = QStringLiteral("en");
#else
#ifdef Q_OS_WIN
const QString EnglishLanguageSpeller::languageName;
#else
const QString EnglishLanguageSpeller::languageName = QStringLiteral("en_US");
#endif
#endif

struct SpellCheckServiceRequest
{
    QString text;
    int timestamp;
    QStringList characterNames;
    QStringList ignoreList;
};
Q_DECLARE_METATYPE(SpellCheckServiceRequest)

class SpellCheckServiceResult
{
    char padding[4];

public:
    SpellCheckServiceResult() { padding[0] = 0; }

    int timestamp = -1;
    QString text;
    QList<TextFragment> misspelledFragments;
};
Q_DECLARE_METATYPE(SpellCheckServiceResult)

class Spellers : public QObject
{
public:
    Spellers(const QList<int> &languageCodes)
    {
        this->reloadSpellers(languageCodes);

        QObject::connect(
                LanguageEngine::instance()->supportedLanguages(),
                &SupportedLanguages::languagesCodesChanged, this,
                [=](const QList<int> &languageCodes) { this->reloadSpellers(languageCodes); });
    }

    ~Spellers() { }

    QList<int> supportedLanguages() const { return m_supportedLanguages.keys(); }

    QStringList getSuggestions(const QString &word) const
    {
        if (word.isEmpty())
            return QStringList();

        QChar::Script wordScript = word.at(0).script();
        QList<Item> wordScriptItems;
        std::copy_if(m_items.begin(), m_items.end(), std::back_inserter(wordScriptItems),
                     [wordScript](const Item &item) { return item.script == wordScript; });
        if (wordScriptItems.isEmpty())
            return QStringList();

        QStringList ret;
        for (const Item &item : qAsConst(wordScriptItems)) {
            if (item.speller.isMisspelled(word)) {
                const QStringList suggestions = item.speller.suggest(word);
                ret += suggestions;
            }
        }

        return ret;
    }

    bool checkSpelling(const Sonnet::TextBreaks::Position &wordPosition,
                       const SpellCheckServiceRequest &request, TextFragment &fragment) const
    {
        const QString word = request.text.mid(wordPosition.start, wordPosition.length);
        if (word.isEmpty())
            return false;

        if (request.ignoreList.contains(word)
            || request.characterNames.contains(word, Qt::CaseInsensitive))
            return false;

        if (word.endsWith("\'s", Qt::CaseInsensitive)) {
            if (request.characterNames.contains(word.leftRef(word.length() - 2),
                                                Qt::CaseInsensitive))
                return false;
        }

        QChar::Script wordScript = word.at(0).script();
        QList<Item> wordScriptItems;
        std::copy_if(m_items.begin(), m_items.end(), std::back_inserter(wordScriptItems),
                     [wordScript](const Item &item) { return item.script == wordScript; });
        if (wordScriptItems.isEmpty())
            return false;

        QMap<QLocale::Language, QStringList> languageSuggestions;
        for (const Item &item : qAsConst(wordScriptItems)) {
            if (item.speller.isMisspelled(word)) {
                const QStringList suggestions = item.speller.suggest(word);
                languageSuggestions[item.language] = suggestions;
            }
        }

        if (languageSuggestions.size() == wordScriptItems.size()) {
            fragment = TextFragment(wordPosition.start, wordPosition.length, languageSuggestions);
            return true;
        }

        return false;
    }

private:
    void reloadSpellers(const QList<int> &languageCodes)
    {
        if (m_supportedLanguages.isEmpty()) {
            const QStringList languages = Sonnet::Loader::openLoader()->languages();
            for (QString language : languages) {
                m_supportedLanguages[QLocale(language).language()].append(language);
            }
        }

        m_items.clear();

        for (int code : languageCodes) {
            Item item;
            item.language = QLocale::Language(code);

            const QStringList languageNames =
                    m_supportedLanguages.value(item.language, QStringList());
            if (languageNames.size() == 1) {
                item.languageName = languageNames.first();
            } else {
                item.languageName = QLocale(item.language).name();
#ifdef Q_OS_WIN
                item.languageName = item.languageName.replace('_', '-');
#endif
                if (!languageNames.isEmpty() && !languageNames.contains(item.languageName))
                    item.languageName = languageNames.first();
            }
            item.script = Language::scriptForLanguage(item.language);
            item.speller = item.language == QLocale::English ? EnglishLanguageSpeller()
                                                             : Sonnet::Speller(item.languageName);

            if (item.speller.isValid()
                && (item.language == QLocale::English
                            ? true
                            : item.speller.language() == item.languageName))
                m_items.append(item);
            else if (!languageNames.isEmpty()) {
                item.languageName = languageNames.first();
                item.speller = Sonnet::Speller(item.languageName);
                if (item.speller.language() == item.languageName)
                    m_items.append(item);
            }
        }
    }

private:
    struct Item
    {
        QString languageName;
        QLocale::Language language;
        QChar::Script script;
        Sonnet::Speller speller;
    };
    QList<Item> m_items;
    QMap<int, QStringList> m_supportedLanguages;
};

Q_GLOBAL_STATIC(QThreadStorage<Spellers *>, ThreadSpellers)

QList<int> InitializeSpellCheckThread(const QList<int> &languageCodes)
{
    Sonnet::Loader::openLoader();
    Spellers *spellers = new Spellers(languageCodes);
    ThreadSpellers->setLocalData(spellers);
    return spellers->supportedLanguages();
}

SpellCheckServiceResult CheckSpellings(const SpellCheckServiceRequest &request)
{
    SpellCheckServiceResult result;
    result.timestamp = request.timestamp;
    result.text = request.text;

    if (request.text.isEmpty())
        return result; // Should never happen

    const Sonnet::TextBreaks::Positions wordPositions =
            Sonnet::TextBreaks::wordBreaks(request.text);
    if (wordPositions.isEmpty() || Sonnet::Loader::openLoader() == nullptr)
        return result;

    const Spellers *spellers = ::ThreadSpellers->localData();
    if (spellers == nullptr)
        return result;

    for (const Sonnet::TextBreaks::Position &wordPosition : wordPositions) {
        TextFragment fragment;
        if (spellers->checkSpelling(wordPosition, request, fragment)) {
            result.misspelledFragments.append(fragment);
        }
    }

    return result;
}

bool AddToDictionary(const QString &word)
{
    /**
     * It is assumed that word contains a single word. We won't bother checking for that.
     */
    EnglishLanguageSpeller speller;
    return speller.addToPersonal(word);
}

QStringList GetSpellingSuggestions(const QString &word)
{
    const Spellers *spellers = ::ThreadSpellers->localData();
    if (spellers == nullptr) {
        EnglishLanguageSpeller speller;
        return speller.suggest(word);
    }

    return spellers->getSuggestions(word);
}

class SpellCheckThreadPool : public QThreadPool
{
public:
    SpellCheckThreadPool()
    {
#ifdef Q_OS_MAC
        NSSpellCheckerClient::ensureSpellCheckerAvailability();
#endif
        const QList<int> supportedLanguageCodes =
                LanguageEngine::instance()->supportedLanguages()->languageCodes();

        setExpiryTimeout(-1);
        setMaxThreadCount(1);
        QFuture<QList<int>> future = QtConcurrent::run(this, [supportedLanguageCodes]() {
            return InitializeSpellCheckThread(supportedLanguageCodes);
        });
        future.waitForFinished();

        m_supportedLanguages = future.result();
    }

    bool isLanguageSupported(int language) const { return m_supportedLanguages.contains(language); }

private:
    QList<int> m_supportedLanguages;
};

Q_GLOBAL_STATIC(SpellCheckThreadPool, SpellCheckServiceThreadPool)

SpellCheckService::SpellCheckService(QObject *parent)
    : QObject(parent), m_textTracker(&m_textModifiable)
{
}

SpellCheckService::~SpellCheckService() { }

void SpellCheckService::setText(const QString &val)
{
    if (m_text == val)
        return;

    m_text = val;
    m_textModifiable.markAsModified();

    emit textChanged();

    this->doUpdate();
}

void SpellCheckService::setMethod(SpellCheckService::Method val)
{
    if (m_method == val)
        return;

    m_method = val;
    emit methodChanged();
}

void SpellCheckService::setAsynchronous(bool val)
{
    if (m_asynchronous == val)
        return;

    m_asynchronous = val;
    emit asynchronousChanged();
}

void SpellCheckService::scheduleUpdate()
{
    QFutureWatcherBase *watcher =
            this->findChild<QFutureWatcherBase *>(QString(), Qt::FindDirectChildrenOnly);
    if (watcher != nullptr) {
        connect(watcher, &QFutureWatcherBase::destroyed, this, &SpellCheckService::scheduleUpdate);
        return;
    }

    m_textModifiable.markAsModified();
    m_updateTimer.start(500, this);
}

void SpellCheckService::update()
{
    m_updateTimer.stop();

    if (!m_textTracker.isModified())
        return;

    emit started();

    this->setMisspelledFragments(QList<TextFragment>());

    QThreadPool *threadPool = SpellCheckServiceThreadPool();
    if (threadPool == nullptr || m_text.isEmpty()) {
        emit finished();
        return;
    }

    // Since the result travels from background thread to main thread
    static int serviceResultTypeId = qRegisterMetaType<SpellCheckServiceResult>();
    Q_UNUSED(serviceResultTypeId)

    // Since the result travels from background thread to main thread
    static int serviceRequestTypeId = qRegisterMetaType<SpellCheckServiceRequest>();
    Q_UNUSED(serviceRequestTypeId)

    SpellCheckServiceRequest request;
    request.text = m_text;
    request.timestamp = m_textModifiable.modificationTime();
    request.characterNames = ScriteDocument::instance()->structure()->characterNames();
    request.ignoreList = ScriteDocument::instance()->spellCheckIgnoreList();

    request.characterNames << QStringLiteral("Rajkumar");

    QFutureWatcher<SpellCheckServiceResult> *watcher =
            new QFutureWatcher<SpellCheckServiceResult>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(spellCheckComplete()), Qt::QueuedConnection);

    QFuture<SpellCheckServiceResult> future =
            QtConcurrent::run(threadPool, CheckSpellings, request);
    watcher->setFuture(future);
}

QStringList SpellCheckService::suggestions(const QString &word)
{
    QThreadPool *threadPool = SpellCheckServiceThreadPool();
    if (threadPool == nullptr || word.isEmpty())
        return QStringList();

    QFuture<QStringList> future = QtConcurrent::run(threadPool, GetSpellingSuggestions, word);
    future.waitForFinished();
    return future.result();
}

bool SpellCheckService::addToDictionary(const QString &word)
{
    QThreadPool *threadPool = SpellCheckServiceThreadPool();
    if (threadPool == nullptr || word.isEmpty())
        return false;

    QFuture<bool> future = QtConcurrent::run(threadPool, AddToDictionary, word);
    future.waitForFinished();
    return future.result();
}

bool SpellCheckService::canCheckLanguage(int language)
{
    SpellCheckThreadPool *threadPool = SpellCheckServiceThreadPool();
    if (threadPool == nullptr)
        return false;

    return threadPool->isLanguageSupported(language);
}

void SpellCheckService::classBegin() { }

void SpellCheckService::componentComplete()
{
    this->doUpdate();
}

void SpellCheckService::setMisspelledFragments(const QList<TextFragment> &val)
{
    if (m_misspelledFragments == val)
        return;

    m_misspelledFragments = val;

    QJsonArray json;
    for (const TextFragment &textFrag : qAsConst(m_misspelledFragments)) {
        QJsonObject item;
        item.insert("start", textFrag.start());
        item.insert("length", textFrag.length());
        item.insert("isValid", textFrag.isValid());
        item.insert("end", textFrag.end());
        item.insert("text", m_text.mid(textFrag.start(), textFrag.length()));
        json.append(item);
    }
    m_misspelledFragmentsJson = json;

    this->markAsModified();

    emit misspelledFragmentsChanged();
}

void SpellCheckService::doUpdate()
{
    if (m_method == Automatic) {
        if (m_asynchronous)
            this->scheduleUpdate();
        else
            this->update();
    }
}

void SpellCheckService::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimer.timerId())
        this->update();
}

void SpellCheckService::spellCheckComplete()
{
    QFutureWatcher<SpellCheckServiceResult> *watcher =
            dynamic_cast<QFutureWatcher<SpellCheckServiceResult> *>(this->sender());
    if (watcher == nullptr)
        return;

    GarbageCollector::instance()->add(watcher);

    const SpellCheckServiceResult result = watcher->result();
    if (m_textModifiable.isModified(result.timestamp))
        return;

    this->acceptResult(result);
}

void SpellCheckService::acceptResult(const SpellCheckServiceResult &result)
{
    this->setMisspelledFragments(result.misspelledFragments);
    emit finished();
}
