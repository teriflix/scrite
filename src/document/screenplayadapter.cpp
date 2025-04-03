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

#include "screenplay.h"
#include "application.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "screenplayadapter.h"

ScreenplayAdapter::ScreenplayAdapter(QObject *parent)
    : QIdentityProxyModel(parent),
      m_source(this, "source"),
      m_currentElement(this, "currentElement")
{
    connect(this, &ScreenplayAdapter::modelReset, this,
            &ScreenplayAdapter::updateCurrentIndexAndCount);
    connect(this, &ScreenplayAdapter::rowsRemoved, this,
            &ScreenplayAdapter::updateCurrentIndexAndCount);
    connect(this, &ScreenplayAdapter::rowsInserted, this,
            &ScreenplayAdapter::updateCurrentIndexAndCount);

    connect(this, &ScreenplayAdapter::modelAboutToBeReset, this,
            &ScreenplayAdapter::clearCurrentIndex);
    connect(this, &ScreenplayAdapter::rowsAboutToBeRemoved, this,
            &ScreenplayAdapter::clearCurrentIndex);
    connect(this, &ScreenplayAdapter::rowsAboutToBeInserted, this,
            &ScreenplayAdapter::clearCurrentIndex);

    connect(this, &ScreenplayAdapter::sourceChanged, this, &ScreenplayAdapter::wordCountChanged);
    connect(this, &ScreenplayAdapter::sourceChanged, this,
            &ScreenplayAdapter::heightHintsAvailableChanged);
}

ScreenplayAdapter::~ScreenplayAdapter() { }

void ScreenplayAdapter::setSource(QObject *val)
{
    if (m_source == val)
        return;

    if (this->sourceModel() != nullptr) {
        QAbstractItemModel *srcModel = this->sourceModel();
        Screenplay *screenplay = qobject_cast<Screenplay *>(srcModel);
        if (screenplay != nullptr) {
            if (screenplay->parent() == this)
                GarbageCollector::instance()->add(screenplay);
            else {
                disconnect(this, &ScreenplayAdapter::currentIndexChanged, screenplay,
                           &Screenplay::setCurrentElementIndex);
                disconnect(screenplay, &Screenplay::currentElementIndexChanged, this,
                           &ScreenplayAdapter::setCurrentIndex);
                disconnect(screenplay, &Screenplay::hasNonStandardScenesChanged, this,
                           &ScreenplayAdapter::hasNonStandardScenesChanged);
                disconnect(screenplay, &Screenplay::wordCountChanged, this,
                           &ScreenplayAdapter::wordCountChanged);
            }
        }
    }

    this->setCurrentIndex(-1);

    m_source = val;
    m_adapterRowCount = MAX_ELEMENT_COUNT;

    if (m_source != nullptr) {
        Screenplay *screenplay = qobject_cast<Screenplay *>(m_source);
        if (screenplay != nullptr) {
            if (m_initialLoadTreshold < 0 || screenplay->elementCount() <= m_initialLoadTreshold)
                m_adapterRowCount = MAX_ELEMENT_COUNT;
            else {
                m_adapterRowCount = 0;
                screenplay->setCurrentElementIndex(-1);
            }

            connect(this, &ScreenplayAdapter::currentIndexChanged, screenplay,
                    &Screenplay::setCurrentElementIndex);
            connect(screenplay, &Screenplay::currentElementIndexChanged, this,
                    &ScreenplayAdapter::setCurrentIndex);
            connect(screenplay, &Screenplay::hasNonStandardScenesChanged, this,
                    &ScreenplayAdapter::hasNonStandardScenesChanged);
            connect(screenplay, &Screenplay::wordCountChanged, this,
                    &ScreenplayAdapter::wordCountChanged);
            connect(screenplay, &Screenplay::heightHintsAvailableChanged, this,
                    &ScreenplayAdapter::heightHintsAvailableChanged);

            this->setSourceModel(screenplay);
        } else {
            Scene *scene = qobject_cast<Scene *>(m_source);
            if (scene != nullptr) {
                Screenplay *masterScreenplay = ScriteDocument::instance()->screenplay();

                screenplay = new Screenplay(this);
                if (masterScreenplay != nullptr) {
                    screenplay->setEmail(masterScreenplay->email());
                    screenplay->setTitle(masterScreenplay->title());
                    screenplay->setAuthor(masterScreenplay->author());
                    screenplay->setAddress(masterScreenplay->address());
                    screenplay->setBasedOn(masterScreenplay->basedOn());
                    screenplay->setContact(masterScreenplay->contact());
                    screenplay->setVersion(masterScreenplay->version());
                    screenplay->setSubtitle(masterScreenplay->subtitle());
                    screenplay->setPhoneNumber(masterScreenplay->phoneNumber());
                    screenplay->setProperty("#useDocumentScreenplayForCoverPagePhoto", true);
                }

                ScreenplayElement *element = new ScreenplayElement(screenplay);
                element->setScene(scene);
                screenplay->addElement(element);

                connect(screenplay, &Screenplay::hasNonStandardScenesChanged, this,
                        &ScreenplayAdapter::hasNonStandardScenesChanged);
                connect(screenplay, &Screenplay::wordCountChanged, this,
                        &ScreenplayAdapter::wordCountChanged);
                connect(screenplay, &Screenplay::heightHintsAvailableChanged, this,
                        &ScreenplayAdapter::heightHintsAvailableChanged);

                this->setSourceModel(screenplay);
            } else
                this->setSourceModel(nullptr);
        }
    } else
        this->setSourceModel(nullptr);

    emit sourceChanged();

    if (this->isSourceScreenplay())
        this->setCurrentIndex(this->screenplay()->currentElementIndex());
    else
        this->setCurrentIndex(0);
}

bool ScreenplayAdapter::isSourceScene() const
{
    return qobject_cast<Scene *>(m_source) != nullptr;
}

bool ScreenplayAdapter::isSourceScreenplay() const
{
    return qobject_cast<Screenplay *>(m_source) != nullptr;
}

Screenplay *ScreenplayAdapter::screenplay() const
{
    return qobject_cast<Screenplay *>(this->sourceModel());
}

void ScreenplayAdapter::setCurrentIndex(int val)
{
    this->setCurrentIndexInternal(val);
    emit currentIndexChanged(m_currentIndex);
}

Scene *ScreenplayAdapter::currentScene() const
{
    return m_currentElement == nullptr ? nullptr : m_currentElement->scene();
}

int ScreenplayAdapter::elementCount() const
{
    const int nrElements =
            m_source.isNull() || this->sourceModel() == nullptr ? 0 : this->rowCount(QModelIndex());
    return qMax(nrElements, 0);
}

bool ScreenplayAdapter::hasNonStandardScenes() const
{
    Screenplay *screenplay = this->screenplay();
    if (screenplay != nullptr)
        return screenplay->hasNonStandardScenes();

    return false;
}

int ScreenplayAdapter::wordCount() const
{
    Screenplay *screenplay = this->screenplay();
    return screenplay == nullptr ? 0 : screenplay->wordCount();
}

bool ScreenplayAdapter::isHeightHintsAvailable() const
{
    Screenplay *screenplay = this->screenplay();
    return screenplay == nullptr ? true : screenplay->isHeightHintsAvailable();
}

void ScreenplayAdapter::setInitialLoadTreshold(int val)
{
    if (m_initialLoadTreshold == val)
        return;

    m_initialLoadTreshold = val;
    emit initialLoadTresholdChanged();
}

ScreenplayElement *ScreenplayAdapter::splitElement(ScreenplayElement *ptr, SceneElement *element,
                                                   int textPosition)
{
    Screenplay *screenplay = qobject_cast<Screenplay *>(m_source);
    if (screenplay != nullptr)
        return screenplay->splitElement(ptr, element, textPosition);

    return nullptr;
}

ScreenplayElement *ScreenplayAdapter::mergeElementWithPrevious(ScreenplayElement *ptr)
{
    Screenplay *screenplay = qobject_cast<Screenplay *>(m_source);
    if (screenplay != nullptr)
        return screenplay->mergeElementWithPrevious(ptr);

    return nullptr;
}

int ScreenplayAdapter::previousSceneElementIndex()
{
    Screenplay *screenplay = qobject_cast<Screenplay *>(m_source);
    if (screenplay != nullptr)
        return screenplay->previousSceneElementIndex();

    return 0;
}

int ScreenplayAdapter::nextSceneElementIndex()
{
    Screenplay *screenplay = qobject_cast<Screenplay *>(m_source);
    if (screenplay != nullptr)
        return screenplay->nextSceneElementIndex();

    return 0;
}

QVariant ScreenplayAdapter::at(int row) const
{
    const QModelIndex index = this->index(row, 0);
    if (index.isValid())
        return this->data(index, ModelDataRole);

    return QVariant();
}

void ScreenplayAdapter::refresh()
{
    this->beginResetModel();
    this->endResetModel();
}

QHash<int, QByteArray> ScreenplayAdapter::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[IdRole] = "id";
        roles[SceneRole] = "scene";
        roles[BreakTypeRole] = "breakType";
        roles[ModelDataRole] = "modelData";
        roles[ScreenplayElementRole] = "screenplayElement";
        roles[ScreenplayElementTypeRole] = "screenplayElementType";
    }

    return roles;
}

QVariant ScreenplayAdapter::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || this->sourceModel() == nullptr)
        return QVariant();

    const QModelIndex srcIndex = this->mapToSource(index);
    const QVariant srcData = this->sourceModel()->data(srcIndex, Screenplay::ScreenplayElementRole);
    QObject *elementObject = srcData.value<QObject *>();
    ScreenplayElement *element = qobject_cast<ScreenplayElement *>(elementObject);
    return this->data(element, index.row(), role);
}

int ScreenplayAdapter::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    const Screenplay *screenplay = this->screenplay();
    return m_source.isNull() || screenplay == nullptr
            ? 0
            : qMin(screenplay->elementCount(), m_adapterRowCount);
}

void ScreenplayAdapter::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    if (this->isSourceScreenplay()) {
        const Screenplay *screenplay = this->screenplay();
        const int delta = m_initialLoadTreshold < 0 ? MAX_ELEMENT_COUNT : m_initialLoadTreshold;
        const int start = qMax(0, m_adapterRowCount);
        const int end = qMin(screenplay->elementCount() - 1, start + delta - 1);
        if (end >= start) {
            this->beginInsertRows(QModelIndex(), start, end);
            m_adapterRowCount = end >= screenplay->elementCount() - 1 ? MAX_ELEMENT_COUNT : end + 1;
            this->endInsertRows();

            if (m_adapterRowCount < MAX_ELEMENT_COUNT && m_fetchMoreTimer.isNull()) {
                m_fetchMoreTimer = new QTimer(this);
                connect(m_fetchMoreTimer, &QTimer::timeout, this,
                        &ScreenplayAdapter::continueFetchingMore);
                m_fetchMoreTimer->setInterval(500);
                m_fetchMoreTimer->setSingleShot(false);
                m_fetchMoreTimer->start();
            }
        }
    }
}

bool ScreenplayAdapter::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    if (this->isSourceScreenplay())
        return m_adapterRowCount < MAX_ELEMENT_COUNT;

    return false;
}

void ScreenplayAdapter::setCurrentIndexInternal(int val)
{
    if (m_currentIndex < 0 && val < 0)
        return;

    if (m_source.isNull()) {
        m_currentIndex = -1;
        this->setCurrentElement(nullptr);
        this->setSourceModel(nullptr);
        return;
    }

    const int nrRows = this->elementCount();
    val = nrRows > 0 ? qBound(-1, val, nrRows - 1) : -1;
    if (m_currentIndex == val)
        return;

    m_currentIndex = val;

    if (m_currentIndex >= 0) {
        const QModelIndex index = this->index(m_currentIndex, 0);
        ScreenplayElement *element =
                this->data(index, ScreenplayElementRole).value<ScreenplayElement *>();
        this->setCurrentElement(element);
    } else
        this->setCurrentElement(nullptr);
}

void ScreenplayAdapter::setCurrentElement(ScreenplayElement *val)
{
    if (m_currentElement == val)
        return;

    m_currentElement = val;
    emit currentElementChanged();
}

QVariant ScreenplayAdapter::data(ScreenplayElement *element, int row, int role) const
{
    if (element == nullptr)
        return QVariant();

    switch (role) {
    case IdRole:
        return element->sceneID();
    case ScreenplayElementRole:
        return QVariant::fromValue<ScreenplayElement *>(element);
    case ScreenplayElementTypeRole:
        return element->elementType();
    case BreakTypeRole:
        return element->breakType();
    case SceneRole:
        return QVariant::fromValue<Scene *>(element->scene());
    case ModelDataRole: {
        QVariantMap ret;
        const QHash<int, QByteArray> roles = this->roleNames();
        QHash<int, QByteArray>::const_iterator it = roles.begin();
        QHash<int, QByteArray>::const_iterator end = roles.end();
        while (it != end) {
            if (it.key() != ModelDataRole)
                ret[QString::fromLatin1(it.value())] = this->data(element, row, it.key());
            ++it;
        }
        return ret;
    }
    default:
        break;
    }

    return QVariant();
}

void ScreenplayAdapter::clearCurrentIndex()
{
    this->setCurrentIndexInternal(-1);
}

void ScreenplayAdapter::continueFetchingMore()
{
    if (this->sender() != m_fetchMoreTimer || m_fetchMoreTimer.isNull())
        return;

    if (this->canFetchMore(QModelIndex())) {
        this->fetchMore(QModelIndex());
        m_fetchMoreTimer->setInterval(10);
    } else {
        m_fetchMoreTimer->stop();
        m_fetchMoreTimer->deleteLater();
    }
}

void ScreenplayAdapter::updateCurrentIndexAndCount()
{
    Screenplay *sp = this->screenplay();
    if (sp != nullptr)
        this->setCurrentIndexInternal(sp->currentElementIndex());
    else
        this->setCurrentIndexInternal(-1);

    emit elementCountChanged();
}

void ScreenplayAdapter::resetSource()
{
    this->setSourceModel(nullptr);

    m_source = nullptr;
    emit sourceChanged();

    m_currentElement = nullptr;
    emit currentElementChanged();

    m_currentIndex = -1;
    emit currentIndexChanged(-1);
}
