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

#include "completer.h"

#include <QtDebug>
#include <QStringListModel>
#include <QEvent>

Completer::Completer(QObject *parent)
          :QCompleter(parent),
            m_updateSuggestionTimer("Completer.m_updateSuggestionTimer")
{
    this->setCompletionMode(QCompleter::PopupCompletion);
    this->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    this->setCaseSensitivity(Qt::CaseInsensitive);

    m_stringsModel = new QStringListModel(this);
    this->setModel(m_stringsModel);

    QAbstractItemModel *cmodel = this->completionModel();
    connect(cmodel, &QAbstractItemModel::rowsInserted, this, &Completer::updateSuggestionsLater);
    connect(cmodel, &QAbstractItemModel::rowsRemoved, this, &Completer::updateSuggestionsLater);
    connect(cmodel, &QAbstractItemModel::modelReset, this, &Completer::updateSuggestionsLater);
    connect(cmodel, &QAbstractItemModel::dataChanged, this, &Completer::updateSuggestionsLater);
}

Completer::~Completer()
{

}

void Completer::setStrings(const QStringList &val)
{
    if(m_strings == val)
        return;

    m_strings = val;
    m_stringsModel->setStringList(m_strings);

    emit stringsChanged();

    this->updateSuggestionsLater();
}

void Completer::setMinimumPrefixLength(int val)
{
    if(m_minimumPrefixLength == val)
        return;

    m_minimumPrefixLength = val;
    emit minimumPrefixLengthChanged();

    this->updateSuggestionsLater();
}

void Completer::setSuggestionMode(Completer::SuggestionMode val)
{
    if(m_suggestionMode == val)
        return;

    m_suggestionMode = val;
    emit suggestionModeChanged();

    this->updateSuggestionsLater();
}

void Completer::timerEvent(QTimerEvent *te)
{
    if(m_updateSuggestionTimer.timerId() == te->timerId())
    {
        m_updateSuggestionTimer.stop();
        this->updateSuggestions();
    }

    QObject::timerEvent(te);
}

void Completer::setSuggestions(const QStringList &val)
{
    if(m_suggestions == val)
        return;

    m_suggestions = val;
    emit suggestionsChanged();
}

void Completer::updateSuggestions()
{
    QStringList vals;

    const QString prefix = this->completionPrefix();
    if(m_minimumPrefixLength > 0 && prefix.length() < m_minimumPrefixLength)
    {
        this->setSuggestions(vals);
        return;
    }

    if(!m_strings.isEmpty())
    {
        const QAbstractItemModel *cmodel = this->completionModel();
        const int rows = qMin(cmodel->rowCount(), this->maxVisibleItems());

        for(int i=0; i<rows; i++)
        {
            const QModelIndex index = cmodel->index(i, 0);
            QString val = index.data(Qt::DisplayRole).toString();
            if(m_suggestionMode == AutoCompleteSuggestion)
                val = val.remove(0, prefix.length());
            if(!val.isEmpty())
                vals << val;
        }
    }

    this->setSuggestions(vals);
}

void Completer::updateSuggestionsLater()
{
    m_updateSuggestionTimer.start(0, this);
}
