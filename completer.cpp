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
          :QCompleter(parent)
{
    this->setCompletionMode(QCompleter::PopupCompletion);
    this->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    this->setCaseSensitivity(Qt::CaseInsensitive);

    m_stringsModel = new QStringListModel(this);
    this->setModel(m_stringsModel);

    QAbstractItemModel *cmodel = this->completionModel();
    connect(cmodel, &QAbstractItemModel::rowsInserted, this, &Completer::updateSuggestionLater);
    connect(cmodel, &QAbstractItemModel::rowsRemoved, this, &Completer::updateSuggestionLater);
    connect(cmodel, &QAbstractItemModel::modelReset, this, &Completer::updateSuggestionLater);
    connect(cmodel, &QAbstractItemModel::dataChanged, this, &Completer::updateSuggestionLater);
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
}

void Completer::setSuggestionMode(Completer::SuggestionMode val)
{
    if(m_suggestionMode == val)
        return;

    m_suggestionMode = val;
    emit suggestionModeChanged();
    emit suggestionChanged();
}

void Completer::timerEvent(QTimerEvent *te)
{
    if(m_updateSuggestionTimer.timerId() == te->timerId())
    {
        this->updateSuggestion();
        m_updateSuggestionTimer.stop();
    }

    QObject::timerEvent(te);
}

void Completer::updateSuggestion()
{
    const QAbstractItemModel *cmodel = this->completionModel();
    const int rows = cmodel->rowCount();
    QString val;

    if(rows > 0)
    {
        const QModelIndex index = cmodel->index(0, 0);

        val = index.data(Qt::DisplayRole).toString();
        if(m_suggestionMode == AutoCompleteSuggestion)
            val = val.remove(0, this->completionPrefix().length());
    }

    if(m_suggestion == val)
        return;

    m_suggestion = val;
    emit suggestionChanged();
}

void Completer::updateSuggestionLater()
{
    m_updateSuggestionTimer.start(0, this);
}
