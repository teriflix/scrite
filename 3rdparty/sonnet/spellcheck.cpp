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

#include "spellcheck.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"

#include <QFuture>
#include <QJsonObject>
#include <QTimerEvent>
#include <QFutureWatcher>
#include <QThreadStorage>
#include <QtConcurrentRun>
#include <QRandomGenerator>
#include <QCoreApplication>

#include "3rdparty/sonnet/sonnet/src/core/speller.h"
#include "3rdparty/sonnet/sonnet/src/core/loader_p.h"
#include "3rdparty/sonnet/sonnet/src/core/textbreaks_p.h"
#include "3rdparty/sonnet/sonnet/src/core/guesslanguage.h"

class SpellCheckResult
{
    char padding[4];

public:
    SpellCheckResult() { padding[0] = 0; }

    int timestamp = -1;
    QString text;
    QList<TextFragment> misspelledFragments;
};

class EnglishLanguageSpeller : public Sonnet::Speller
{
public:
    static EnglishLanguageSpeller &instance();

    EnglishLanguageSpeller() : Sonnet::Speller("en") { }
    ~EnglishLanguageSpeller() { }
};

EnglishLanguageSpeller &EnglishLanguageSpeller::instance()
{
    Sonnet::Loader::openLoader();
    static EnglishLanguageSpeller theInstance;
    return theInstance;
}

SpellCheckResult CheckSpellings(const QString &text, int timestamp, const QStringList &characterNames)
{
    SpellCheckResult result;
    result.timestamp = timestamp;
    result.text = text;

    if(text.isEmpty())
        return result; // Should never happen

    /*
     * Much of the code here is inspired from Sonnet::Highlighter::highlightBlock()
     * Obviously their implementation is 'smarter' because their implementation works
     * directly on the view, our's works on the model.
     *
     * Sonnet::Highlighter is intended to be used on a QTextEdit or QPlainTextEdit
     * both of which are widgets (which means views). The highlighter directly works
     * on QTextBlocks within a QTextDocument, which is great.
     *
     * We cannot do that in Scrite because we open a TextArea for each scene separately.
     * And TextAreas get created and destroyed frequently depending on the scroll position
     * of contentView in ScreenplayEditor. This means that each time a scene scrolls into
     * visibility, it will have to reinitialize the entire underlying data structure related
     * spell-check. That is an expensive thing to do.
     *
     * In Scrite, we simply associate a SpellCheck instance with each SceneElement and reuse
     * its results across all views hooked to the Scene. Additionally, we use SpellCheck on
     * Note and StructureElement also. This fits into the whole model-view thinking that
     * QML apps are required to leverage.
     *
     * There is obviously room for improvement in this implementation. We could cache
     * spell-check done in the previous round and only check for delta changes in here.
     * But for now, we simply take a brute-force approach. But this function could be a
     * good place for us to accept community contribution.
     */

    EnglishLanguageSpeller &speller = EnglishLanguageSpeller::instance();
    const Sonnet::TextBreaks::Positions wordPositions = Sonnet::TextBreaks::wordBreaks(text);
    Q_FOREACH(Sonnet::TextBreaks::Position wordPosition, wordPositions)
    {
        const QString word = text.mid(wordPosition.start, wordPosition.length);
        const bool misspelled = speller.isMisspelled(word);
        if(misspelled)
        {
            if(characterNames.contains(word.toUpper()))
                continue;

            if(word.endsWith("\'s", Qt::CaseInsensitive))
            {
                if(characterNames.contains(word.leftRef(word.length()-2)))
                    continue;
            }

            TextFragment fragment(wordPosition.start, wordPosition.length);
            if(fragment.isValid())
                result.misspelledFragments << fragment;
        }
    }

    return result;
}

SpellCheck::SpellCheck(QObject *parent)
    : QObject(parent),
      m_textTracker(&m_textModifiable)
{
    EnglishLanguageSpeller::instance();
}

SpellCheck::~SpellCheck()
{

}

void SpellCheck::setText(const QString &val)
{
    if(m_text == val)
        return;

    m_text = val;
    m_textModifiable.markAsModified();

    emit textChanged();

    this->doUpdate();
}

void SpellCheck::setMethod(SpellCheck::Method val)
{
    if(m_method == val)
        return;

    m_method = val;
    emit methodChanged();
}

void SpellCheck::setAsynchronous(bool val)
{
    if(m_asynchronous == val)
        return;

    m_asynchronous = val;
    emit asynchronousChanged();
}

void SpellCheck::setThreaded(bool val)
{
    if(m_threaded == val)
        return;

    m_threaded = val;
    emit threadedChanged();
}

void SpellCheck::scheduleUpdate()
{
    m_updateTimer.start(500, this);
}

void SpellCheck::update()
{
    if(!m_textTracker.isModified())
        return;

    this->setMisspelledFragments(QList<TextFragment>());

    if(m_text.isEmpty())
        return;

    emit started();

    const QStringList characterNames = ScriteDocument::instance()->structure()->characterNames();
    const int timestamp = m_textModifiable.modificationTime();

    if(m_threaded)
    {
        QFutureWatcher<SpellCheckResult> *watcher = new QFutureWatcher<SpellCheckResult>(this);
        connect(watcher, SIGNAL(finished()), this, SLOT(spellCheckThreadComplete()), Qt::QueuedConnection);

#ifdef Q_OS_MAC
        /**
         * NSSpellChecker that we use on macOS provides us only a shared instance of the actual spell-checker.
         * Using that shared instance from multiple threads is really of no use, because macOS will end up using
         * mutexes to serialize access to it. We are better off queuing all the CheckSpellings() calls to a
         * single thread.
         */
        static QThreadPool threadPool;
        threadPool.setMaxThreadCount(1);
        QFuture<SpellCheckResult> future = QtConcurrent::run(&threadPool, CheckSpellings, m_text, timestamp, characterNames);
#else
        QFuture<SpellCheckResult> future = QtConcurrent::run(CheckSpellings, m_text, timestamp, characterNames);
#endif
        watcher->setFuture(future);
    }
    else
    {
        const SpellCheckResult result = CheckSpellings(m_text, timestamp, characterNames);
        this->acceptResult(result);
    }
}

void SpellCheck::classBegin()
{

}

void SpellCheck::componentComplete()
{
    this->doUpdate();
}

void SpellCheck::setMisspelledFragments(const QList<TextFragment> &val)
{
    if(m_misspelledFragments == val)
        return;

    m_misspelledFragments = val;

    QJsonArray json;
    Q_FOREACH(TextFragment textFrag, m_misspelledFragments)
    {
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

void SpellCheck::doUpdate()
{
    if(m_method == Automatic)
    {
        if(m_asynchronous)
            this->scheduleUpdate();
        else
            this->update();
    }
}

void SpellCheck::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_updateTimer.timerId())
    {
        m_updateTimer.stop();
        this->update();
    }
}

void SpellCheck::spellCheckThreadComplete()
{
    QFutureWatcher<SpellCheckResult> *watcher = dynamic_cast< QFutureWatcher<SpellCheckResult> *>(this->sender());
    if(watcher == nullptr)
        return;

    const SpellCheckResult result = watcher->result();
    if(m_textModifiable.isModified(result.timestamp))
        return;

    this->acceptResult(result);

    GarbageCollector::instance()->add(watcher);
}

void SpellCheck::acceptResult(const SpellCheckResult &result)
{
    this->setMisspelledFragments(result.misspelledFragments);
    emit finished();
}
