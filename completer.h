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

    enum SuggestionMode { CompleteSuttion, AutoCompleteSuggestion };
    Q_ENUM(SuggestionMode)
    Q_PROPERTY(SuggestionMode suggestionMode READ suggestionMode WRITE setSuggestionMode NOTIFY suggestionModeChanged)
    void setSuggestionMode(SuggestionMode val);
    SuggestionMode suggestionMode() const { return m_suggestionMode; }
    Q_SIGNAL void suggestionModeChanged();

    Q_PROPERTY(QString suggestion READ suggestion NOTIFY suggestionChanged)
    QString suggestion() const;
    Q_SIGNAL void suggestionChanged();

private:
    QStringList m_strings;
    SuggestionMode m_suggestionMode;
    QStringListModel *m_stringsModel;
};

#endif // COMPLETER_H
