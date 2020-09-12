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

#ifndef COMPLETER_H
#define COMPLETER_H

#include <QCompleter>

#include "execlatertimer.h"

class QStringListModel;
class Completer : public QCompleter
{
    Q_OBJECT

public:
    Completer(QObject *parent=nullptr);
    ~Completer();

    Q_PROPERTY(QStringList strings READ strings WRITE setStrings NOTIFY stringsChanged)
    void setStrings(const QStringList &val);
    QStringList strings() const { return m_strings; }
    Q_SIGNAL void stringsChanged();

    Q_PROPERTY(int minimumPrefixLength READ minimumPrefixLength WRITE setMinimumPrefixLength NOTIFY minimumPrefixLengthChanged)
    void setMinimumPrefixLength(int val);
    int minimumPrefixLength() const { return m_minimumPrefixLength; }
    Q_SIGNAL void minimumPrefixLengthChanged();

    // model() method is available in parent class.
    Q_PROPERTY(QAbstractItemModel* completionModel READ completionModel CONSTANT)

    enum SuggestionMode { CompleteSuggestion, AutoCompleteSuggestion };
    Q_ENUM(SuggestionMode)
    Q_PROPERTY(SuggestionMode suggestionMode READ suggestionMode WRITE setSuggestionMode NOTIFY suggestionModeChanged)
    void setSuggestionMode(SuggestionMode val);
    SuggestionMode suggestionMode() const { return m_suggestionMode; }
    Q_SIGNAL void suggestionModeChanged();

    Q_PROPERTY(bool hasSuggestion READ hasSuggestion NOTIFY suggestionsChanged)
    bool hasSuggestion() const { return !m_suggestions.isEmpty(); }

    Q_PROPERTY(QString suggestion READ suggestion NOTIFY suggestionsChanged)
    QString suggestion() const { return m_suggestions.isEmpty() ? QString() : m_suggestions.first(); }

    Q_PROPERTY(QStringList suggestions READ suggestions NOTIFY suggestionsChanged)
    QStringList suggestions() const { return m_suggestions; }
    Q_SIGNAL void suggestionsChanged();

protected:
    void timerEvent(QTimerEvent *te);

private:
    void setSuggestions(const QStringList &val);
    void updateSuggestions();
    void updateSuggestionsLater();

private:
    QStringList m_strings;
    int m_minimumPrefixLength = 1;
    QStringList m_suggestions;
    SuggestionMode m_suggestionMode = AutoCompleteSuggestion;
    QStringListModel *m_stringsModel = nullptr;
    ExecLaterTimer m_updateSuggestionTimer;
};

#endif // COMPLETER_H
