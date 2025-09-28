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
}

ScreenplayAdapter::~ScreenplayAdapter() { }

void ScreenplayAdapter::setSource(QObject *val)
{
    if (m_source == val)
        return;

    m_source = val;
    QTimer::singleShot(0, this, &ScreenplayAdapter::sourceChanged);

    if (val == nullptr) {
        this->setSourceModel(nullptr);
        return;
    }

    if (val->metaObject()->inherits(&Screenplay::staticMetaObject)) {
        this->setSourceModel(qobject_cast<Screenplay *>(val));
        return;
    }

    if (val->metaObject()->inherits(&Scene::staticMetaObject)) {
        const Screenplay *masterScreenplay = ScriteDocument::instance()->screenplay();

        Screenplay *sceneScreenplay = new Screenplay(this);
        if (masterScreenplay != nullptr) {
            sceneScreenplay->setEmail(masterScreenplay->email());
            sceneScreenplay->setTitle(masterScreenplay->title());
            sceneScreenplay->setAuthor(masterScreenplay->author());
            sceneScreenplay->setAddress(masterScreenplay->address());
            sceneScreenplay->setBasedOn(masterScreenplay->basedOn());
            sceneScreenplay->setContact(masterScreenplay->contact());
            sceneScreenplay->setVersion(masterScreenplay->version());
            sceneScreenplay->setSubtitle(masterScreenplay->subtitle());
            sceneScreenplay->setPhoneNumber(masterScreenplay->phoneNumber());
            sceneScreenplay->setProperty("#useDocumentScreenplayForCoverPagePhoto", true);
        }

        Scene *scene = qobject_cast<Scene *>(val);
        ScreenplayElement *element = new ScreenplayElement(sceneScreenplay);
        element->setScene(scene);
        sceneScreenplay->addElement(element);

        this->setSourceModel(sceneScreenplay);
        return;
    }

    // Ignore all other sources
    m_source = nullptr;
    this->setSourceModel(nullptr);
}

void ScreenplayAdapter::setSourceModel(QAbstractItemModel *model)
{
    QAbstractItemModel *currentSourceModel = this->sourceModel();
    if (currentSourceModel) {
        disconnect(currentSourceModel, nullptr, this, nullptr);
        if (currentSourceModel->parent() == this)
            currentSourceModel->deleteLater();
        currentSourceModel = nullptr;
    }

    if (model && model->metaObject()->inherits(&Screenplay::staticMetaObject)) {
        QIdentityProxyModel::setSourceModel(model);

        Screenplay *screenplay = qobject_cast<Screenplay *>(model);
        connect(screenplay, &Screenplay::elementCountChanged, this,
                &ScreenplayAdapter::elementCountChanged);
        connect(screenplay, &Screenplay::currentElementIndexChanged, this,
                &ScreenplayAdapter::currentIndexChanged);
        connect(screenplay, &Screenplay::hasNonStandardScenesChanged, this,
                &ScreenplayAdapter::hasNonStandardScenesChanged);
        connect(screenplay, &Screenplay::wordCountChanged, this,
                &ScreenplayAdapter::wordCountChanged);
        connect(screenplay, &Screenplay::heightHintsAvailableChanged, this,
                &ScreenplayAdapter::heightHintsAvailableChanged);

    } else
        QIdentityProxyModel::setSourceModel(nullptr);

    emit hasNonStandardScenesChanged();
    emit wordCountChanged();
    emit heightHintsAvailableChanged();

    emit currentIndexChanged();
    emit elementCountChanged();
}

bool ScreenplayAdapter::isSourceScene() const
{
    return m_source && m_source->metaObject()->inherits(&Scene::staticMetaObject);
}

bool ScreenplayAdapter::isSourceScreenplay() const
{
    return m_source && m_source->metaObject()->inherits(&Screenplay::staticMetaObject);
}

Screenplay *ScreenplayAdapter::screenplay() const
{
    return qobject_cast<Screenplay *>(this->sourceModel());
}

void ScreenplayAdapter::setCurrentIndex(int val)
{
    Screenplay *screenplay = this->screenplay();
    if (screenplay)
        screenplay->setCurrentElementIndex(val);
}

int ScreenplayAdapter::currentIndex() const
{
    Screenplay *screenplay = this->screenplay();
    return screenplay ? screenplay->currentElementIndex() : -1;
}

ScreenplayElement *ScreenplayAdapter::currentElement() const
{
    const Screenplay *screenplay = this->screenplay();
    const int currentIndex = screenplay ? screenplay->currentElementIndex() : -1;
    ScreenplayElement *currentElement =
            currentIndex >= 0 ? screenplay->elementAt(currentIndex) : nullptr;
    return currentElement;
}

Scene *ScreenplayAdapter::currentScene() const
{
    const ScreenplayElement *currentElement = this->currentElement();
    return currentElement ? currentElement->scene() : nullptr;
}

bool ScreenplayAdapter::hasNonStandardScenes() const
{
    Screenplay *screenplay = this->screenplay();
    return screenplay ? screenplay->hasNonStandardScenes() : false;
}

int ScreenplayAdapter::wordCount() const
{
    Screenplay *screenplay = this->screenplay();
    return screenplay ? screenplay->wordCount() : 0;
}

bool ScreenplayAdapter::isHeightHintsAvailable() const
{
    Screenplay *screenplay = this->screenplay();
    return screenplay ? screenplay->isHeightHintsAvailable() : true;
}

ScreenplayElement *ScreenplayAdapter::splitElement(ScreenplayElement *screenplayElement,
                                                   SceneElement *paragraph, int textPosition)
{
    Screenplay *screenplay = this->isSourceScreenplay() ? this->screenplay() : nullptr;
    if (screenplay)
        return screenplay->splitElement(screenplayElement, paragraph, textPosition);

    return nullptr;
}

ScreenplayElement *ScreenplayAdapter::mergeElementWithPrevious(ScreenplayElement *screenplayElement)
{
    Screenplay *screenplay = this->isSourceScreenplay() ? this->screenplay() : nullptr;
    if (screenplay)
        return screenplay->mergeElementWithPrevious(screenplayElement);

    return nullptr;
}

int ScreenplayAdapter::firstSceneElementIndex() const
{
    Screenplay *screenplay = this->screenplay();
    if (screenplay != nullptr)
        return screenplay->firstSceneElementIndex();

    return 0;
}

int ScreenplayAdapter::lastSceneElementIndex() const
{
    Screenplay *screenplay = this->screenplay();
    if (screenplay != nullptr)
        return screenplay->lastSceneElementIndex();

    return 0;
}

int ScreenplayAdapter::previousSceneElementIndex() const
{
    Screenplay *screenplay = this->screenplay();
    if (screenplay != nullptr)
        return screenplay->previousSceneElementIndex();

    return 0;
}

int ScreenplayAdapter::nextSceneElementIndex() const
{
    Screenplay *screenplay = this->screenplay();
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
        roles[IdRole] = "sceneID";
        roles[SceneRole] = "scene";
        roles[BreakTypeRole] = "breakType";
        roles[ModelDataRole] = "modelData";
        roles[DelegateKindRole] = "delegateKind";
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
    case DelegateKindRole:
        return element->delegateKind();
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

void ScreenplayAdapter::resetSource()
{
    this->setSourceModel(nullptr);

    m_source = nullptr;
    emit sourceChanged();
}
