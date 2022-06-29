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

#include "hourglass.h"
#include "searchengine.h"

#include <QSet>
#include <QJsonObject>
#include <QTextCursor>
#include <QTimerEvent>

SearchAgent::SearchAgent(QObject *parent)
    : QObject(parent), m_engine(this, "engine"), m_textDocument(this, "textDocument")
{
}

SearchAgent::~SearchAgent()
{
    emit aboutToDelete(this);

    if (m_engine != nullptr)
        m_engine->removeSearchAgent(this);
    m_engine = nullptr;
}

SearchAgent *SearchAgent::qmlAttachedProperties(QObject *object)
{
    return new SearchAgent(object);
}

void SearchAgent::setEngine(SearchEngine *val)
{
    if (m_engine == val)
        return;

    if (m_engine != nullptr)
        m_engine->removeSearchAgent(this);

    m_engine = val;

    if (m_engine != nullptr)
        m_engine->addSearchAgent(this);

    emit engineChanged();
}

void SearchAgent::resetEngine()
{
    m_engine = nullptr;

    this->setSearchResultCount(0);
    this->setCurrentSearchResultIndex(-1);
    emit clearSearchRequest();
}

void SearchAgent::setSequenceNumber(int val)
{
    if (m_sequenceNumber == val)
        return;

    m_sequenceNumber = val;
    if (m_engine != nullptr)
        m_engine->sortSearchAgentsLater();

    emit sequenceNumberChanged();
}

void SearchAgent::setSearchResultCount(int val)
{
    if (m_searchResultCount == val)
        return;

    m_searchResultCount = val;
    emit searchResultCountChanged();
}

void SearchAgent::setCurrentSearchResultIndex(int val)
{
    if (m_searchResultCount == 0) {
        if (m_currentSearchResultIndex != -1) {
            emit clearHighlight();

            m_currentSearchResultIndex = -1;
            emit currentSearchResultIndexChanged();
        }

        return;
    }

    val = qBound(-1, val, m_searchResultCount - 1);
    if (m_currentSearchResultIndex == val)
        return;

    emit clearHighlight();

    m_currentSearchResultIndex = val;
    emit currentSearchResultIndexChanged();

    if (val < 0 || val >= m_textDocumentSearchResults.size())
        return;

    const QPair<int, int> result = m_textDocumentSearchResults.at(val);
    emit highlightText(result.first, result.second);
}

void SearchAgent::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    m_textDocument = val;

    if (m_textDocument == nullptr) {
        disconnect(this, &SearchAgent::searchRequest, this, &SearchAgent::onSearchRequest);
        disconnect(this, &SearchAgent::clearSearchRequest, this,
                   &SearchAgent::onClearSearchRequest);
    } else {
        connect(this, &SearchAgent::searchRequest, this, &SearchAgent::onSearchRequest);
        connect(this, &SearchAgent::clearSearchRequest, this, &SearchAgent::onClearSearchRequest);
    }

    emit textDocumentChanged();
}

void SearchAgent::resetTextDocument()
{
    this->setTextDocument(nullptr);
}

QJsonArray SearchAgent::indexesOf(const QString &of, const QString &in) const
{
    SearchEngine::SearchFlags flags;
    if (m_engine != nullptr)
        flags = m_engine->searchFlags();
    return SearchEngine::indexesOf(of, in, int(flags));
}

QString SearchAgent::createMarkupText(const QString &text, int from, int to, const QColor &bg,
                                      const QColor &fg) const
{
    return SearchEngine::createMarkupText(text, from, to, QBrush(bg), QBrush(fg));
}

void SearchAgent::onSearchRequest(const QString &string)
{
    if (m_textDocument == nullptr)
        return;

    m_textDocumentSearchResults.clear();
    this->setSearchResultCount(0);
    this->setCurrentSearchResultIndex(-1);

    if (string.isEmpty())
        return;

    QTextDocument *document = m_textDocument->textDocument();
    QTextDocument::FindFlags flags;
    if (m_engine != nullptr)
        flags &= int(m_engine->searchFlags());

    QTextCursor cursor(document);
    while (1) {
        cursor = document->find(string, cursor, flags);
        if (cursor.isNull())
            break;

        m_textDocumentSearchResults
                << qMakePair<int, int>(cursor.selectionStart(), cursor.selectionEnd());
        cursor.setPosition(cursor.selectionEnd());
    }

    this->setSearchResultCount(m_textDocumentSearchResults.size());
}

void SearchAgent::onClearSearchRequest()
{
    m_textDocumentSearchResults.clear();
}

///////////////////////////////////////////////////////////////////////////////

SearchEngine::SearchEngine(QObject *parent)
    : QObject(parent),
      m_searchTimer("SearchEngine.m_searchTimer"),
      m_searchAgentSortTimer("SearchEngine.m_searchAgentSortTimer")
{
}

SearchEngine::~SearchEngine() { }

QQmlListProperty<SearchAgent> SearchEngine::searchAgents()
{
    return QQmlListProperty<SearchAgent>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &SearchEngine::staticSearchAgentCount, &SearchEngine::staticSearchAgentAt);
}

SearchAgent *SearchEngine::searchAgentAt(int index) const
{
    return index < 0 || index >= m_searchAgents.size() ? nullptr : m_searchAgents.at(index);
}

void SearchEngine::clearSearchAgents()
{
    while (m_searchAgents.size())
        this->removeSearchAgent(m_searchAgents.first());
}

void SearchEngine::setSearchFlags(SearchFlags val)
{
    if (m_searchFlags == val)
        return;

    m_searchFlags = val;
    emit searchFlagsChanged();
}

void SearchEngine::setSearchString(const QString &val)
{
    if (m_searchString == val) {
        if (m_searchResults.isEmpty())
            this->doSearchLater();
        return;
    }

    m_searchString = val;
    emit searchStringChanged();

    this->doSearchLater();
}

void SearchEngine::replace(const QString &string)
{
    HourGlass hourGlass;

    if (m_currentSearchResultIndex >= 0 && m_currentSearchResultIndex < m_searchResults.size()) {
        const QPair<SearchAgent *, int> result = m_searchResults.at(m_currentSearchResultIndex);
        if (result.first != nullptr)
            result.first->replaceCurrent(string);
    }
}

void SearchEngine::replaceAll(const QString &string)
{
    HourGlass hourGlass;

    if (m_searchResults.isEmpty())
        return;

    this->setCurrentSearchResultIndex(-1);

    SearchAgent *agent = nullptr;
    while (!m_searchResults.isEmpty()) {
        const QPair<SearchAgent *, int> result = m_searchResults.takeFirst();
        if (result.first != agent) {
            agent = result.first;
            agent->replaceAll(string);
            agent->clearHighlight();
            agent->clearSearchRequest();
            agent->setSearchResultCount(0);
            agent->setCurrentSearchResultIndex(-1);
        }
    }

    emit searchResultCountChanged();
}

void SearchEngine::clearSearch()
{
    this->setSearchString(QString());
}

void SearchEngine::nextSearchResult()
{
    this->setCurrentSearchResultIndex(m_currentSearchResultIndex + 1);
}

void SearchEngine::previousSearchResult()
{
    this->setCurrentSearchResultIndex(m_currentSearchResultIndex - 1);
}

void SearchEngine::cycleSearchResult()
{
    if (m_searchResults.isEmpty())
        this->setCurrentSearchResultIndex(-1);
    else {
        const int newIndex = (m_currentSearchResultIndex + 1) % m_searchResults.size();
        this->setCurrentSearchResultIndex(newIndex);
    }
}

QJsonArray SearchEngine::indexesOf(const QString &of, const QString &in, int givenFlags)
{
    SearchEngine::SearchFlags flags(givenFlags);
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;

    if (flags.testFlag(SearchEngine::SearchCaseSensitively))
        cs = Qt::CaseSensitive;

    auto createResultItem = [](int from, int to) {
        QJsonObject item;
        item.insert("from", from);
        item.insert("to", to);
        return item;
    };

    QJsonArray ret;
    int from = 0;
    while (1) {
        int pos = in.indexOf(of, from, cs);
        if (pos < 0)
            break;

        if (flags.testFlag(SearchEngine::SearchWholeWords)) {
            if (pos + of.length() >= in.length() || in.at(pos + of.length()).isSpace())
                ret.append(createResultItem(pos, pos + of.length() - 1));
        } else
            ret.append(createResultItem(pos, pos + of.length() - 1));

        from = pos + of.length();
    }

    return ret;
}

QString SearchEngine::createMarkupText(const QString &text, int from, int to, const QBrush &bg,
                                       const QBrush &fg)
{
    QString ret = text;
    ret.insert(to + 1, "</span>");
    ret.insert(from,
               QString("<span style=\"background-color: %1; color: %2;\">")
                       .arg(bg.color().name())
                       .arg(fg.color().name()));
    return ret;
}

void SearchEngine::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_searchAgentSortTimer.timerId()) {
        m_searchAgentSortTimer.stop();
        this->sortSearchAgents();
    }

    if (event->timerId() == m_searchTimer.timerId()) {
        m_searchTimer.stop();
        this->doSearch();
    }
}

void SearchEngine::addSearchAgent(SearchAgent *ptr)
{
    if (ptr == nullptr || m_searchAgents.indexOf(ptr) >= 0)
        return;

    m_searchAgents.append(ptr);
    this->sortSearchAgentsLater();

    emit searchAgentCountChanged();
    emit searchAgentsChanged();
}

void SearchEngine::removeSearchAgent(SearchAgent *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_searchAgents.indexOf(ptr);
    if (index < 0)
        return;

    m_searchAgents.removeAt(index);

    const int oldSearchResultCount = m_searchResults.size();
    for (int i = m_searchResults.size() - 1; i >= 0; i--) {
        const QPair<SearchAgent *, int> result = m_searchResults.at(i);
        if (result.first == ptr)
            m_searchResults.removeAt(i);
    }
    const int newSearchResultCount = m_searchResults.size();

    emit searchAgentCountChanged();
    emit searchAgentsChanged();

    if (newSearchResultCount != oldSearchResultCount) {
        emit searchResultCountChanged();
        this->setCurrentSearchResultIndex(0);
    }
}

bool searchAgentLessThan(const SearchAgent *a1, const SearchAgent *a2)
{
    return a1->sequenceNumber() < a2->sequenceNumber();
}

void SearchEngine::sortSearchAgents()
{
    std::sort(m_searchAgents.begin(), m_searchAgents.end(), searchAgentLessThan);
    emit searchAgentsChanged();
}

void SearchEngine::sortSearchAgentsLater()
{
    m_searchAgentSortTimer.start(0, this);
}

SearchAgent *SearchEngine::staticSearchAgentAt(QQmlListProperty<SearchAgent> *list, int index)
{
    return reinterpret_cast<SearchEngine *>(list->data)->searchAgentAt(index);
}

int SearchEngine::staticSearchAgentCount(QQmlListProperty<SearchAgent> *list)
{
    return reinterpret_cast<SearchEngine *>(list->data)->searchAgentCount();
}

void SearchEngine::doSearch()
{
    HourGlass hourGlass;

    if (!m_searchResults.isEmpty()) {
        SearchAgent *agent = nullptr;
        for (const QPair<SearchAgent *, int> &result : qAsConst(m_searchResults)) {
            if (agent == result.first)
                continue;

            agent = result.first;
            agent->clearHighlight();
            agent->clearSearchRequest();
            agent->setSearchResultCount(0);
            agent->setCurrentSearchResultIndex(-1);
        }

        m_searchResults.clear();
        emit searchResultCountChanged();
    }

    this->setCurrentSearchResultIndex(-1);

    if (m_searchAgentSortTimer.isActive()) {
        this->sortSearchAgents();
        m_searchAgentSortTimer.stop();
    }

    if (m_searchString.isEmpty())
        return;

    for (SearchAgent *agent : qAsConst(m_searchAgents)) {
        // Ask the agent to perform search
        agent->searchRequest(m_searchString);

        // Collect search results
        const int nrResults = agent->searchResultCount();
        for (int i = 0; i < nrResults; i++) {
            QPair<SearchAgent *, int> result(agent, i);
            m_searchResults << result;
        }

        agent->setCurrentSearchResultIndex(-1);
    }

    emit searchResultCountChanged();

    if (!m_searchResults.isEmpty())
        this->setCurrentSearchResultIndex(0);
}

void SearchEngine::doSearchLater()
{
    m_searchTimer.start(0, this);
}

void SearchEngine::setCurrentSearchResultIndex(int val)
{
    if (m_searchResults.isEmpty()) {
        if (m_currentSearchResultIndex != -1) {
            m_currentSearchResultIndex = -1;
            emit currentSearchResultIndexChanged();
        }

        return;
    }

    val = qBound(0, val, m_searchResults.size() - 1);
    if (m_currentSearchResultIndex == val)
        return;

    const QPair<SearchAgent *, int> oldResult =
            (m_currentSearchResultIndex >= 0 && m_currentSearchResultIndex < m_searchResults.size())
            ? m_searchResults.at(m_currentSearchResultIndex)
            : qMakePair<SearchAgent *, int>(nullptr, -1);
    m_currentSearchResultIndex = val;
    const QPair<SearchAgent *, int> newResult = m_searchResults.at(m_currentSearchResultIndex);

    if (oldResult.first != nullptr && oldResult.first != newResult.first)
        oldResult.first->setCurrentSearchResultIndex(-1);
    newResult.first->setCurrentSearchResultIndex(newResult.second);

    emit currentSearchResultIndexChanged();
}

///////////////////////////////////////////////////////////////////////////////

TextDocumentSearch::TextDocumentSearch(QObject *parent)
    : QObject(parent), m_textDocument(this, "textDocument")
{
}

TextDocumentSearch::~TextDocumentSearch() { }

void TextDocumentSearch::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    m_textDocument = val;

    if (m_textDocument != nullptr) {
        if (!m_searchString.isEmpty())
            this->doSearch(m_searchString);
    }

    emit textDocumentChanged();
}

void TextDocumentSearch::resetTextDocument()
{
    this->setTextDocument(nullptr);
}

void TextDocumentSearch::setSearchString(const QString &val)
{
    if (m_searchString == val)
        return;

    this->doSearch(val);

    // This signal will be emitted by doSearch()
    // emit searchStringChanged();
}

void TextDocumentSearch::setCurrentResultIndex(int val)
{
    if (m_searchResults.isEmpty()) {
        if (m_currentResultIndex != -1) {
            emit clearHighlight();

            m_currentResultIndex = -1;
            emit currentResultIndexChanged();
        }

        return;
    }

    val = qBound(-1, val, m_searchResults.size() - 1);
    if (m_currentResultIndex == val)
        return;

    emit clearHighlight();

    m_currentResultIndex = val;
    emit currentResultIndexChanged();

    if (val < 0 || val >= m_searchResults.size())
        return;

    const QPair<int, int> result = m_searchResults.at(val);
    emit highlightText(result.first, result.second);
}

void TextDocumentSearch::setSearchFlags(SearchEngine::SearchFlags val)
{
    if (m_searchFlags == val)
        return;

    m_searchFlags = val;
    emit searchFlagsChanged();
}

void TextDocumentSearch::clearSearch()
{
    this->setCurrentResultIndex(-1);

    if (!m_searchResults.isEmpty()) {
        m_searchResults.clear();
        emit searchResultCountChanged();
    }

    if (!m_searchString.isEmpty()) {
        m_searchString.clear();
        emit searchStringChanged();
    }
}

void TextDocumentSearch::nextSearchResult()
{
    this->setCurrentResultIndex(m_currentResultIndex + 1);
}

void TextDocumentSearch::previousSearchResult()
{
    this->setCurrentResultIndex(m_currentResultIndex - 1);
}

void TextDocumentSearch::cycleSearchResult()
{
    if (!m_searchResults.isEmpty())
        this->setCurrentResultIndex((m_currentResultIndex + 1) % m_searchResults.size());
    else
        this->setCurrentResultIndex(-1);
}

void TextDocumentSearch::replace(const QString &replacementText)
{
    if (m_currentResultIndex < 0 || m_currentResultIndex >= m_searchResults.size())
        return;

    if (m_textDocument == nullptr)
        return;

    QTextDocument *document = m_textDocument->textDocument();
    if (document == nullptr)
        return;

    const QPair<int, int> result = m_searchResults.at(m_currentResultIndex);

    QTextCursor cursor(document);
    cursor.setPosition(result.first);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                        result.second - result.first);
    if (cursor.selectedText() != replacementText) {
        const int diff = replacementText.length() - cursor.selectedText().length();
        cursor.insertText(replacementText);

        if (diff != 0 && m_currentResultIndex < m_searchResults.size() - 1) {
            for (int i = m_currentResultIndex + 1; i < m_searchResults.size(); i++) {
                m_searchResults[i].first += diff;
                m_searchResults[i].second += diff;
            }
        }
    }
}

void TextDocumentSearch::replaceAll(const QString &replacementText)
{
    if (m_textDocument == nullptr)
        return;

    QTextDocument *document = m_textDocument->textDocument();
    if (document == nullptr)
        return;

    QTextCursor cursor(document);
    for (int i = m_searchResults.size() - 1; i >= 0; i--) {
        const QPair<int, int> result = m_searchResults.at(i);
        cursor.setPosition(result.first);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                            result.second - result.first);
        if (cursor.selectedText() != replacementText)
            cursor.insertText(replacementText);
    }
}

void TextDocumentSearch::doSearch(const QString &string)
{
    if (m_textDocument == nullptr)
        return;

    this->clearSearch();
    if (string.isEmpty())
        return;

    QTextDocument *document = m_textDocument->textDocument();
    QTextDocument::FindFlags flags;
    flags &= int(m_searchFlags);

    QTextCursor cursor(document);
    while (1) {
        cursor = document->find(string, cursor, flags);
        if (cursor.isNull())
            break;

        m_searchResults << qMakePair<int, int>(cursor.selectionStart(), cursor.selectionEnd());
        cursor.setPosition(cursor.selectionEnd());
    }

    m_searchString = string;
    emit searchStringChanged();

    if (!m_searchResults.isEmpty())
        emit searchResultCountChanged();
}
