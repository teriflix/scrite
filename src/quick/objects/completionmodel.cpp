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

#include <QGuiApplication>
#include <QKeyEvent>

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

    if (m_acceptEnglishStringsOnly) {
        m_strings.clear();
        std::copy_if(
                val.begin(), val.end(), std::back_inserter(m_strings), [](const QString &item) {
                    QList<QChar> nonLatinChars;
                    std::copy_if(item.begin(), item.end(), std::back_inserter(nonLatinChars),
                                 [](const QChar &ch) {
                                     return ch.isLetter() && ch.script() != QChar::Script_Latin;
                                 });
                    return nonLatinChars.isEmpty();
                });
    } else
        m_strings = val;

    if (m_sortStrings)
        std::sort(m_strings.begin(), m_strings.end());

    emit stringsChanged();

    this->filterStrings();
}

void CompletionModel::setAcceptEnglishStringsOnly(bool val)
{
    if (m_acceptEnglishStringsOnly == val)
        return;

    m_acceptEnglishStringsOnly = val;
    emit acceptEnglishStringsOnlyChanged();
}

void CompletionModel::setSortStrings(bool val)
{
    if (m_sortStrings == val)
        return;

    m_sortStrings = val;
    emit sortStringsChanged();

    if (val) {
        std::sort(m_strings.begin(), m_strings.end());
        emit stringsChanged();

        this->filterStrings();
    }
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
    if (!m_enabled || m_completionPrefix.size() < m_minimumCompletionPrefixLength) {
        if (m_filteredStrings.isEmpty())
            return;

        this->beginResetModel();
        m_filteredStrings.clear();
        this->endResetModel();

        this->setCurrentRow(-1);

        return;
    }

    QStringList fstrings;
    std::copy_if(m_strings.begin(), m_strings.end(), std::back_inserter(fstrings),
                 [=](const QString &item) {
                     return item.compare(m_completionPrefix, Qt::CaseInsensitive)
                             && item.startsWith(m_completionPrefix, Qt::CaseInsensitive);
                 });

    this->beginResetModel();
    m_filteredStrings = m_maxVisibleItems > 0 ? fstrings.mid(0, m_maxVisibleItems) : fstrings;
    this->endResetModel();

    if (m_filteredStrings.isEmpty())
        this->setCurrentRow(-1);
    else {
        m_currentRow = 0;
        emit currentRowChanged();
    }
}
