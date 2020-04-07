/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "searchengine.h"

#include <QJsonObject>
#include <QSet>
#include <QTextCursor>
#include <QTimerEvent>

SearchAgent::SearchAgent(QObject *parent)
            :QObject(parent),
              m_sequenceNumber(-1),
              m_engine(nullptr),
              m_searchResultCount(0),
              m_currentSearchResultIndex(-1),
              m_textDocument(nullptr)
{

}

SearchAgent::~SearchAgent()
{
    emit aboutToDelete(this);

    if(m_engine != nullptr)
        m_engine->removeSearchAgent(this);
    m_engine = nullptr;
}

SearchAgent *SearchAgent::qmlAttachedProperties(QObject *object)
{
    return new SearchAgent(object);
}

void SearchAgent::setEngine(SearchEngine *val)
{
    if(m_engine == val)
        return;

    if(m_engine != nullptr)
    {
        m_engine->removeSearchAgent(this);
        disconnect(m_engine, &SearchEngine::destroyed, this, &SearchAgent::onSearchEngineDestroyed);
    }

    m_engine = val;

    if(m_engine != nullptr)
    {
        m_engine->addSearchAgent(this);
        connect(m_engine, &SearchEngine::destroyed, this, &SearchAgent::onSearchEngineDestroyed);
    }

    emit engineChanged();
}

void SearchAgent::setSequenceNumber(int val)
{
    if(m_sequenceNumber == val)
        return;

    m_sequenceNumber = val;
    if(m_engine != nullptr)
        m_engine->sortSearchAgentsLater();

    emit sequenceNumberChanged();
}

void SearchAgent::setSearchResultCount(int val)
{
    if(m_searchResultCount == val)
        return;

    m_searchResultCount = val;
    emit searchResultCountChanged();
}

void SearchAgent::setCurrentSearchResultIndex(int val)
{
    if(m_searchResultCount == 0)
    {
        if(m_currentSearchResultIndex != -1)
        {
            emit clearHighlight();

            m_currentSearchResultIndex = -1;
            emit currentSearchResultIndexChanged();

        }

        return;
    }

    val = qBound(-1, val, m_searchResultCount-1);
    if(m_currentSearchResultIndex == val)
        return;

    emit clearHighlight();

    m_currentSearchResultIndex = val;
    emit currentSearchResultIndexChanged();

    if(val < 0 || val >= m_textDocumentSearchResults.size())
        return;

    const QPair<int,int> result = m_textDocumentSearchResults.at(val);
    emit highlightText(result.first, result.second);
}

void SearchAgent::setTextDocument(QQuickTextDocument *val)
{
    if(m_textDocument == val)
        return;

    if(m_textDocument != nullptr)
        disconnect(m_textDocument, &QQuickTextDocument::destroyed, this, &SearchAgent::onTextDocumentDestroyed);

    m_textDocument = val;
    if(m_textDocument == nullptr)
    {
        disconnect(this, &SearchAgent::searchRequest, this, &SearchAgent::onSearchRequest);
        disconnect(this, &SearchAgent::clearSearchRequest, this, &SearchAgent::onClearSearchRequest);
    }
    else
    {
        connect(m_textDocument, &QQuickTextDocument::destroyed, this, &SearchAgent::onTextDocumentDestroyed);
        connect(this, &SearchAgent::searchRequest, this, &SearchAgent::onSearchRequest);
        connect(this, &SearchAgent::clearSearchRequest, this, &SearchAgent::onClearSearchRequest);
    }

    emit textDocumentChanged();
}

QJsonArray SearchAgent::indexesOf(const QString &of, const QString &in) const
{
#if 1
    SearchEngine::SearchFlags flags;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;

    if(m_engine != nullptr)
        flags = m_engine->searchFlags();

    if(flags.testFlag(SearchEngine::SearchCaseSensitively))
        cs = Qt::CaseSensitive;

    auto createResultItem = [](int from, int to) {
        QJsonObject item;
        item.insert("from", from);
        item.insert("to", to);
        return item;
    };

    QJsonArray ret;
    int from = 0;
    while(1)
    {
        int pos = in.indexOf(of, from, cs);
        if(pos < 0)
            break;

        if(flags.testFlag(SearchEngine::SearchWholeWords))
        {
            if(pos + of.length() >= in.length() || in.at(pos+of.length()).isSpace())
                ret.append( createResultItem(pos,pos+of.length()-1) );
        }
        else
            ret.append( createResultItem(pos,pos+of.length()-1) );

        from = pos + of.length();
    }

    return ret;
#else
    QTextDocument::FindFlags flags;
    if(m_engine != nullptr)
        flags &= int(m_engine->searchFlags());

    QTextDocument document;
    document.setPlainText(in);

    QJsonArray array;

    QTextCursor cursor(&document);
    while(1)
    {
        cursor = document.find(of, cursor, flags);
        if(cursor.isNull())
            break;

        QJsonObject item;
        item.insert("start", cursor.selectionStart());
        item.insert("end", cursor.selectionEnd());
        array.append(item);

        cursor.setPosition(cursor.selectionEnd());
    }

    return array;
#endif
}

QString SearchAgent::createMarkupText(const QString &text, int from, int to, const QColor &bg, const QColor &fg) const
{
    QString ret = text;
    ret.insert(to+1, "</span>");
    ret.insert(from, QString("<span style=\"background-color: %1; color: %2;\">").arg(bg.name()).arg(fg.name()));
    return ret;
}

void SearchAgent::onTextDocumentDestroyed()
{
    m_textDocument = nullptr;
    disconnect(this, &SearchAgent::searchRequest, this, &SearchAgent::onSearchRequest);
    disconnect(this, &SearchAgent::clearSearchRequest, this, &SearchAgent::onClearSearchRequest);

    this->setSearchResultCount(0);
    this->setCurrentSearchResultIndex(-1);
    emit clearSearchRequest();
}

void SearchAgent::onSearchEngineDestroyed()
{
    m_engine = nullptr;

    this->setSearchResultCount(0);
    this->setCurrentSearchResultIndex(-1);
    emit clearSearchRequest();
}

void SearchAgent::onSearchRequest(const QString &string)
{
    if(m_textDocument == nullptr)
        return;

    m_textDocumentSearchResults.clear();
    this->setSearchResultCount(0);
    this->setCurrentSearchResultIndex(-1);

    if(string.isEmpty())
        return;

    QTextDocument *document = m_textDocument->textDocument();
    QTextDocument::FindFlags flags;
    if(m_engine != nullptr)
        flags &= int(m_engine->searchFlags());

    QTextCursor cursor(document);
    while(1)
    {
        cursor = document->find(string, cursor, flags);
        if(cursor.isNull())
            break;

        m_textDocumentSearchResults << qMakePair<int,int>(cursor.selectionStart(),cursor.selectionEnd());
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
    :QObject(parent),
      m_errorReport(new ErrorReport(this)),
              m_progressReport(new ProgressReport(this))
{

}

SearchEngine::~SearchEngine()
{

}

QQmlListProperty<SearchAgent> SearchEngine::searchAgents()
{
    return QQmlListProperty<SearchAgent>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &SearchEngine::staticSearchAgentCount,
                &SearchEngine::staticSearchAgentAt);
}

SearchAgent *SearchEngine::searchAgentAt(int index) const
{
    return index < 0 || index >= m_searchAgents.size() ? nullptr : m_searchAgents.at(index);
}

void SearchEngine::clearSearchAgents()
{
    while(m_searchAgents.size())
        this->removeSearchAgent(m_searchAgents.first());
}

void SearchEngine::setSearchFlags(SearchFlags val)
{
    if(m_searchFlags == val)
        return;

    m_searchFlags = val;
    emit searchFlagsChanged();
}

void SearchEngine::setSearchString(const QString &val)
{
    if(m_searchString == val)
        return;

    m_searchString = val;
    emit searchStringChanged();

    this->doSearchLater();
}

void SearchEngine::clearSearch()
{
    this->setSearchString(QString());
}

void SearchEngine::nextSearchResult()
{
    this->setCurrentSearchResultIndex(m_currentSearchResultIndex+1);
}

void SearchEngine::previousSearchResult()
{
    this->setCurrentSearchResultIndex(m_currentSearchResultIndex-1);
}

void SearchEngine::cycleSearchResult()
{
    if(!m_searchResults.isEmpty())
        this->setCurrentSearchResultIndex( (m_currentSearchResultIndex+1)%m_searchResults.size() );
    else
        this->setCurrentSearchResultIndex(-1);
}

void SearchEngine::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_searchAgentSortTimer.timerId())
    {
        this->sortSearchAgents();
        m_searchAgentSortTimer.stop();
    }

    if(event->timerId() == m_searchTimer.timerId())
    {
        this->doSearch();
        m_searchTimer.stop();
    }
}

void SearchEngine::addSearchAgent(SearchAgent *ptr)
{
    if(ptr == nullptr || m_searchAgents.indexOf(ptr) >= 0)
        return;

    m_searchAgents.append(ptr);
    this->sortSearchAgentsLater();

    emit searchAgentCountChanged();
    emit searchAgentsChanged();
}

void SearchEngine::removeSearchAgent(SearchAgent *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_searchAgents.indexOf(ptr);
    if(index < 0)
        return;

    m_searchAgents.removeAt(index);

    const int oldSearchResultCount = m_searchResults.size();
    for(int i=m_searchResults.size()-1; i>=0 ;i--)
    {
        const QPair<SearchAgent*,int> result = m_searchResults.at(i);
        if(result.first == ptr)
            m_searchResults.removeAt(i);
    }
    const int newSearchResultCount = m_searchResults.size();

    emit searchAgentCountChanged();
    emit searchAgentsChanged();

    if(newSearchResultCount != oldSearchResultCount)
    {
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
    return reinterpret_cast< SearchEngine* >(list->data)->searchAgentAt(index);
}

int SearchEngine::staticSearchAgentCount(QQmlListProperty<SearchAgent> *list)
{
    return reinterpret_cast< SearchEngine* >(list->data)->searchAgentCount();
}

void SearchEngine::doSearch()
{
    this->setCurrentSearchResultIndex(-1);

    if(!m_searchResults.isEmpty())
    {
        QSet<SearchAgent*> agents;
        QPair<SearchAgent*,int> result;
        Q_FOREACH(result, m_searchResults)
            agents += result.first;
        Q_FOREACH(SearchAgent *agent, agents)
            agent->clearSearchRequest();

        m_searchResults.clear();
        emit searchResultCountChanged();
    }

    if(m_searchAgentSortTimer.isActive())
    {
        this->sortSearchAgents();
        m_searchAgentSortTimer.stop();
    }

    if(m_searchString.isEmpty())
        return;

    Q_FOREACH(SearchAgent *agent, m_searchAgents)
    {
        // Ask the agent to perform search
        agent->searchRequest(m_searchString);

        // Collect search results
        const int nrResults = agent->searchResultCount();
        for(int i=0; i<nrResults; i++)
        {
            QPair<SearchAgent*,int> result(agent, i);
            m_searchResults << result;
        }

        agent->setCurrentSearchResultIndex(-1);
    }

    emit searchResultCountChanged();

    if(!m_searchResults.isEmpty())
        this->setCurrentSearchResultIndex(0);
}

void SearchEngine::doSearchLater()
{
    m_searchTimer.start(0, this);
}

void SearchEngine::setCurrentSearchResultIndex(int val)
{
    if(m_searchResults.isEmpty())
    {
        if(m_currentSearchResultIndex != -1)
        {
            m_currentSearchResultIndex = -1;
            emit currentSearchResultIndexChanged();
        }

        return;
    }

    val = qBound(0, val, m_searchResults.size()-1);
    if(m_currentSearchResultIndex == val)
        return;

    const QPair<SearchAgent*,int> oldResult = (m_currentSearchResultIndex >= 0 && m_currentSearchResultIndex < m_searchResults.size()) ? m_searchResults.at(m_currentSearchResultIndex) : qMakePair<SearchAgent*,int>(nullptr,-1);
    m_currentSearchResultIndex = val;
    const QPair<SearchAgent*,int> newResult = m_searchResults.at(m_currentSearchResultIndex);

    if(oldResult.first != nullptr && oldResult.first != newResult.first)
        oldResult.first->setCurrentSearchResultIndex(-1);
    newResult.first->setCurrentSearchResultIndex(newResult.second);

    emit currentSearchResultIndexChanged();
}

///////////////////////////////////////////////////////////////////////////////

TextDocumentSearch::TextDocumentSearch(QObject *parent)
                   : QObject(parent),
                     m_currentResultIndex(-1),
                     m_textDocument(nullptr)
{

}

TextDocumentSearch::~TextDocumentSearch()
{

}

void TextDocumentSearch::setTextDocument(QQuickTextDocument *val)
{
    if(m_textDocument == val)
        return;

    m_textDocument = val;

    if(m_textDocument != nullptr)
        disconnect(m_textDocument, &QQuickTextDocument::destroyed, this, &TextDocumentSearch::onTextDocumentDestroyed);

    m_textDocument = val;

    if(m_textDocument != nullptr)
    {
        connect(m_textDocument, &QQuickTextDocument::destroyed, this, &TextDocumentSearch::onTextDocumentDestroyed);

        if(!m_searchString.isEmpty())
            this->doSearch(m_searchString);
    }

    emit textDocumentChanged();
}

void TextDocumentSearch::setSearchString(const QString &val)
{
    if(m_searchString == val)
        return;

    m_searchString = val;
    emit searchStringChanged();

    this->doSearch(val);
}

void TextDocumentSearch::setCurrentResultIndex(int val)
{
    if(m_searchResults.isEmpty())
    {
        if(m_currentResultIndex != -1)
        {
            emit clearHighlight();

            m_currentResultIndex = -1;
            emit currentResultIndexChanged();
        }

        return;
    }

    val = qBound(-1, val, m_searchResults.size()-1);
    if(m_currentResultIndex == val)
        return;

    emit clearHighlight();

    m_currentResultIndex = val;
    emit currentResultIndexChanged();

    if(val < 0 || val >= m_searchResults.size())
        return;

    const QPair<int,int> result = m_searchResults.at(val);
    emit highlightText(result.first, result.second);
}

void TextDocumentSearch::setSearchFlags(SearchEngine::SearchFlags val)
{
    if(m_searchFlags == val)
        return;

    m_searchFlags = val;
    emit searchFlagsChanged();
}

void TextDocumentSearch::clearSearch()
{
    this->setCurrentResultIndex(-1);

    m_searchResults.clear();
    emit searchResultCountChanged();
}

void TextDocumentSearch::nextSearchResult()
{
    this->setCurrentResultIndex(m_currentResultIndex+1);
}

void TextDocumentSearch::previousSearchResult()
{
    this->setCurrentResultIndex(m_currentResultIndex-1);
}

void TextDocumentSearch::cycleSearchResult()
{
    if(!m_searchResults.isEmpty())
        this->setCurrentResultIndex((m_currentResultIndex+1)%m_searchResults.size());
    else
        this->setCurrentResultIndex(-1);
}

void TextDocumentSearch::onTextDocumentDestroyed()
{
    m_textDocument = nullptr;

    this->setCurrentResultIndex(-1);

    m_searchResults.clear();
    emit searchResultCountChanged();
}

void TextDocumentSearch::doSearch(const QString &string)
{
    if(m_textDocument == nullptr)
        return;

    this->clearSearch();
    if(string.isEmpty())
        return;

    QTextDocument *document = m_textDocument->textDocument();
    QTextDocument::FindFlags flags;
    flags &= int(m_searchFlags);

    QTextCursor cursor(document);
    while(1)
    {
        cursor = document->find(string, cursor, flags);
        if(cursor.isNull())
            break;

        m_searchResults << qMakePair<int,int>(cursor.selectionStart(),cursor.selectionEnd());
        cursor.setPosition(cursor.selectionEnd());
    }

    if(!m_searchResults.isEmpty())
        emit searchResultCountChanged();
}
