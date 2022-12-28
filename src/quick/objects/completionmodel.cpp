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

#include "completionmodel.h"
#include "timeprofiler.h"
#include "application.h"

#include <QGuiApplication>
#include <QKeyEvent>

static bool isEnglishString(const QString &item)
{
    for (const QChar &ch : item) {
        if (ch.isLetter() && ch.script() != QChar::Script_Latin)
            return false;
    }

    return true;
}

CompletionModel::CompletionModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::rowsInserted, this, &CompletionModel::countChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &CompletionModel::countChanged);
    connect(this, &QAbstractListModel::modelReset, this, &CompletionModel::countChanged);
}

CompletionModel::~CompletionModel() { }

void CompletionModel::setStrings(const QStringList &val)
{
    if (m_strings == val)
        return;

    m_strings = val;
    emit stringsChanged();

    this->prepareStrings();
}

void CompletionModel::setPriorityStrings(QStringList val)
{
    if (m_priorityStrings == val)
        return;

    m_priorityStrings = val;
    emit priorityStringsChanged();

    this->prepareStrings();
}

void CompletionModel::setAcceptEnglishStringsOnly(bool val)
{
    if (m_acceptEnglishStringsOnly == val)
        return;

    m_acceptEnglishStringsOnly = val;
    emit acceptEnglishStringsOnlyChanged();

    this->prepareStrings();
}

void CompletionModel::setSortStrings(bool val)
{
    if (m_sortStrings == val)
        return;

    m_sortStrings = val;
    emit sortStringsChanged();

    this->prepareStrings();
}

void CompletionModel::setSortMode(SortMode val)
{
    if (m_sortMode == val)
        return;

    m_sortMode = val;
    emit sortModeChanged();
}

void CompletionModel::setMaxVisibleItems(int val)
{
    if (m_maxVisibleItems == val)
        return;

    m_maxVisibleItems = val;
    emit maxVisibleItemsChanged();

    this->filterStrings();
}

void CompletionModel::setMinimumCompletionPrefixLength(int val)
{
    if (m_minimumCompletionPrefixLength == val)
        return;

    m_minimumCompletionPrefixLength = val;
    emit minimumCompletionPrefixLengthChanged();

    this->filterStrings();
}

void CompletionModel::setCompletionPrefix(const QString &val)
{
    if (m_completionPrefix == val)
        return;

    m_completionPrefix = val;
    emit completionPrefixChanged();

    this->filterStrings();
}

QString CompletionModel::currentCompletion() const
{
    return m_currentRow < 0 || m_currentRow >= m_filteredStrings.size()
            ? QString()
            : m_filteredStrings.at(m_currentRow);
}

void CompletionModel::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    this->filterStrings();
}

void CompletionModel::setFilterKeyStrokes(bool val)
{
    if (m_filterKeyStrokes == val)
        return;

    m_filterKeyStrokes = val;
    emit filterKeyStrokesChanged();

    if (val)
        qApp->installEventFilter(this);
    else
        qApp->removeEventFilter(this);
}

int CompletionModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_filteredStrings.size();
}

QVariant CompletionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_filteredStrings.size())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    return m_filteredStrings.at(index.row());
}

QHash<int, QByteArray> CompletionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "string";
    return roles;
}

bool CompletionModel::eventFilter(QObject *target, QEvent *event)
{
    Q_UNUSED(target);

    if (m_filteredStrings.isEmpty())
        return false;

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        switch (ke->key()) {
        case Qt::Key_Up:
            if (m_currentRow > 0) {
                this->setCurrentRow(qMax(m_currentRow - 1, 0));
                return true;
            }
            break;
        case Qt::Key_Down:
            if (m_currentRow < m_filteredStrings.size()) {
                this->setCurrentRow(qMin(m_currentRow + 1, m_filteredStrings.size() - 1));
                return true;
            }
            break;
        case Qt::Key_Escape:
            this->beginResetModel();
            m_filteredStrings.clear();
            this->endResetModel();
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            const QString cc = this->currentCompletion();
            if (!cc.isEmpty()) {
                emit requestCompletion(cc);
                return true;
            }
        } break;
        }
    }

    return false;
}

void CompletionModel::setCurrentRow(int val)
{
    if (m_currentRow == val)
        return;

    m_currentRow = val;
    emit currentRowChanged();
}

void CompletionModel::filterStrings()
{
    if (!m_enabled || m_completionPrefix.size() < m_minimumCompletionPrefixLength
        || m_strings2.isEmpty()) {
        if (m_filteredStrings.isEmpty())
            return;

        this->beginResetModel();
        m_filteredStrings.clear();
        this->endResetModel();

        this->setCurrentRow(-1);

        return;
    }

    bool someFilteringHappened = false;
    QStringList fstrings;
    if (m_completionPrefix.isEmpty())
        fstrings = m_strings2;
    else
        std::copy_if(m_strings2.begin(), m_strings2.end(), std::back_inserter(fstrings),
                     [&](const QString &item) {
                         if (m_completionPrefix.isEmpty())
                             return true;
                         const bool ret = item.compare(m_completionPrefix, Qt::CaseInsensitive)
                                 && item.startsWith(m_completionPrefix, Qt::CaseInsensitive);
                         someFilteringHappened |= !ret;
                         return ret;
                     });

    this->beginResetModel();
    m_filteredStrings = m_maxVisibleItems > 0 ? fstrings.mid(0, m_maxVisibleItems) : fstrings;
    this->endResetModel();

    if (m_filteredStrings.isEmpty() || !someFilteringHappened)
        this->setCurrentRow(-1);
    else
        this->setCurrentRow(0);
}

class CaseInsensitiveFinder
{
public:
    CaseInsensitiveFinder(const QStringList &list) : m_list(list) { }
    ~CaseInsensitiveFinder() { }

    int indexOf(const QString &item) const
    {
        if (m_list.isEmpty() || item.isEmpty())
            return -1;

        if (m_indexMap.isEmpty()) {
            int index = 0;
            for (const QString &litem : qAsConst(m_list))
                m_indexMap[litem.toUpper()] = index++;
        }

        return m_indexMap.value(item.toUpper(), -1);
    }

private:
    QStringList m_list;
    mutable QHash<QString, int> m_indexMap;
};

void CompletionModel::prepareStrings()
{
    m_strings2.clear();

    if (m_acceptEnglishStringsOnly) {
        if (!m_strings.isEmpty()) {
            std::copy_if(m_strings.begin(), m_strings.end(), std::back_inserter(m_strings2),
                         isEnglishString);
        }
    } else
        m_strings2 = m_strings;

    if (!m_priorityStrings.isEmpty())
        std::copy_if(m_priorityStrings.begin(), m_priorityStrings.end(),
                     std::back_inserter(m_priorityStrings2), [=](const QString &item) {
                         return m_strings2.contains(item, Qt::CaseInsensitive);
                     });

    CaseInsensitiveFinder priorityStrings2Finder(m_priorityStrings2);
    CaseInsensitiveFinder strings2Finder(m_strings2);

    std::sort(m_strings2.begin(), m_strings2.end(), [=](const QString &a, const QString &b) {
        const int a_index = m_priorityStrings2.isEmpty()
                ? -1
                : (m_sortMode == CaseInsensitiveSort ? priorityStrings2Finder.indexOf(a)
                                                     : m_priorityStrings2.indexOf(a));
        const int b_index = m_priorityStrings2.isEmpty()
                ? -1
                : (m_sortMode == CaseInsensitiveSort ? priorityStrings2Finder.indexOf(b)
                                                     : m_priorityStrings2.indexOf(b));
        if (a_index >= 0 && b_index >= 0)
            return a_index < b_index;

        if (a_index >= 0)
            return true;

        if (b_index >= 0)
            return false;

        if (m_sortStrings)
            return a.compare(b,
                             m_sortMode == CaseInsensitiveSort ? Qt::CaseInsensitive
                                                               : Qt::CaseSensitive)
                    < 0;

        if (m_sortMode == CaseInsensitiveSort)
            return strings2Finder.indexOf(a) < strings2Finder.indexOf(b);

        return m_strings2.indexOf(a) < m_strings2.indexOf(b);
    });

    this->filterStrings();
}
