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

#include "completer.h"

#include <QStringListModel>

Completer::Completer(QObject *parent)
          :QCompleter(parent),
           m_suggestionMode(AutoCompleteSuggestion)
{
    this->setCompletionMode(QCompleter::PopupCompletion);
    this->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    this->setCaseSensitivity(Qt::CaseInsensitive);

    m_stringsModel = new QStringListModel(this);
    this->setModel(m_stringsModel);

    QAbstractItemModel *cmodel = this->completionModel();
    connect(cmodel, &QAbstractItemModel::rowsInserted, this, &Completer::suggestionChanged);
    connect(cmodel, &QAbstractItemModel::rowsRemoved, this, &Completer::suggestionChanged);
    connect(cmodel, &QAbstractItemModel::modelReset, this, &Completer::suggestionChanged);
    connect(cmodel, &QAbstractItemModel::dataChanged, this, &Completer::suggestionChanged);
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

QString Completer::suggestion() const
{
    const QAbstractItemModel *cmodel = this->completionModel();
    const int rows = cmodel->rowCount();
    if(rows == 0)
        return QString();

    const QModelIndex index = cmodel->index(0, 0);

    QString ret = index.data(Qt::DisplayRole).toString();
    if(m_suggestionMode == AutoCompleteSuggestion)
        ret = ret.remove(0, this->completionPrefix().length());

    return ret;
}
