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
          :QCompleter(parent)
{
    this->setCompletionMode(QCompleter::PopupCompletion);
    this->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    this->setCaseSensitivity(Qt::CaseInsensitive);

    m_stringsModel = new QStringListModel(this);
    this->setModel(m_stringsModel);
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
