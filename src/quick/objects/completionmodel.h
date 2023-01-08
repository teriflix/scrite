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

#ifndef COMPLETIONMODEL_H
#define COMPLETIONMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

class CompletionModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CompletionModel(QObject *parent = nullptr);
    ~CompletionModel();

    Q_PROPERTY(QStringList strings READ strings WRITE setStrings NOTIFY stringsChanged)
    void setStrings(const QStringList &val);
    QStringList strings() const { return m_strings; }
    Q_SIGNAL void stringsChanged();

    Q_PROPERTY(QStringList priorityStrings READ priorityStrings WRITE setPriorityStrings NOTIFY priorityStringsChanged)
    void setPriorityStrings(QStringList val);
    QStringList priorityStrings() const { return m_priorityStrings; }
    Q_SIGNAL void priorityStringsChanged();

    Q_PROPERTY(bool acceptEnglishStringsOnly READ isAcceptEnglishStringsOnly WRITE setAcceptEnglishStringsOnly NOTIFY acceptEnglishStringsOnlyChanged)
    void setAcceptEnglishStringsOnly(bool val);
    bool isAcceptEnglishStringsOnly() const { return m_acceptEnglishStringsOnly; }
    Q_SIGNAL void acceptEnglishStringsOnlyChanged();

    Q_PROPERTY(bool sortStrings READ sortStrings WRITE setSortStrings NOTIFY sortStringsChanged)
    void setSortStrings(bool val);
    bool sortStrings() const { return m_sortStrings; }
    Q_SIGNAL void sortStringsChanged();

    enum SortMode { CaseSensitiveSort, CaseInsensitiveSort };
    Q_ENUM(SortMode)

    Q_PROPERTY(SortMode sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    void setSortMode(SortMode val);
    SortMode sortMode() const { return m_sortMode; }
    Q_SIGNAL void sortModeChanged();

    Q_PROPERTY(int maxVisibleItems READ maxVisibleItems WRITE setMaxVisibleItems NOTIFY maxVisibleItemsChanged)
    void setMaxVisibleItems(int val);
    int maxVisibleItems() const { return m_maxVisibleItems; }
    Q_SIGNAL void maxVisibleItemsChanged();

    Q_PROPERTY(int minimumCompletionPrefixLength READ minimumCompletionPrefixLength WRITE setMinimumCompletionPrefixLength NOTIFY minimumCompletionPrefixLengthChanged)
    void setMinimumCompletionPrefixLength(int val);
    int minimumCompletionPrefixLength() const { return m_minimumCompletionPrefixLength; }
    Q_SIGNAL void minimumCompletionPrefixLengthChanged();

    Q_PROPERTY(QString completionPrefix READ completionPrefix WRITE setCompletionPrefix NOTIFY completionPrefixChanged)
    void setCompletionPrefix(const QString &val);
    QString completionPrefix() const { return m_completionPrefix; }
    Q_SIGNAL void completionPrefixChanged();

    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
    void setCurrentRow(int val);
    int currentRow() const { return m_currentRow; }
    Q_SIGNAL void currentRowChanged();

    Q_PROPERTY(QString currentCompletion READ currentCompletion NOTIFY currentCompletionChanged)
    QString currentCompletion() const;
    Q_SIGNAL void currentCompletionChanged();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_filteredStrings.size(); }
    Q_SIGNAL void countChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(bool filterKeyStrokes READ isFilterKeyStrokes WRITE setFilterKeyStrokes NOTIFY filterKeyStrokesChanged)
    void setFilterKeyStrokes(bool val);
    bool isFilterKeyStrokes() const { return m_filterKeyStrokes; }
    Q_SIGNAL void filterKeyStrokesChanged();

    Q_SIGNAL void requestCompletion(const QString &string);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    bool eventFilter(QObject *target, QEvent *event);

private:
    void filterStrings();
    void prepareStrings();
    void clearFilterStrings();

private:
    int m_currentRow = -1;
    QString m_prefix;
    bool m_enabled = true;
    QStringList m_strings;
    QStringList m_priorityStrings;
    bool m_sortStrings = true;
    int m_maxVisibleItems = 7;
    QString m_completionPrefix;
    QStringList m_strings2;
    QStringList m_priorityStrings2;
    QStringList m_filteredStrings;
    bool m_filterKeyStrokes = false;
    bool m_acceptEnglishStringsOnly = true;
    int m_minimumCompletionPrefixLength = 0;
    SortMode m_sortMode = CaseInsensitiveSort;
};

#endif // COMPLETIONMODEL_H
