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

#include "systemtextinputmanager.h"
#include <QCoreApplication>
#include <QtDebug>

#ifdef Q_OS_MAC
#include "systemtextinputmanager_macos.h"
#endif

SystemTextInputManager *SystemTextInputManager::instance()
{
    static SystemTextInputManager *theInstance = new SystemTextInputManager(qApp);
    return theInstance;
}

SystemTextInputManager::SystemTextInputManager(QObject *parent)
    : QAbstractListModel(parent)
{
#ifdef Q_OS_MAC
    m_backend = new SystemTextInputManagerBackend_macOS(this);
#endif

    if(m_backend != nullptr)
        m_backend->reloadSources();
}

SystemTextInputManager::~SystemTextInputManager()
{
    this->clear();
}

void SystemTextInputManager::reload()
{
    this->clear();
    if(m_backend != nullptr)
        m_backend->reloadSources();
}

AbstractSystemTextInputSource *SystemTextInputManager::sourceAt(int index) const
{
    if(index < 0 || index >= m_inputSources.size())
        return nullptr;

    return m_inputSources.at(index);
}

int SystemTextInputManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_inputSources.size();
}

QVariant SystemTextInputManager::data(const QModelIndex &index, int role) const
{
    if(index.row() >= 0 && index.row() <= m_inputSources.size())
    {
        AbstractSystemTextInputSource *source = m_inputSources.at(index.row());
        switch(role)
        {
        case TextInputSourceRole:
            return QVariant::fromValue<QObject*>(source);
        case TextInputSourceIDRole:
            return source->id();
        case TextInputSourceDisplayNameRole:
            return source->displayName();
        case TextInputSourceSelectedRole:
            return source->isSelected();
        case Qt::DisplayRole:
            return source->displayName() + QStringLiteral(" [") + source->id() + QStringLiteral("]: ") + (source->isSelected() ? QStringLiteral("ACTIVE") : QString());
        }
    }

    return QVariant();
}

QHash<int, QByteArray> SystemTextInputManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TextInputSourceRole] = "tisObject";
    roles[TextInputSourceIDRole] = "tisID";
    roles[TextInputSourceDisplayNameRole] = "tisDisplayName";
    roles[TextInputSourceSelectedRole] = "tisSelected";
    return roles;
}

void SystemTextInputManager::clear()
{
    if(!m_inputSources.isEmpty())
    {
        this->beginResetModel();

        QList<AbstractSystemTextInputSource*> sources = m_inputSources;
        m_inputSources.clear();
        qDeleteAll(sources);

        this->endResetModel();

        emit countChanged();
    }
}

void SystemTextInputManager::add(AbstractSystemTextInputSource *source)
{
    if(source == nullptr || m_inputSources.contains(source))
        return;

    this->beginInsertRows(QModelIndex(), m_inputSources.size(), m_inputSources.size());
    m_inputSources.append(source);
    this->endInsertRows();

    emit countChanged();
}

void SystemTextInputManager::remove(AbstractSystemTextInputSource *source)
{
    if(source == nullptr || m_inputSources.isEmpty())
        return;

    const int row = m_inputSources.indexOf(source);
    if(row < 0)
        return;

    if(m_selectedInputSource == source)
        this->setSelected(nullptr);

    this->beginRemoveRows(QModelIndex(), row, row);
    m_inputSources.removeAt(row);
    this->endRemoveRows();

    emit countChanged();

    if(m_selectedInputSource == nullptr && m_backend != nullptr)
        m_backend->determineSelectedInputSource();
}

void SystemTextInputManager::setSelected(AbstractSystemTextInputSource *source)
{
    if(m_selectedInputSource == source)
        return;

    const int oldRow = m_selectedInputSource == nullptr ? -1 : m_inputSources.indexOf(m_selectedInputSource);

    m_selectedInputSource = source;
    emit selectedInputSourceChanged();

    if(oldRow >= 0)
    {
        const QModelIndex oldIndex = this->index(oldRow);
        emit dataChanged(oldIndex, oldIndex);
    }

    const int newRow = m_selectedInputSource == nullptr ? -1 : m_inputSources.indexOf(m_selectedInputSource);
    if(newRow >= 0)
    {
        const QModelIndex newIndex = this->index(newRow);
        emit dataChanged(newIndex, newIndex);
    }
}

///////////////////////////////////////////////////////////////////////////////

AbstractSystemTextInputSource::AbstractSystemTextInputSource(SystemTextInputManager *parent)
    : QObject(parent), m_inputManager(parent)
{
    m_addTimer.start(0, this);
}

AbstractSystemTextInputSource::~AbstractSystemTextInputSource()
{
    m_inputManager->remove(this);
}

void AbstractSystemTextInputSource::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_addTimer.timerId())
    {
        m_addTimer.stop();
        m_inputManager->add(this);
        return;
    }

    QObject::timerEvent(event);
}

void AbstractSystemTextInputSource::setSelected(bool val)
{
    if(m_selected == val)
        return;

    m_selected = val;
    emit selectedChanged();

    m_inputManager->setSelected(this);
}

///////////////////////////////////////////////////////////////////////////////

AbstractSystemTextInputManagerBackend::AbstractSystemTextInputManagerBackend(SystemTextInputManager *parent)
    : QObject(parent), m_inputManager(parent)
{

}

AbstractSystemTextInputManagerBackend::~AbstractSystemTextInputManagerBackend()
{

}


