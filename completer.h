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

#ifndef COMPLETER_H
#define COMPLETER_H

#include <QCompleter>

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

    // model() method is available in parent class.
    Q_PROPERTY(QAbstractItemModel* completionModel READ completionModel CONSTANT)

private:
    QStringList m_strings;
    QStringListModel *m_stringsModel;
};

#endif // COMPLETER_H
