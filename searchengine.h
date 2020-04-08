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

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include <QJsonArray>
#include <QQmlEngine>
#include <QBasicTimer>
#include <QQuickTextDocument>

#include "errorreport.h"
#include "progressreport.h"

class SearchEngine;

class SearchAgent : public QObject
{
    Q_OBJECT

public:
    ~SearchAgent();
    Q_SIGNAL void aboutToDelete(SearchAgent *agent);

    static SearchAgent *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(SearchEngine* engine READ engine WRITE setEngine NOTIFY engineChanged)
    void setEngine(SearchEngine* val);
    SearchEngine* engine() const { return m_engine; }
    Q_SIGNAL void engineChanged();

    Q_PROPERTY(int sequenceNumber READ sequenceNumber WRITE setSequenceNumber NOTIFY sequenceNumberChanged)
    void setSequenceNumber(int val);
    int sequenceNumber() const { return m_sequenceNumber; }
    Q_SIGNAL void sequenceNumberChanged();

    Q_SIGNAL void searchRequest(const QString &string);

    Q_PROPERTY(int searchResultCount READ searchResultCount WRITE setSearchResultCount NOTIFY searchResultCountChanged)
    void setSearchResultCount(int val);
    int searchResultCount() const { return m_searchResultCount; }
    Q_SIGNAL void searchResultCountChanged();

    Q_PROPERTY(int currentSearchResultIndex READ currentSearchResultIndex WRITE setCurrentSearchResultIndex NOTIFY currentSearchResultIndexChanged)
    void setCurrentSearchResultIndex(int val);
    int currentSearchResultIndex() const { return m_currentSearchResultIndex; }
    Q_SIGNAL void currentSearchResultIndexChanged();

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)
    void setTextDocument(QQuickTextDocument* val);
    QQuickTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    // Emitted only if textDocument is set
    Q_SIGNAL void highlightText(int start, int end);
    Q_SIGNAL void clearHighlight();

    Q_SIGNAL void clearSearchRequest();

    // Helper function
    Q_INVOKABLE QJsonArray indexesOf(const QString &of, const QString &in) const;
    Q_INVOKABLE QString createMarkupText(const QString &text, int from, int to, const QColor &bg, const QColor &fg) const;

protected:
    SearchAgent(QObject *parent=nullptr);
    void onTextDocumentDestroyed();
    void onSearchEngineDestroyed();
    void onSearchRequest(const QString &string);
    void onClearSearchRequest();

private:
    int m_sequenceNumber;
    SearchEngine* m_engine;
    int m_searchResultCount;
    int m_currentSearchResultIndex;
    QQuickTextDocument* m_textDocument;
    QList< QPair<int,int> > m_textDocumentSearchResults;
};
Q_DECLARE_METATYPE(SearchAgent*)
QML_DECLARE_TYPEINFO(SearchAgent, QML_HAS_ATTACHED_PROPERTIES)

class SearchEngine : public QObject
{
    Q_OBJECT

public:
    SearchEngine(QObject *parent=nullptr);
    ~SearchEngine();

    Q_PROPERTY(QQmlListProperty<SearchAgent> searchAgents READ searchAgents NOTIFY searchAgentsChanged)
    QQmlListProperty<SearchAgent> searchAgents();
    Q_INVOKABLE SearchAgent *searchAgentAt(int index) const;
    Q_PROPERTY(int searchAgentCount READ searchAgentCount NOTIFY searchAgentCountChanged)
    int searchAgentCount() const { return m_searchAgents.size(); }
    Q_INVOKABLE void clearSearchAgents();
    Q_SIGNAL void searchAgentCountChanged();
    Q_SIGNAL void searchAgentsChanged();

    enum SearchFlag
    {
        SearchBackward        = 0x00001,
        SearchCaseSensitively = 0x00002,
        SearchWholeWords      = 0x00004
    };
    Q_DECLARE_FLAGS(SearchFlags, SearchFlag)
    Q_PROPERTY(SearchFlags searchFlags READ searchFlags WRITE setSearchFlags NOTIFY searchFlagsChanged)
    void setSearchFlags(SearchFlags val);
    SearchFlags searchFlags() const { return m_searchFlags; }
    Q_SIGNAL void searchFlagsChanged();

    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    void setSearchString(const QString &val);
    QString searchString() const { return m_searchString; }
    Q_SIGNAL void searchStringChanged();

    Q_PROPERTY(int searchResultCount READ searchResultCount NOTIFY searchResultCountChanged)
    int searchResultCount() const { return m_searchResults.size(); }
    Q_SIGNAL void searchResultCountChanged();

    Q_PROPERTY(int currentSearchResultIndex READ currentSearchResultIndex NOTIFY currentSearchResultIndexChanged)
    int currentSearchResultIndex() const { return m_currentSearchResultIndex; }
    Q_SIGNAL void currentSearchResultIndexChanged();

    Q_INVOKABLE void clearSearch();
    Q_INVOKABLE void nextSearchResult();
    Q_INVOKABLE void previousSearchResult();
    Q_INVOKABLE void cycleSearchResult();

    static QJsonArray indexesOf(const QString &of, const QString &in, int flags);
    static QString createMarkupText(const QString &text, int from, int to, const QBrush &bg, const QBrush &fg);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void addSearchAgent(SearchAgent *ptr);
    void removeSearchAgent(SearchAgent *ptr);
    void sortSearchAgents();
    void sortSearchAgentsLater();
    static SearchAgent* staticSearchAgentAt(QQmlListProperty<SearchAgent> *list, int index);
    static int staticSearchAgentCount(QQmlListProperty<SearchAgent> *list);

    void doSearch();
    void doSearchLater();
    void setCurrentSearchResultIndex(int val);

private:
    QString m_searchString;
    friend class SearchAgent;
    SearchFlags m_searchFlags;
    QBasicTimer m_searchTimer;
    ErrorReport *m_errorReport;
    int m_currentSearchResultIndex;
    ProgressReport *m_progressReport;
    QBasicTimer m_searchAgentSortTimer;
    QList<SearchAgent *> m_searchAgents;
    QList< QPair<SearchAgent*,int> > m_searchResults;
};

class TextDocumentSearch : public QObject
{
    Q_OBJECT

public:
    TextDocumentSearch(QObject *parent=nullptr);
    ~TextDocumentSearch();

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)
    void setTextDocument(QQuickTextDocument* val);
    QQuickTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    void setSearchString(const QString &val);
    QString searchString() const { return m_searchString; }
    Q_SIGNAL void searchStringChanged();

    Q_PROPERTY(int searchResultCount READ searchResultCount NOTIFY searchResultCountChanged)
    int searchResultCount() const { return m_searchResults.size(); }
    Q_SIGNAL void searchResultCountChanged();

    Q_PROPERTY(int currentResultIndex READ currentResultIndex WRITE setCurrentResultIndex NOTIFY currentResultIndexChanged)
    void setCurrentResultIndex(int val);
    int currentResultIndex() const { return m_currentResultIndex; }
    Q_SIGNAL void currentResultIndexChanged();

    Q_PROPERTY(SearchEngine::SearchFlags searchFlags READ searchFlags WRITE setSearchFlags NOTIFY searchFlagsChanged)
    void setSearchFlags(SearchEngine::SearchFlags val);
    SearchEngine::SearchFlags searchFlags() const { return m_searchFlags; }
    Q_SIGNAL void searchFlagsChanged();

    Q_INVOKABLE void clearSearch();
    Q_INVOKABLE void nextSearchResult();
    Q_INVOKABLE void previousSearchResult();
    Q_INVOKABLE void cycleSearchResult();

    Q_SIGNAL void clearHighlight();
    Q_SIGNAL void highlightText(int start, int end);

private:
    void setCurrentResultInternal(int val);
    void onTextDocumentDestroyed();
    void doSearch(const QString &string);

private:
    QString m_searchString;
    int m_currentResultIndex;
    QQuickTextDocument* m_textDocument;
    SearchEngine::SearchFlags m_searchFlags;
    QList< QPair<int,int> > m_searchResults;
};

#endif // SEARCHENGINE_H
