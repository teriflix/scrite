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

#include <QKeyEvent>
#include <QtConcurrentRun>
#include <QGuiApplication>

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

    connect(this, &CompletionModel::currentRowChanged, this,
            &CompletionModel::currentCompletionChanged);
    connect(this, &QAbstractListModel::rowsInserted, this,
            &CompletionModel::currentCompletionChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this,
            &CompletionModel::currentCompletionChanged);
    connect(this, &QAbstractListModel::modelReset, this,
            &CompletionModel::currentCompletionChanged);
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
            this->clearFilterStrings();
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            const QString cc = this->currentCompletion();
            if (!cc.isEmpty()) {
                emit requestCompletion(cc);

                QTimer::singleShot(0, this, &CompletionModel::clearFilterStrings);
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
        this->clearFilterStrings();
        return;
    }

    bool someFilteringHappened = false;
    QStringList fstrings;
    if (m_completionPrefix.isEmpty())
        fstrings = m_strings2;
    else {
        if (m_completionPrefix.isEmpty())
            fstrings = m_strings2;
        // if an exact match was found, then clear the completion model
        // even if there is another potential match possible.
        else if (m_strings2.contains(m_completionPrefix, Qt::CaseInsensitive))
            fstrings.clear();
        else
            std::copy_if(m_strings2.begin(), m_strings2.end(), std::back_inserter(fstrings),
                         [&](const QString &item) {
                             const bool ret =
                                     item.startsWith(m_completionPrefix, Qt::CaseInsensitive);
                             someFilteringHappened |= !ret;
                             return ret;
                         });
    }

    this->beginResetModel();
    m_filteredStrings = fstrings.isEmpty() ? fstrings
            : m_maxVisibleItems > 0        ? fstrings.mid(0, m_maxVisibleItems)
                                           : fstrings;
    this->endResetModel();

    if (m_filteredStrings.isEmpty() || !someFilteringHappened)
        this->setCurrentRow(-1);
    else
        this->setCurrentRow(0);
}

void CompletionModel::prepareStrings()
{
    m_strings2.clear();
    m_priorityStrings2.clear();

    if (m_acceptEnglishStringsOnly)
        std::copy_if(m_strings.begin(), m_strings.end(), std::back_inserter(m_strings2),
                     isEnglishString);
    else
        m_strings2 = m_strings;
    m_strings2.removeDuplicates();

    std::copy_if(m_priorityStrings.begin(), m_priorityStrings.end(),
                 std::back_inserter(m_priorityStrings2), [=](const QString &item) {
                     return m_strings2.contains(item, Qt::CaseInsensitive);
                 });
    m_priorityStrings2.removeDuplicates();

    if (m_sortStrings)
        std::sort(m_strings2.begin(), m_strings2.end());

    for (int i = m_priorityStrings2.size() - 1; i >= 0; i--) {
        const QString priorityString2 = m_priorityStrings2.at(i);
        m_strings2.removeOne(priorityString2);
        m_strings2.prepend(priorityString2);
    }

    this->filterStrings();
}

void CompletionModel::clearFilterStrings()
{
    if (!m_filteredStrings.isEmpty()) {
        this->beginResetModel();
        m_filteredStrings.clear();
        this->endResetModel();
    }

    this->setCurrentRow(-1);
}
