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

#include "systemtextinputmanager.h"
#include "transliteration.h"

#include <QtDebug>
#include <QMetaEnum>
#include <QQmlEngine>
#include <QGuiApplication>

#ifdef Q_OS_MAC
#include "systemtextinputmanager_macos.h"
#endif

#ifdef Q_OS_WIN
#include "systemtextinputmanager_windows.h"
#endif

SystemTextInputManager *SystemTextInputManager::instance()
{
    static SystemTextInputManager *theInstance = nullptr;
    if (theInstance == nullptr) {
        qmlRegisterUncreatableType<AbstractSystemTextInputSource>(
                "Scrite", 1, 0, "TextInputSource",
                "Use from app.textInputManager method return values");
        theInstance = new SystemTextInputManager(qApp);
        theInstance->reload();
    }

    return theInstance;
}

SystemTextInputManager::SystemTextInputManager(QObject *parent) : QAbstractListModel(parent)
{
#ifdef Q_OS_MAC
    m_backend = new SystemTextInputManagerBackend_macOS(this);
#endif

#ifdef Q_OS_WIN
    m_backend = new SystemTextInputManagerBackend_Windows(this);
#endif

    qApp->installEventFilter(this);
}

SystemTextInputManager::~SystemTextInputManager()
{
    // In anycase, this object will be removed from filter lists during destruction.
    // qApp->installEventFilter(this);

    if (m_defaultInputSource != nullptr)
        m_defaultInputSource->select();

    this->clear();

    delete m_backend;
    m_backend = nullptr;
}

QList<AbstractSystemTextInputSource *>
SystemTextInputManager::sourcesForLanguage(int language) const
{
    QList<AbstractSystemTextInputSource *> ret;
    for (AbstractSystemTextInputSource *source : m_inputSources) {
        if (source->language() == language)
            ret.append(source);
    }

    return ret;
}

QJsonArray SystemTextInputManager::sourcesForLanguageJson(int language) const
{
    QJsonArray ret;
    const QList<AbstractSystemTextInputSource *> sources = this->sourcesForLanguage(language);
    for (AbstractSystemTextInputSource *source : sources) {
        QJsonObject item;
        item.insert("id", source->id());
        item.insert("title", source->title());
        ret.append(item);
    }

    return ret;
}

AbstractSystemTextInputSource *SystemTextInputManager::findSourceById(const QString &id) const
{
    for (AbstractSystemTextInputSource *source : m_inputSources) {
        if (source->id() == id)
            return source;
    }

    return nullptr;
}

AbstractSystemTextInputSource *
SystemTextInputManager::fallbackInputSource(int fallbackLanguage) const
{
    AbstractSystemTextInputSource *fallbackSource = this->defaultInputSource();
    if (fallbackSource == nullptr || fallbackSource->language() != fallbackLanguage) {
        QList<AbstractSystemTextInputSource *> fallbackSources =
                this->sourcesForLanguage(fallbackLanguage);
        if (!fallbackSources.isEmpty())
            fallbackSource = fallbackSources.first();
    }

    return fallbackSource;
}

void SystemTextInputManager::reload()
{
    this->clear();

    if (m_backend != nullptr) {
        const QList<AbstractSystemTextInputSource *> sources = m_backend->reloadSources();
        for (AbstractSystemTextInputSource *source : sources)
            this->add(source);

        m_backend->determineSelectedInputSource();

        this->setDefault(this->selectedInputSource());
    }
}

AbstractSystemTextInputSource *SystemTextInputManager::sourceAt(int index) const
{
    if (index < 0 || index >= m_inputSources.size())
        return nullptr;

    return m_inputSources.at(index);
}

int SystemTextInputManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_inputSources.size();
}

QVariant SystemTextInputManager::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() <= m_inputSources.size()) {
        AbstractSystemTextInputSource *source = m_inputSources.at(index.row());
        switch (role) {
        case TextInputSourceRole:
            return QVariant::fromValue<QObject *>(source);
        case TextInputSourceIDRole:
            return source->id();
        case TextInputSourceDisplayNameRole:
            return source->displayName();
        case TextInputSourceSelectedRole:
            return source->isSelected();
        case TextInputSourceLanguageRole:
            return source->language();
        case TextInputSourceLanguageAsStringRole: {
            if (source->language() < 0)
                return QStringLiteral("Unknown");
            const QMetaEnum metaEnum = QMetaEnum::fromType<TransliterationEngine::Language>();
            return QString::fromLatin1(metaEnum.valueToKey(source->language()));
        }
        case Qt::DisplayRole:
            return source->displayName() + QStringLiteral(" [") + source->id()
                    + QStringLiteral("]: ")
                    + (source->isSelected() ? QStringLiteral("ACTIVE") : QString());
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
    roles[TextInputSourceLanguageRole] = "tisLanguage";
    roles[TextInputSourceLanguageAsStringRole] = "tisLanguageAsString";
    return roles;
}

bool SystemTextInputManager::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ApplicationStateChange && object == qApp) {
        if (qApp->applicationState() != Qt::ApplicationActive) {
            if (m_defaultInputSource != nullptr)
                m_defaultInputSource->select();
        } else {
            AbstractSystemTextInputSource *source =
                    this->findSourceById(m_revertToInputSourceUponActivation);
            if (source != nullptr)
                source->select();
        }
    }

    return false;
}

void SystemTextInputManager::clear()
{
    if (!m_inputSources.isEmpty()) {
        m_selectedInputSource = nullptr;
        emit selectedInputSourceChanged();

        if (m_defaultInputSource != nullptr)
            m_defaultInputSource->select();
        m_defaultInputSource = nullptr;
        emit defaultInputSourceChanged();

        this->beginResetModel();
        QList<AbstractSystemTextInputSource *> sources = m_inputSources;
        m_inputSources.clear();
        while (!sources.isEmpty())
            sources.takeFirst()->deleteLater();
        this->endResetModel();

        emit countChanged();
    }
}

void SystemTextInputManager::add(AbstractSystemTextInputSource *source)
{
    if (source == nullptr || m_inputSources.contains(source))
        return;

    connect(source, &AbstractSystemTextInputSource::aboutToDelete, this,
            &SystemTextInputManager::remove);

    const int index = m_inputSources.size();
    this->beginInsertRows(QModelIndex(), index, index);
    m_inputSources.append(source);
    this->endInsertRows();

    emit countChanged();

    if (source->isSelected())
        this->setSelected(source);
}

void SystemTextInputManager::remove(AbstractSystemTextInputSource *source)
{
    if (source == nullptr || m_inputSources.isEmpty())
        return;

    const int row = m_inputSources.indexOf(source);
    if (row < 0)
        return;

    if (m_selectedInputSource == source)
        this->setSelected(nullptr);

    if (m_defaultInputSource == source)
        this->setDefault(nullptr); // We will have a NULL default for a while

    this->beginRemoveRows(QModelIndex(), row, row);
    m_inputSources.removeAt(row);
    this->endRemoveRows();

    emit countChanged();

    if (m_selectedInputSource == nullptr && m_backend != nullptr)
        m_backend->determineSelectedInputSource();
}

void SystemTextInputManager::setSelected(AbstractSystemTextInputSource *source)
{
    if (m_selectedInputSource == source)
        return;

    if (m_selectedInputSource != nullptr)
        m_selectedInputSource->setSelected(false);

    m_selectedInputSource = source;
    emit selectedInputSourceChanged();

    if (m_selectedInputSource != nullptr && qApp->applicationState() == Qt::ApplicationActive)
        m_revertToInputSourceUponActivation = m_selectedInputSource->id();

    const QModelIndex firstRow = this->index(0);
    const QModelIndex lastRow = this->index(m_inputSources.size() - 1);
    emit dataChanged(firstRow, lastRow);
}

void SystemTextInputManager::setDefault(AbstractSystemTextInputSource *dSource)
{
    if (m_defaultInputSource == dSource)
        return;

    m_defaultInputSource = dSource;
    emit defaultInputSourceChanged();
}

///////////////////////////////////////////////////////////////////////////////

AbstractSystemTextInputSource::AbstractSystemTextInputSource(SystemTextInputManager *parent)
    : QObject(parent), m_inputManager(parent)
{
}

AbstractSystemTextInputSource::~AbstractSystemTextInputSource()
{
    emit aboutToDelete(this);
}

void AbstractSystemTextInputSource::setSelected(bool val)
{
    if (m_selected == val)
        return;

    m_selected = val;
    emit selectedChanged();

    if (val)
        m_inputManager->setSelected(this);
}

///////////////////////////////////////////////////////////////////////////////

AbstractSystemTextInputManagerBackend::AbstractSystemTextInputManagerBackend(
        SystemTextInputManager *parent)
    : QObject(parent), m_inputManager(parent)
{
}

AbstractSystemTextInputManagerBackend::~AbstractSystemTextInputManagerBackend() { }

void AbstractSystemTextInputManagerBackend::determineSelectedInputSource()
{
    const int nrInputSources = this->inputManager()->count();
    if (nrInputSources == 0)
        return;

    for (int i = 0; i < nrInputSources; i++) {
        AbstractSystemTextInputSource *inputSource = this->inputManager()->sourceAt(i);
        if (inputSource)
            inputSource->checkSelection();
    }
}
