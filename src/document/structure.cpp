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

#include "scrite.h"
#include "undoredo.h"
#include "fountain.h"
#include "structure.h"
#include "hourglass.h"
#include "filemanager.h"
#include "application.h"
#include "deltadocument.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "structureexporter.h"
#include "screenplaytextdocument.h"

#include <QDir>
#include <QtMath>
#include <QStack>
#include <QBuffer>
#include <QJSValue>
#include <QMimeData>
#include <QDateTime>
#include <QJSEngine>
#include <QTextList>
#include <QTextTable>
#include <QTextFrame>
#include <QClipboard>
#include <QScopeGuard>
#include <QQuickWindow>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QFileSystemWatcher>
#include <QScopedValueRollback>

StructureElement::StructureElement(QObject *parent)
    : QObject(parent), m_structure(qobject_cast<Structure *>(parent)), m_follow(this, "follow")
{
    connect(this, &StructureElement::xChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::xChanged, this, &StructureElement::positionChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::positionChanged);
    connect(this, &StructureElement::xChanged, this, &StructureElement::xfChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::yfChanged);
    connect(this, &StructureElement::widthChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::heightChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::stackIdChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::titleChanged, this, &StructureElement::elementChanged);

    connect(this, &StructureElement::xChanged, this, &StructureElement::geometryChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::geometryChanged);
    connect(this, &StructureElement::widthChanged, this, &StructureElement::geometryChanged);
    connect(this, &StructureElement::heightChanged, this, &StructureElement::geometryChanged);

    if (m_structure) {
        connect(m_structure, &Structure::canvasWidthChanged, this, &StructureElement::xfChanged);
        connect(m_structure, &Structure::canvasHeightChanged, this, &StructureElement::yfChanged);
        connect(m_structure, &Structure::groupsModelChanged, this,
                &StructureElement::groupVerificationRequired);
    }

    connect(this, &StructureElement::sceneHeadingChanged, [=]() {
        if (m_title.isEmpty())
            emit titleChanged();
    });
}

StructureElement::~StructureElement()
{
    emit aboutToDelete(this);
}

StructureElement *StructureElement::duplicate()
{
    if (m_structure == nullptr)
        return nullptr;

    const int newIndex = m_structure->indexOfElement(this) + 1;

    StructureElement *newElement = new StructureElement(m_structure);
    newElement->setScene(m_scene->clone(newElement));
    newElement->setX(m_x);
    newElement->setY(m_y + (m_height > 0 ? m_height + 100 : 200));
    m_structure->insertElement(newElement, newIndex);

    return newElement;
}

void StructureElement::setX(qreal val)
{
    if (qFuzzyCompare(m_x, val))
        return;

    if (!m_placed)
        m_placed = !qFuzzyIsNull(m_x) && !qFuzzyIsNull(m_y);

    if (m_undoRedoEnabled) {
        ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "position");
        QScopedPointer<PushObjectPropertyUndoCommand> cmd;
        if (!info->isLocked())
            cmd.reset(new PushObjectPropertyUndoCommand(this, info->property, m_placed));
    }

    m_x = val;
    emit xChanged();
}

void StructureElement::setY(qreal val)
{
    if (qFuzzyCompare(m_y, val))
        return;

    if (!m_placed)
        m_placed = !qFuzzyIsNull(m_x) && !qFuzzyIsNull(m_y);

    if (m_undoRedoEnabled) {
        ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "position");
        QScopedPointer<PushObjectPropertyUndoCommand> cmd;
        if (!info->isLocked())
            cmd.reset(new PushObjectPropertyUndoCommand(this, info->property, m_placed));
    }

    m_y = val;
    emit yChanged();
}

void StructureElement::setWidth(qreal val)
{
    if (qFuzzyCompare(m_width, val))
        return;

    m_width = val;
    emit widthChanged();
}

void StructureElement::setHeight(qreal val)
{
    if (qFuzzyCompare(m_height, val))
        return;

    m_height = val;
    emit heightChanged();
}

void StructureElement::setFollow(QQuickItem *val)
{
    if (m_follow == val)
        return;

    if (m_follow != nullptr) {
        disconnect(m_follow, &QQuickItem::xChanged, this, &StructureElement::syncWithFollowItem);
        disconnect(m_follow, &QQuickItem::yChanged, this, &StructureElement::syncWithFollowItem);
        disconnect(m_follow, &QQuickItem::widthChanged, this,
                   &StructureElement::syncWithFollowItem);
        disconnect(m_follow, &QQuickItem::heightChanged, this,
                   &StructureElement::syncWithFollowItem);
    }

    m_follow = val;

    if (m_follow) {
        this->setWidth(m_follow->width());
        this->setHeight(m_follow->height());
        connect(m_follow, &QQuickItem::xChanged, this, &StructureElement::syncWithFollowItem);
        connect(m_follow, &QQuickItem::yChanged, this, &StructureElement::syncWithFollowItem);
        connect(m_follow, &QQuickItem::widthChanged, this, &StructureElement::syncWithFollowItem);
        connect(m_follow, &QQuickItem::heightChanged, this, &StructureElement::syncWithFollowItem);
    }

    emit followChanged();

    ExecLaterTimer::call(
            "StructureElement::setSyncWithFollow", this, [=]() { this->setSyncWithFollow(true); },
            250);
}

void StructureElement::setUndoRedoEnabled(bool val)
{
    if (m_undoRedoEnabled == val)
        return;

    m_undoRedoEnabled = val;
    emit undoRedoEnabledChanged();
}

void StructureElement::setSyncWithFollow(bool val)
{
    if (m_syncWithFollow == val)
        return;

    m_syncWithFollow = val;
    emit syncWithFollowChanged();

    if (m_syncWithFollow)
        this->syncWithFollowItem();
}

void StructureElement::resetFollow()
{
    m_follow = nullptr;
    emit followChanged();
}

void StructureElement::setXf(qreal val)
{
    if (m_structure == nullptr)
        return;

    val = qBound(0.0, val, 1.0);
    this->setX(m_structure->canvasWidth() * val);
}

qreal StructureElement::xf() const
{
    return m_structure == nullptr ? 0 : m_x / m_structure->canvasWidth();
}

void StructureElement::setYf(qreal val)
{
    if (m_structure == nullptr)
        return;

    val = qBound(0.0, val, 1.0);
    this->setY(m_structure->canvasHeight() * val);
}

qreal StructureElement::yf() const
{
    return m_structure == nullptr ? 0 : m_y / m_structure->canvasHeight();
}

void StructureElement::setPosition(const QPointF &pos)
{
    if (QPointF(m_x, m_y) == pos)
        return;

    this->setX(pos.x());
    this->setY(pos.y());
}

void StructureElement::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;

    if (m_scene != nullptr) {
        SceneHeading *heading = m_scene->heading();
        if (heading != nullptr && heading->isEnabled()) {
            QString _locationType, _location, _moment;
            if (SceneHeading::parse(val, _locationType, _location, _moment, true)) {
                heading->setLocationType(_locationType);
                heading->setLocation(_location);
                heading->setMoment(_moment);

                m_title.clear();
            }
        }
    }

    emit titleChanged();
}

QString StructureElement::title() const
{
    if (!m_title.isEmpty())
        return m_title;

    return m_scene->heading()->isEnabled() ? m_scene->heading()->text() : QString();
}

void StructureElement::setScene(Scene *val)
{
    if (m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    m_scene->setStructureElement(this);
    connect(m_scene, &Scene::sceneChanged, this, &StructureElement::elementChanged);
    connect(m_scene, &Scene::aboutToDelete, this, &StructureElement::deleteLater);

    connect(m_scene->heading(), &SceneHeading::enabledChanged, this,
            &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::locationTypeChanged, this,
            &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::locationChanged, this,
            &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::momentChanged, this,
            &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::locationChanged, this,
            &StructureElement::sceneLocationChanged);

    emit sceneChanged();
}

void StructureElement::setSelected(bool val)
{
    if (m_selected == val)
        return;

    m_selected = val;
    emit selectedChanged();
}

void StructureElement::setStackId(const QString &val)
{
    QString val2 = val;

    if (!val2.isEmpty()) {
        ScriteDocument *doc = m_structure ? m_structure->scriteDocument() : nullptr;
        if (doc && !doc->isLoading()) {
            Screenplay *screenplay = doc ? doc->screenplay() : nullptr;
            if (screenplay) {
                if (screenplay->firstIndexOfScene(m_scene) < 0)
                    val2.clear();
            }
        }
    }

    if (m_stackId == val2)
        return;

    m_stackId = val2;
    this->setStackLeader(false);

    emit stackIdChanged();
}

void StructureElement::setStackLeader(bool val)
{
    if (m_stackLeader == val)
        return;

    m_stackLeader = val;
    emit stackLeaderChanged();
}

void StructureElement::unstack()
{
    if (m_stackId.isEmpty())
        return;

    auto clearStackId = qScopeGuard([=] { this->setStackId(QString()); });
    Q_UNUSED(clearStackId)

    if (m_structure == nullptr)
        return;

    const StructureElementStack *stack = m_structure->elementStacks()->findStackById(m_stackId);
    if (stack == nullptr)
        return;

    const int elementIndex = stack->constList().indexOf(this);
    if (elementIndex == 0 || elementIndex == stack->constList().size() - 1)
        return;

    StructureElementStack::stackEm(stack->constList().mid(0, elementIndex));
    StructureElementStack::stackEm(stack->constList().mid(elementIndex + 1));
}

void StructureElement::serializeToJson(QJsonObject &json) const
{
    if (m_scene != nullptr && m_scene->heading() != nullptr && m_scene->heading()->isEnabled()
        && m_scene->heading()->text() == this->title())
        json.remove(QStringLiteral("title"));
}

bool StructureElement::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        if (m_structure) {
            disconnect(m_structure, &Structure::canvasWidthChanged, this,
                       &StructureElement::xfChanged);
            disconnect(m_structure, &Structure::canvasHeightChanged, this,
                       &StructureElement::yfChanged);
        }

        m_structure = qobject_cast<Structure *>(this->parent());

        if (m_structure) {
            connect(m_structure, &Structure::canvasWidthChanged, this,
                    &StructureElement::xfChanged);
            connect(m_structure, &Structure::canvasHeightChanged, this,
                    &StructureElement::yfChanged);
            connect(m_structure, &Structure::groupsModelChanged, this,
                    &StructureElement::groupVerificationRequired);
        }

        emit xfChanged();
        emit yfChanged();
    }

    return QObject::event(event);
}

void StructureElement::syncWithFollowItem()
{
    if (m_follow.isNull() || !m_syncWithFollow)
        return;

    this->setX(m_follow->x());
    this->setY(m_follow->y());
    this->setWidth(m_follow->width());
    this->setHeight(m_follow->height());
}

void StructureElement::groupVerificationRequired()
{
    if (m_scene && m_structure)
        m_scene->verifyGroups(m_structure->groupsModel());
}

void StructureElement::renameCharacter(const QString &from, const QString &to)
{
    if (!m_title.isEmpty()) {
        int nrReplacements = 0;
        const QString newTitle =
                Application::replaceCharacterName(from, to, m_title, &nrReplacements);
        if (nrReplacements > 0) {
            m_title = newTitle;
            emit titleChanged();
        }
    }

    m_scene->renameCharacter(from, to);
}

///////////////////////////////////////////////////////////////////////////////

StructureElementStack::StructureElementStack(QObject *parent)
    : QObjectListModel<StructureElement *>(parent)
{
    StructureElementStacks *stacks = qobject_cast<StructureElementStacks *>(parent);
    if (stacks)
        connect(stacks->structure(), &Structure::currentElementIndexChanged, this,
                &StructureElementStack::onStructureCurrentElementChanged);
}

StructureElementStack::~StructureElementStack()
{
    emit aboutToDelete(this);
}

StructureElement *StructureElementStack::stackLeader() const
{
    for (StructureElement *element : qAsConst(this->list()))
        if (element->isStackLeader())
            return element;

    return this->first();
}

int StructureElementStack::topmostElementIndex() const
{
    return this->indexOf(this->topmostElement());
}

StructureElement *StructureElementStack::topmostElement() const
{
    return m_topmostElement == nullptr ? this->stackLeader() : m_topmostElement;
}

void StructureElementStack::moveToStackId(const QString &stackID)
{
    if (stackID.isEmpty() || stackID == m_stackId)
        return;

    StructureElementStacks *stacks = qobject_cast<StructureElementStacks *>(this->parent());
    if (stacks == nullptr)
        return;

    this->moveToStack(stacks->findStackById(stackID));
}

void StructureElementStack::moveToStack(StructureElementStack *other)
{
    if (other == nullptr || other == this)
        return;

    for (StructureElement *element : qAsConst(this->list()))
        element->setStackId(other->stackId());
}

void StructureElementStack::bringElementToTop(int index)
{
    if (index < 0 || index >= this->list().size())
        return;

    StructureElementStacks *stacks = qobject_cast<StructureElementStacks *>(this->parent());
    if (stacks == nullptr)
        return;

    Structure *structure = stacks->structure();
    if (structure == nullptr)
        return;

    StructureElement *element = this->list().at(index);
    int elementIndex = structure->indexOfElement(element);
    structure->setCurrentElementIndex(elementIndex);
}

void StructureElementStack::sortByScreenplayOccurance(Screenplay *screenplay)
{
    QList<StructureElement *> &list = this->list();

    bool shifted = false;
    std::sort(list.begin(), list.end(),
              [screenplay, &shifted](StructureElement *e1, StructureElement *e2) {
                  const int i1 = screenplay->firstIndexOfScene(e1->scene());
                  const int i2 = screenplay->firstIndexOfScene(e2->scene());
                  e1->setStackLeader(false);
                  e2->setStackLeader(false);
                  if (i1 > i2)
                      shifted = true;
                  return i1 < i2;
              });

    list.first()->setStackLeader(true);
    this->setTopmostElement(nullptr);

    if (shifted) {
        const QModelIndex start = this->index(0);
        const QModelIndex end = this->index(list.size() - 1);
        emit dataChanged(start, end);
    }
}

void StructureElementStack::stackEm(const QList<StructureElement *> &elements)
{
    if (elements.isEmpty())
        return;

    const QString newStackId = elements.size() == 1 ? QString() : QUuid::createUuid().toString();
    for (StructureElement *element : elements)
        element->setStackId(newStackId);
}

void StructureElementStack::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_initializeTimer.timerId()) {
        m_initializeTimer.stop();
        this->initialize();
    } else
        QObjectListModel<StructureElement *>::timerEvent(te);
}

void StructureElementStack::itemInsertEvent(StructureElement *ptr)
{
    if (this->list().size() == 1) {
        m_stackId = ptr->stackId();
        emit stackIdChanged();

        m_actIndex = ptr->scene()->actIndex();
        emit actIndexChanged();
    }

    connect(ptr, &StructureElement::aboutToDelete, this, &StructureElementStack::objectDestroyed);
    connect(ptr, &StructureElement::stackLeaderChanged, this,
            &StructureElementStack::onStackLeaderChanged);
    connect(ptr, &StructureElement::geometryChanged, this,
            &StructureElementStack::onElementGeometryChanged);
    connect(ptr, &StructureElement::followChanged, this,
            &StructureElementStack::onElementFollowSet);

    Scene *scene = ptr->scene();
    if (scene != nullptr) {
        connect(scene, &Scene::colorChanged, this, &StructureElementStack::objectChanged);
        connect(scene, &Scene::groupsChanged, this, &StructureElementStack::onElementGroupChanged);
    } else
        connect(ptr, &StructureElement::elementChanged, this,
                &StructureElementStack::objectChanged);
}

void StructureElementStack::itemRemoveEvent(StructureElement *ptr)
{
    disconnect(ptr, &StructureElement::elementChanged, this, &StructureElementStack::objectChanged);
    disconnect(ptr, &StructureElement::aboutToDelete, this,
               &StructureElementStack::objectDestroyed);
    disconnect(ptr, &StructureElement::stackLeaderChanged, this,
               &StructureElementStack::onStackLeaderChanged);
    disconnect(ptr, &StructureElement::geometryChanged, this,
               &StructureElementStack::onElementGeometryChanged);
    disconnect(ptr, &StructureElement::followChanged, this,
               &StructureElementStack::onElementFollowSet);

    Scene *scene = ptr->scene();
    if (scene != nullptr) {
        disconnect(scene, &Scene::colorChanged, this, &StructureElementStack::objectChanged);
        disconnect(scene, &Scene::groupsChanged, this,
                   &StructureElementStack::onElementGroupChanged);
    }
}

void StructureElementStack::setHasCurrentElement(bool val)
{
    if (m_hasCurrentElement == val)
        return;

    m_hasCurrentElement = val;
    emit hasCurrentElementChanged();
}

void StructureElementStack::setTopmostElement(StructureElement *val)
{
    if (m_topmostElement == val)
        return;

    m_topmostElement = val;
    emit topmostElementChanged();
}

void StructureElementStack::setGeometry(const QRectF &val)
{
    if (m_geometry == val)
        return;

    m_geometry = val;
    emit geometryChanged();
}

void StructureElementStack::initialize()
{
    QScopedValueRollback<bool> rollback(m_enabled, false);

    QRectF geo;
    StructureElement *leader = nullptr;

    QList<StructureElement *> &list = this->list();

    QSet<QString> stackGroups;

    Screenplay *screenplay = nullptr;
    StructureElementStacks *stacks = qobject_cast<StructureElementStacks *>(this->parent());
    if (stacks && stacks->structure() && stacks->structure()->scriteDocument())
        screenplay = stacks->structure()->scriteDocument()->screenplay();
    else
        screenplay = ScriteDocument::instance()->screenplay();

    qreal x = 0, y = 0, w = 350, h = 375;

    for (int i = list.size() - 1; i >= 0; i--) {
        StructureElement *element = list.at(i);
        if (element->stackId() != m_stackId) {
            this->removeAt(i);
            continue;
        }

        if (geo.isValid()) {
            x = qMin(geo.y(), element->y());
            y = qMin(geo.x(), element->x());

            w = qMax(geo.width(), element->width());
            h = qMax(geo.height(), element->height());
        } else {
            x = element->x();
            y = element->y();
            w = qMax(w, element->width());
            h = qMax(h, element->height());
        }

        if (element->isStackLeader()) {
            if (leader == nullptr)
                leader = element;
            else
                element->setStackLeader(false);
        }

        const QStringList elementGroups = element->scene()->groups();
        stackGroups += QSet<QString>(elementGroups.begin(), elementGroups.end());
    }

    if (list.isEmpty()) {
        this->deleteLater();
        return;
    }

    if (list.size() == 1) {
        list.first()->setStackId(QString());
        this->deleteLater();
        return;
    }

    this->setGeometry(QRectF(x, y, w, h));

    if (screenplay != nullptr)
        this->sortByScreenplayOccurance(screenplay);

    const QStringList groups = stackGroups.values();
    for (StructureElement *element : qAsConst(this->list())) {
        element->setX(x);
        element->setY(y);
        element->scene()->setGroups(groups);
    }

    if (leader == nullptr) {
        leader = this->first();
        if (leader != nullptr)
            leader->setStackLeader(true);
    }

    this->onStructureCurrentElementChanged();
}

void StructureElementStack::onElementFollowSet()
{
    m_initializeTimer.start(0, this);
}

void StructureElementStack::onStackLeaderChanged()
{
    if (!m_enabled)
        return;

    QScopedValueRollback<bool> rollback(m_enabled, false);

    if (!this->isEmpty()) {
        StructureElement *changedElement = qobject_cast<StructureElement *>(this->sender());
        if (changedElement->isStackLeader()) {
            for (StructureElement *element : qAsConst(this->list())) {
                if (element != changedElement)
                    element->setStackLeader(false);
            }
        } else {
            for (StructureElement *element : qAsConst(this->list())) {
                if (element->isStackLeader())
                    return;
            }

            this->first()->setStackLeader(true);
        }
    }

    if (m_topmostElement == nullptr)
        emit topmostElementChanged();
}

void StructureElementStack::onElementGroupChanged()
{
    if (!m_enabled)
        return;

    QScopedValueRollback<bool> rollback(m_enabled, false);

    StructureElement *changedElement = qobject_cast<StructureElement *>(this->sender());
    if (changedElement == nullptr) {
        Scene *changedScene = qobject_cast<Scene *>(this->sender());
        if (changedScene == nullptr)
            return;

        StructureElementStacks *stacks = qobject_cast<StructureElementStacks *>(this->parent());
        if (stacks == nullptr)
            return;

        Structure *structure = stacks->structure();
        if (structure == nullptr)
            return;

        int index = structure->indexOfScene(changedScene);
        if (index < 0)
            return;

        changedElement = structure->elementAt(index);
    }

    if (!this->list().contains(changedElement))
        return;

    const QStringList changedGroups = changedElement->scene()->groups();
    for (StructureElement *element : qAsConst(this->list())) {
        if (element != changedElement)
            element->scene()->setGroups(changedGroups);
    }
}

void StructureElementStack::onElementGeometryChanged()
{
    if (!m_geometry.isValid() || !m_enabled)
        return;

    QScopedValueRollback<bool> rollback(m_enabled, false);

    StructureElement *changedElement = qobject_cast<StructureElement *>(this->sender());
    if (changedElement == nullptr || !this->list().contains(changedElement))
        return;

    const qreal dx = changedElement->x() - m_geometry.x();
    const qreal dy = changedElement->y() - m_geometry.y();
    const QPointF dp(dx, dy);
    for (StructureElement *element : qAsConst(this->list())) {
        if (element != changedElement)
            element->setPosition(element->position() + dp);
    }
    m_geometry.moveTopLeft(m_geometry.topLeft() + dp);

    m_geometry.setWidth(qMax(m_geometry.width(), changedElement->width()));
    m_geometry.setHeight(qMax(m_geometry.height(), changedElement->height()));

    emit geometryChanged();
}

void StructureElementStack::onStructureCurrentElementChanged()
{
    StructureElementStacks *stacks = qobject_cast<StructureElementStacks *>(this->parent());
    if (stacks == nullptr)
        return;

    Structure *structure = stacks->structure();
    if (structure == nullptr) {
        this->setHasCurrentElement(false);
        this->setTopmostElement(nullptr);
        return;
    }

    StructureElement *element = structure->elementAt(structure->currentElementIndex());
    this->setHasCurrentElement(this->list().contains(element));
    if (m_hasCurrentElement)
        this->setTopmostElement(element);
    else if (m_topmostElement != nullptr && !this->list().contains(m_topmostElement))
        this->setTopmostElement(nullptr);
}

///////////////////////////////////////////////////////////////////////////////

StructureElementStacks::StructureElementStacks(QObject *parent)
    : QObjectListModel<StructureElementStack *>(parent)
{
}

StructureElementStacks::~StructureElementStacks() { }

StructureElementStack *StructureElementStacks::findStackById(const QString &stackID) const
{
    if (stackID.isEmpty())
        return nullptr;

    for (StructureElementStack *stack : qAsConst(this->list())) {
        if (stack->stackId() == stackID)
            return stack;
    }

    return nullptr;
}

StructureElementStack *StructureElementStacks::findStackByElement(StructureElement *element) const
{
    return element == nullptr ? nullptr : this->findStackById(element->stackId());
}

void StructureElementStacks::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_evaluateTimer.timerId()) {
        m_evaluateTimer.stop();
        this->evaluateStacks();
    } else
        QAbstractListModel::timerEvent(te);
}

void StructureElementStacks::itemInsertEvent(StructureElementStack *ptr)
{
    connect(ptr, &StructureElementStack::aboutToDelete, this,
            &StructureElementStacks::objectDestroyed);
    connect(ptr, &StructureElementStack::stackLeaderChanged, this,
            &StructureElementStacks::objectChanged);
    connect(ptr, &StructureElementStack::geometryChanged, this,
            &StructureElementStacks::objectChanged);
}

void StructureElementStacks::itemRemoveEvent(StructureElementStack *ptr)
{
    disconnect(ptr, &StructureElementStack::aboutToDelete, this,
               &StructureElementStacks::objectDestroyed);
    disconnect(ptr, &StructureElementStack::stackLeaderChanged, this,
               &StructureElementStacks::objectChanged);
    disconnect(ptr, &StructureElementStack::geometryChanged, this,
               &StructureElementStacks::objectChanged);
}

void StructureElementStacks::resetAllStacks()
{
    if (this->isEmpty())
        return;

    for (StructureElementStack *stack : qAsConst(this->list()))
        stack->deleteLater();
}

void StructureElementStacks::evaluateStacks()
{
    if (m_structure->canvasUIMode() != Structure::IndexCardUI) {
        this->resetAllStacks();
        return;
    }

    /**
     * We cannot have a stack that holds elements form multiple acts.
     * In this loop we make sure that we segregate elements from mutliple acts,
     * that may have the same stack Id.
     */
    QList<StructureElement *> elementsWithStackId;
    QMap<QString, QMap<int, QList<StructureElement *>>> elementsMap;
    for (int i = 0; i < m_structure->elementCount(); i++) {
        StructureElement *element = m_structure->elementAt(i);
        const QString stackId = element->stackId();
        if (stackId.isEmpty())
            continue;

        int actIndex = element->scene()->actIndex();
        elementsMap[stackId][actIndex].append(element);
        elementsWithStackId.append(element);
    }

    /**
     * Here we sanitize stack-ids by generating unique ids for those elements
     * that may have the same stack-id but are on different stacks.
     */
    QMap<QString, QMap<int, QList<StructureElement *>>>::iterator it = elementsMap.begin();
    QMap<QString, QMap<int, QList<StructureElement *>>>::iterator end = elementsMap.end();
    while (it != end) {
        if (it.value().size() > 1) {
            QMap<int, QList<StructureElement *>>::iterator it2 = it.value().begin();
            QMap<int, QList<StructureElement *>>::iterator end2 = it.value().end();
            ++it2;

            while (it2 != end2) {
                const QString id = Application::createUniqueId();
                for (StructureElement *element : qAsConst(it2.value()))
                    element->setStackId(id);

                ++it2;
            }
        }

        ++it;
    }

    m_evaluateTimer.stop();

    auto findOrCreateStack = [=](const QString &id) {
        for (StructureElementStack *stack : qAsConst(this->list()))
            if (stack->stackId() == id)
                return stack;
        StructureElementStack *newStack = new StructureElementStack(this);
        this->append(newStack);
        return newStack;
    };

    for (StructureElementStack *stack : qAsConst(this->list()))
        stack->setEnabled(false);

    for (StructureElement *element : qAsConst(elementsWithStackId)) {
        const QString stackId = element->stackId();
        if (stackId.isEmpty())
            continue;

        StructureElementStack *stack = findOrCreateStack(stackId);
        stack->append(element);
    }

    for (StructureElementStack *stack : qAsConst(this->list())) {
        if (stack->isEmpty())
            stack->deleteLater();
        else {
            stack->initialize();
            stack->setEnabled(true);
        }
    }

    if (!this->isEmpty())
        m_structure->setCanvasUIMode(Structure::IndexCardUI);

    if (m_structure->isForceBeatBoardLayout()) {
        /**
         * By instantaneously layouting the elements in the beat-board,
         * we do not allow for follow to be set for sync by the newly created
         * structure element on the UI. This will result in invalid layouting
         */
        QTimer::singleShot(100, this, [=]() {
            Screenplay *screenplay = m_structure->scriteDocument()
                    ? m_structure->scriteDocument()->screenplay()
                    : ScriteDocument::instance()->screenplay();
            m_structure->placeElementsInBeatBoardLayout(screenplay);
        });
    }
}

void StructureElementStacks::evaluateStacksLater()
{
    m_evaluateTimer.start(0, this);
}

void StructureElementStacks::evaluateStacksMuchLater(int howMuchLater)
{
    m_evaluateTimer.start(howMuchLater, this);
}

///////////////////////////////////////////////////////////////////////////////

Relationship::Relationship(QObject *parent) : QObject(parent), m_with(this, "with")
{
    m_of = qobject_cast<Character *>(parent);

    connect(this, &Relationship::ofChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::nameChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::withChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::directionChanged, this, &Relationship::relationshipChanged);
    connect(m_notes, &Notes::notesModified, this, &Relationship::relationshipChanged);
}

Relationship::~Relationship()
{
    emit aboutToDelete(this);
}

void Relationship::setDirection(Relationship::Direction val)
{
    if (m_direction == val)
        return;

    m_direction = val;
    emit directionChanged();
}

QString Relationship::polishName(const QString &val)
{
    return Application::instance()->camelCased(val);
}

void Relationship::setName(const QString &val)
{
    const QString val2 = polishName(val);
    if (m_name == val2)
        return;

    m_name = val2;
    emit nameChanged();

    if (m_with != nullptr) {
        Relationship *rel = m_with->findRelationship(m_of);
        if (rel && rel != this)
            rel->setName(m_name);
    }
}

void Relationship::setWith(Character *val)
{
    if (m_with == val)
        return;

    m_with = val;
    emit withChanged();
}

void Relationship::serializeToJson(QJsonObject &json) const
{
    if (m_with != nullptr)
        json.insert("with", m_with->name());
}

void Relationship::deserializeFromJson(const QJsonObject &json)
{
    m_withName = json.value("with").toString();

    // Previously notes was an array, because the notes property used to be
    // a list property. Now notes is an object, because it represents Notes class.
    // So, if we are loading a notes from a file created using older versions of Scrite,
    // we have to upgrade the notes to the newer format based on the Notes class.
    const QJsonValue notes = json.value(QStringLiteral("notes"));
    if (notes.isArray())
        m_notes->loadOldNotes(notes.toArray());
}

void Relationship::resolveRelationship()
{
    if (!m_withName.isEmpty()) {
        Structure *structure = nullptr;
        if (m_of != nullptr)
            structure = m_of->structure();
        else
            structure = ScriteDocument::instance()->structure();

        m_with = structure->findCharacter(m_withName);
        m_withName.clear();

        if (m_with != nullptr)
            emit withChanged();
        else
            this->deleteLater();
    }
}

bool Relationship::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        if (m_of == nullptr) {
            m_of = qobject_cast<Character *>(this->parent());
            emit ofChanged();
        } else if (m_of != this->parent())
            qFatal("Relationship of a character, once set, cannot be changed.");
    }

    return QObject::event(event);
}

void Relationship::setOf(Character *val)
{
    if (m_of == val)
        return;

    m_of = val;
    emit ofChanged();
}

void Relationship::resetWith()
{
    m_with = nullptr;
    emit withChanged();
}

///////////////////////////////////////////////////////////////////////////////

Character::Character(QObject *parent)
    : QObject(parent), m_structure(qobject_cast<Structure *>(parent))
{
    connect(this, &Character::ageChanged, this, &Character::characterChanged);
    connect(this, &Character::nameChanged, this, &Character::characterChanged);
    connect(this, &Character::typeChanged, this, &Character::characterChanged);
    connect(this, &Character::colorChanged, this, &Character::characterChanged);
    connect(this, &Character::weightChanged, this, &Character::characterChanged);
    connect(this, &Character::photosChanged, this, &Character::characterChanged);
    connect(this, &Character::heightChanged, this, &Character::characterChanged);
    connect(this, &Character::genderChanged, this, &Character::characterChanged);
    connect(this, &Character::aliasesChanged, this, &Character::characterChanged);
    connect(this, &Character::bodyTypeChanged, this, &Character::characterChanged);
    connect(this, &Character::summaryChanged, this, &Character::characterChanged);
    connect(m_notes, &Notes::notesModified, this, &Character::characterChanged);
    connect(this, &Character::designationChanged, this, &Character::characterChanged);
    connect(this, &Character::relationshipCountChanged, this, &Character::characterChanged);
    connect(this, &Character::characterRelationshipGraphChanged, this,
            &Character::characterChanged);
    connect(this, &Character::keyPhotoChanged, this, &Character::characterChanged);
    connect(m_attachments, &Attachments::attachmentsModified, this, &Character::characterChanged);

    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    connect(dfs, &DocumentFileSystem::auction, this, &Character::onDfsAuction);

    connect(this, &Character::photosChanged, this, [=]() {
        const int min = m_photos.isEmpty() ? -1 : 0;
        this->setKeyPhotoIndex(qBound(min, m_keyPhotoIndex, m_photos.size() - 1));
    });

    if (m_structure) {
        connect(this, &Character::nameChanged, m_structure,
                &Structure::updateCharacterNamesShotsTransitionsAndTagsLater);
        connect(this, &Character::tagsChanged, m_structure,
                &Structure::updateCharacterNamesShotsTransitionsAndTagsLater);
        connect(this, &Character::priorityChanged, m_structure,
                &Structure::updateCharacterNamesShotsTransitionsAndTagsLater);
    }
}

Character::~Character() { }

void Character::setName(const QString &val)
{
    if (m_name == val || val.isEmpty() || !m_name.isEmpty())
        return;

    m_name = val.toUpper().trimmed();
    emit nameChanged();
}

bool Character::rename(const QString &givenName)
{
    if (m_structure == nullptr)
        return false;

    const QString newNameUpper = givenName.trimmed().toUpper();
    const QString newNameCamel = Application::camelCased(newNameUpper);

    HourGlass hourGlass;

    this->clearRenameError();

    QString errMsg;
    const bool ret = m_structure->renameCharacter(m_name, newNameUpper, &errMsg);

    if (ret) {
        if (m_notes)
            m_notes->renameCharacter(m_name, newNameCamel);

        {
            int nrReplacements = 0;
            const QJsonObject newSummary = Application::replaceCharacterName(
                    m_name, newNameCamel, m_summary.toObject(), &nrReplacements);
            if (nrReplacements > 0) {
                m_summary = newSummary;
                emit summaryChanged();
            }
        }

        const bool merging = m_structure->allCharacterNames().contains(newNameUpper);

        if (merging) {
            Character *mergeWith = m_structure->findCharacter(newNameUpper);
            if (mergeWith == nullptr)
                mergeWith = m_structure->addCharacter(newNameUpper);

            if (mergeWith) {
                // Merge notes
                m_notes->moveNotes(mergeWith->notes());

                // Merge attachments
                m_attachments->moveAttachments(mergeWith->attachments());

                // Merge photos
                mergeWith->m_photos += m_photos;

                // Create summary of this character as a note in the merged character
                const QString newLine = QStringLiteral("\n");

                QString metaDataNoteContent;
                QTextStream ts(&metaDataNoteContent, QIODevice::WriteOnly);
                if (!m_type.isEmpty())
                    ts << "Type: " << m_type << newLine;
                if (!m_designation.isEmpty())
                    ts << "Designation: " << m_designation << newLine;
                if (!m_gender.isEmpty())
                    ts << "Gender: " << m_gender << newLine;
                if (!m_age.isEmpty())
                    ts << "Age: " << m_age << newLine;
                if (!m_height.isEmpty())
                    ts << "Height: " << m_height << newLine;
                if (!m_weight.isEmpty())
                    ts << "Weight: " << m_weight << newLine;
                if (!m_bodyType.isEmpty())
                    ts << "Body Type: " << m_bodyType << newLine;
                if (!m_aliases.isEmpty())
                    ts << "Aliases: " << m_aliases.join(", ") << newLine;
                ts.flush();

                if (!metaDataNoteContent.isEmpty()) {
                    Note *metaDataNote = mergeWith->notes()->addTextNote();
                    metaDataNote->setTitle(QStringLiteral("Meta data merged from ") + m_name);
                    metaDataNote->setContent(metaDataNoteContent);
                }

                if (!m_summary.isUndefined()) {
                    Note *summaryNote = mergeWith->notes()->addTextNote();
                    summaryNote->setTitle(QStringLiteral("Summary merged from ") + m_name);
                    summaryNote->setContent(m_summary);
                }

                // Create relationships in the merged character, that mimic those in this one.
                for (int i = 0; i < m_relationships.objectCount(); i++) {
                    Relationship *relationship = m_relationships.at(i);
                    if (relationship->of() == this)
                        mergeWith->addRelationship(relationship->name(), relationship->with());
                    else
                        relationship->with()->addRelationship(relationship->name(), mergeWith);
                }
            }

            // Since this character is merged, we won't need this object to stick around
            // anymore.
            QTimer::singleShot(100, this, [=]() { m_structure->removeCharacter(this); });
        }

        this->setCharacterRelationshipGraph(QJsonObject());
        m_structure->setCharacterRelationshipGraph(QJsonObject());
        m_name = newNameUpper;
        emit nameChanged();
    }

    if (!errMsg.isEmpty()) {
        m_renameError = errMsg;
        emit renameErrorChanged();
    }

    return ret;
}

void Character::clearRenameError()
{
    if (!m_renameError.isEmpty()) {
        m_renameError.clear();
        emit renameErrorChanged();
    }
}

void Character::setVisibleOnNotebook(bool val)
{
    if (m_visibleOnNotebook == val)
        return;

    m_visibleOnNotebook = val;
    emit visibleOnNotebookChanged();
}

void Character::setPhotos(const QStringList &val)
{
    if (m_photos == val || !m_photos.isEmpty())
        return;

    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    m_photos.reserve(val.size());
    for (const QString &item : val) {
        if (dfs->contains(item))
            m_photos << dfs->absolutePath(item);
    }

    emit photosChanged();
}

void Character::addPhoto(const QString &photoPath)
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    const QString dstPath = QStringLiteral("characters/")
            + QString::number(QDateTime::currentMSecsSinceEpoch()) + QStringLiteral(".jpg");
    const QString dfsPath = dfs->addImage(photoPath, dstPath, QSize(512, 512), true);
    if (dfsPath.isEmpty())
        return;

    m_photos << dfs->absolutePath(dfsPath);
    emit photosChanged();
}

void Character::removePhoto(int index)
{
    if (index < 0 || index >= m_photos.size())
        return;

    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    const QString dfsPath = dfs->relativePath(m_photos.at(index));
    if (dfsPath.isEmpty())
        return;

    if (dfs->remove(dfsPath)) {
        m_photos.removeAt(index);
        emit photosChanged();
    }
}

void Character::removePhoto(const QString &photoPath)
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    const QString dfsPath = dfs->absolutePath(photoPath);
    if (dfsPath.isEmpty())
        return;

    const int index = m_photos.indexOf(photoPath);
    this->removePhoto(index);
}

void Character::setKeyPhotoIndex(int val)
{
    const int val2 = qBound(-1, val, m_photos.size() - 1);
    if (m_keyPhotoIndex != val2) {
        m_keyPhotoIndex = val2;
        emit keyPhotoIndexChanged();
    }

    const QString kp = val2 >= 0 ? m_photos.at(val2) : QString();
    this->setKeyPhoto(kp);
}

void Character::setType(const QString &val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Character::setDesignation(const QString &val)
{
    if (m_designation == val)
        return;

    m_designation = val;
    emit designationChanged();
}

void Character::setGender(const QString &val)
{
    if (m_gender == val)
        return;

    m_gender = val;
    emit genderChanged();
}

void Character::setAge(const QString &val)
{
    if (m_age == val)
        return;

    m_age = val;
    emit ageChanged();
}

void Character::setHeight(const QString &val)
{
    if (m_height == val)
        return;

    m_height = val;
    emit heightChanged();
}

void Character::setWeight(const QString &val)
{
    if (m_weight == val)
        return;

    m_weight = val;
    emit weightChanged();
}

void Character::setBodyType(const QString &val)
{
    if (m_bodyType == val)
        return;

    m_bodyType = val;
    emit bodyTypeChanged();
}

void Character::setAliases(const QStringList &val)
{
    if (m_aliases == val)
        return;

    m_aliases.clear();
    for (const QString &item : val)
        m_aliases << item.trimmed();

    emit aliasesChanged();
}

void Character::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

void Character::setSummary(const QJsonValue &val)
{
    if (m_summary == val)
        return;

    m_summary = val;
    emit summaryChanged();
}

void Character::setTags(const QStringList &val)
{
    if (m_tags == val)
        return;

    m_tags = val;
    for (int i = m_tags.size() - 1; i >= 0; i--) {
        m_tags[i] = m_tags[i].trimmed();
        if (m_tags[i].isEmpty())
            m_tags.removeAt(i);
    }

    emit tagsChanged();
}

bool Character::addTag(const QString &givenTag)
{
    const QString tag = givenTag.trimmed();
    if (tag.isEmpty())
        return false;

    if (m_tags.contains(tag, Qt::CaseInsensitive))
        return false;

    m_tags.append(tag);
    emit tagsChanged();
    return true;
}

bool Character::removeTag(const QString &givenTag)
{
    const QString tag = givenTag.trimmed();
    if (tag.isEmpty())
        return false;

    for (int i = m_tags.size() - 1; i >= 0; i--) {
        const QString itag = m_tags.at(i);
        if (itag.compare(tag, Qt::CaseInsensitive) == 0) {
            m_tags.removeAt(i);
            emit tagsChanged();
            return true;
        }
    }

    return false;
}

bool Character::hasTag(const QString &givenTag) const
{
    const QString tag = givenTag.trimmed();
    if (tag.isEmpty())
        return false;

    return m_tags.contains(tag, Qt::CaseInsensitive);
}

void Character::setPriority(int val)
{
    if (m_priority == val)
        return;

    m_priority = qBound(-100, val, 100);
    emit priorityChanged();
}

QQmlListProperty<Relationship> Character::relationships()
{
    return QQmlListProperty<Relationship>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &Character::staticAppendRelationship, &Character::staticRelationshipCount,
            &Character::staticRelationshipAt, &Character::staticClearRelationships);
}

void Character::addRelationship(Relationship *ptr)
{
    if (ptr == nullptr || m_relationships.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Relationship::aboutToDelete, this, &Character::removeRelationship);
    connect(ptr, &Relationship::relationshipChanged, this, &Character::characterChanged);

    m_relationships.append(ptr);

    emit relationshipCountChanged();
}

void Character::removeRelationship(Relationship *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_relationships.indexOf(ptr);
    if (index < 0)
        return;

    m_relationships.removeAt(index);
    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);

    disconnect(ptr, &Relationship::aboutToDelete, this, &Character::removeRelationship);
    disconnect(ptr, &Relationship::relationshipChanged, this, &Character::characterChanged);

    emit relationshipCountChanged();
}

Relationship *Character::relationshipAt(int index) const
{
    return index < 0 || index >= m_relationships.size() ? nullptr : m_relationships.at(index);
}

void Character::setRelationships(const QList<Relationship *> &list)
{
    if (!m_relationships.isEmpty() || list.isEmpty())
        return;

    for (Relationship *ptr : list) {
        ptr->setParent(this);

        connect(ptr, &Relationship::aboutToDelete, this, &Character::removeRelationship);
        connect(ptr, &Relationship::relationshipChanged, this, &Character::characterChanged);
    }

    m_relationships.assign(list);
    emit relationshipCountChanged();
}

void Character::clearRelationships()
{
    while (m_relationships.size())
        this->removeRelationship(m_relationships.first());
}

Relationship *Character::addRelationship(const QString &name, Character *with)
{
    Relationship *relationship = nullptr;
    if (with == nullptr || with == this)
        return nullptr;

    // Find out if we have already established this relationsip.
    relationship = this->findRelationship(with);
    if (relationship != nullptr) {
        relationship->setName(name);
        return relationship;
    }

    // Create a new with-of relationship
    Relationship *withOf = new Relationship(with);
    withOf->setName(name);
    withOf->setWith(this);
    withOf->setDirection(Relationship::WithOf);
    with->addRelationship(withOf);

    // Create new of-with relationship
    Relationship *ofWith = new Relationship(this);
    ofWith->setName(name);
    ofWith->setWith(with);
    ofWith->setDirection(Relationship::OfWith);
    this->addRelationship(ofWith);

    // Ensure that if one of the relationships is destroyed, the other
    // must destroy itself.
    connect(withOf, &Relationship::aboutToDelete, ofWith, &Relationship::deleteLater);
    connect(ofWith, &Relationship::aboutToDelete, withOf, &Relationship::deleteLater);

    // Return the newly created relationship
    return ofWith;
}

Relationship *Character::findRelationshipWith(const QString &with) const
{
    const QString with2 = with.toUpper().simplified().trimmed();
    if (with2 == m_name)
        return nullptr;

    for (Relationship *rel : m_relationships.constList()) {
        if (rel->with()->name() == with2)
            return rel;
    }

    return nullptr;
}

Relationship *Character::findRelationship(Character *with) const
{
    if (with == nullptr || with == this)
        return nullptr;

    for (Relationship *rel : m_relationships.constList()) {
        if (rel->with() == with)
            return rel;
    }

    return nullptr;
}

bool Character::isRelatedTo(Character *with) const
{
    QStack<Character *> stack;
    return this->isRelatedToImpl(with, stack);
}

QList<Relationship *> Character::findRelationshipsWith(const QString &name) const
{
    QList<Relationship *> ret;

    const QString name2 = Relationship::polishName(name);
    for (Relationship *rel : m_relationships.constList()) {
        if (rel->name() == name2)
            ret << rel;
    }

    return ret;
}

QStringList Character::unrelatedCharacterNames() const
{
    const QStringList names = this->structure()->characterNames();

    QStringList ret;
    for (const QString &name : names) {
        if (this->name() == name || this->hasRelationshipWith(name))
            continue;

        ret << name;
    }

    return ret;
}

void Character::setCharacterRelationshipGraph(const QJsonObject &val)
{
    if (m_characterRelationshipGraph == val)
        return;

    m_characterRelationshipGraph = val;
    emit characterRelationshipGraphChanged();
}

void Character::serializeToJson(QJsonObject &json) const
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();
    QJsonArray array;
    for (const QString &photo : m_photos)
        array.append(dfs->relativePath(photo));
    json.insert("photos", array);
}

void Character::deserializeFromJson(const QJsonObject &json)
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();
    const QJsonArray array = json.value(QStringLiteral("photos")).toArray();

    QStringList photoPaths;
    for (int i = 0; i < array.size(); i++) {
        QString path = array.at(i).toString();
        if (QDir::isAbsolutePath(path))
            continue;

        path = dfs->absolutePath(path);
        if (path.isEmpty() || !QFile::exists(path))
            continue;

        QImage image(path);
        if (image.isNull())
            continue;

        photoPaths.append(path);
    }

    if (m_photos != photoPaths) {
        m_photos = photoPaths;
        emit photosChanged();
    }

    const QString kpiAttr = QStringLiteral("keyPhotoIndex");
    if (json.contains(kpiAttr)) {
        const int kpi = json.value(kpiAttr).toInt();
        this->setKeyPhotoIndex(kpi);
    }

    // Previously notes was an array, because the notes property used to be
    // a list property. Now notes is an object, because it represents Notes class.
    // So, if we are loading a notes from a file created using older versions of Scrite,
    // we have to upgrade the notes to the newer format based on the Notes class.
    const QJsonValue notes = json.value(QStringLiteral("notes"));
    if (notes.isArray())
        m_notes->loadOldNotes(notes.toArray());
}

bool Character::canSetPropertyFromObjectList(const QString &propName) const
{
    if (propName == QStringLiteral("relationships"))
        return m_relationships.isEmpty();

    return false;
}

void Character::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if (propName == QStringLiteral("relationships")) {
        this->setRelationships(qobject_list_cast<Relationship *>(objects));
        return;
    }
}

void Character::resolveRelationships()
{
    for (Relationship *rel : m_relationships.constList())
        rel->resolveRelationship();
}

void Character::write(QTextCursor &cursor, const WriteOptions &options) const
{
    QTextDocument &document = *cursor.document();

    // Character name
    QTextBlockFormat characaterNameBlockFormat;
    characaterNameBlockFormat.setTopMargin(10);
    characaterNameBlockFormat.setHeadingLevel(options.headingLevel);

    QTextCharFormat characterNameCharFormat;
    characterNameCharFormat.setFontWeight(QFont::Bold);
    characterNameCharFormat.setFontPointSize(
            ScreenplayTextDocument::headingFontPointSize(options.headingLevel));
    characaterNameBlockFormat.setTopMargin(characterNameCharFormat.fontPointSize() / 2);

    if (cursor.block().text().isEmpty()) {
        cursor.setBlockCharFormat(characterNameCharFormat);
        cursor.setBlockFormat(characaterNameBlockFormat);
    } else
        cursor.insertBlock(characaterNameBlockFormat, characterNameCharFormat);

    cursor.insertText(m_name);

    // Character meta-data table
    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(2);
    tableFormat.setCellSpacing(0);
    tableFormat.setLeftMargin(10);
    tableFormat.setTopMargin(10);
    tableFormat.setBottomMargin(10);

    QTextFrame *frame = cursor.currentFrame();
    QTextTable *table = cursor.insertTable(6, this->hasKeyPhoto() ? 3 : 2, tableFormat);
    QTextTableCellFormat cellFormat; // = table->cellAt(0, 0).format();
    cellFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    for (int tr = 0; tr < table->rows(); tr++) {
        for (int tc = 0; tc < table->columns(); tc++) {
            table->cellAt(tr, tc).setFormat(cellFormat);
        }
    }

    if (this->hasKeyPhoto()) {
        table->mergeCells(0, 0, 6, 1);

        const QImage image(this->keyPhoto());
        const qreal imageScale = 120.0 / image.width();
        const QSizeF imageSize = image.size() * imageScale;
        const QUrl imageName(
                QLatin1String("character://") + m_name.toLatin1().toHex() + QLatin1String("/")
                + QString::number(m_structure->indexOfCharacter(const_cast<Character *>(this))));
        document.addResource(QTextDocument::ImageResource, imageName, image);

        QTextImageFormat imageFormat;
        imageFormat.setWidth(imageSize.width());
        imageFormat.setHeight(imageSize.height());
        imageFormat.setName(imageName.toString());

        cursor = table->cellAt(0, 0).firstCursorPosition();
        cursor.insertImage(imageFormat);
    }

    const int col = this->hasKeyPhoto() ? 1 : 0;

    table->mergeCells(0, col, 1, 2);
    cursor = table->cellAt(0, col).firstCursorPosition();
    cursor.insertText(QLatin1String("Role: ") + this->designation());

    table->mergeCells(1, col, 1, 2);
    cursor = table->cellAt(1, col).firstCursorPosition();
    cursor.insertText(QLatin1String("Tags: ") + this->tags().join(QLatin1String(", ")));

    table->mergeCells(2, col, 1, 2);
    cursor = table->cellAt(2, col).firstCursorPosition();
    cursor.insertText(QLatin1String("Aliases: ") + this->aliases().join(QLatin1String(", ")));

    cursor = table->cellAt(3, col).firstCursorPosition();
    cursor.insertText(QLatin1String("Type: ") + this->type());

    cursor = table->cellAt(3, col + 1).firstCursorPosition();
    cursor.insertText(QLatin1String("Gender: ") + this->gender());

    cursor = table->cellAt(4, col).firstCursorPosition();
    cursor.insertText(QLatin1String("Age: ") + this->age());

    cursor = table->cellAt(4, col + 1).firstCursorPosition();
    cursor.insertText(QLatin1String("Body Type: ") + this->bodyType());

    cursor = table->cellAt(5, col).firstCursorPosition();
    cursor.insertText(QLatin1String("Height: ") + this->height());

    cursor = table->cellAt(5, col + 1).firstCursorPosition();
    cursor.insertText(QLatin1String("Weight: ") + this->weight());

    cursor = frame->lastCursorPosition();

    // Character Summary
    auto addSection = [&cursor, options](const QString &sectionName) {
        QTextBlockFormat sectionBlockFormat;
        sectionBlockFormat.setHeadingLevel(options.headingLevel + 1);

        QTextCharFormat sectionCharFormat;
        sectionCharFormat.setFontWeight(QFont::Bold);
        sectionCharFormat.setFontPointSize(
                ScreenplayTextDocument::headingFontPointSize(options.headingLevel));
        sectionBlockFormat.setTopMargin(sectionCharFormat.fontPointSize() / 2);

        cursor.insertBlock(sectionBlockFormat, sectionCharFormat);
        cursor.insertText(sectionName);

        QTextBlockFormat nextBlockFormat;
        nextBlockFormat.setHeadingLevel(0);

        QTextCharFormat nextCharFormat;
        nextCharFormat.setFontWeight(QFont::Normal);
        nextCharFormat.setFontPointSize(cursor.document()->defaultFont().pointSizeF());
        nextBlockFormat.setTopMargin(nextCharFormat.fontPointSize() / 2);

        cursor.insertBlock(nextBlockFormat, nextCharFormat);
    };

    if (options.includeSummary) {
        if (m_summary.isString()) {
            const QString summary = m_summary.toString();
            if (!summary.isEmpty()) {
                addSection(QLatin1String("Summary"));
                TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, summary);
            }
        } else {
            const QJsonObject summary = m_summary.toObject();
            if (!summary.isEmpty()) {
                const DeltaDocument::ResolveResult result = DeltaDocument::blockingResolve(summary);
                if (!result.htmlText.isEmpty()) {
                    addSection(QLatin1String("Summary"));
                    cursor.insertHtml(result.htmlText);
                }
            }
        }
    }

    // Character notes
    if (options.includeTextNotes || options.includeFormNotes) {
        if (m_notes) {
            Notes::WriteOptions notesOptions;
            notesOptions.includeFormNotes = options.includeFormNotes;
            notesOptions.includeTextNotes = options.includeTextNotes;
            m_notes->write(cursor, notesOptions);
        }
    }
}

bool Character::LessThan(Character *a, Character *b)
{
    return a->name().localeAwareCompare(b->name()) < 0;
}

bool Character::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        m_structure = qobject_cast<Structure *>(this->parent());
        if (m_structure) {
            connect(this, &Character::nameChanged, m_structure,
                    &Structure::updateCharacterNamesShotsTransitionsAndTagsLater);
            connect(this, &Character::tagsChanged, m_structure,
                    &Structure::updateCharacterNamesShotsTransitionsAndTagsLater);
            connect(this, &Character::priorityChanged, m_structure,
                    &Structure::updateCharacterNamesShotsTransitionsAndTagsLater);
        }
    }

    return QObject::event(event);
}

bool Character::isRelatedToImpl(Character *with, QStack<Character *> &stack) const
{
    if (with == nullptr || with == this)
        return false;

    QList<Relationship *> rels = m_relationships.list();
    for (Relationship *rel : rels) {
        Character *rwith = rel->with();
        if (rwith == nullptr)
            continue;

        if (rwith == with)
            return true;

        if (stack.contains(rwith))
            continue;

        stack.push(rwith);
        const bool flag = rwith->isRelatedToImpl(with, stack);
        stack.pop();

        if (flag)
            return flag;
    }

    return false;
}

void Character::onDfsAuction(const QString &filePath, int *claims)
{
    if (m_photos.isEmpty() || !filePath.startsWith(QStringLiteral("characters/")))
        return;

    const QString absPath = ScriteDocument::instance()->fileSystem()->absolutePath(filePath);
    if (m_photos.contains(absPath))
        *claims = *claims + 1;
}

void Character::setKeyPhoto(const QString &val)
{
    if (m_keyPhoto == val)
        return;

    m_keyPhoto = val;
    emit keyPhotoChanged();
}

void Character::staticAppendRelationship(QQmlListProperty<Relationship> *list, Relationship *ptr)
{
    reinterpret_cast<Character *>(list->data)->addRelationship(ptr);
}

void Character::staticClearRelationships(QQmlListProperty<Relationship> *list)
{
    reinterpret_cast<Character *>(list->data)->clearRelationships();
}

Relationship *Character::staticRelationshipAt(QQmlListProperty<Relationship> *list, int index)
{
    return reinterpret_cast<Character *>(list->data)->relationshipAt(index);
}

int Character::staticRelationshipCount(QQmlListProperty<Relationship> *list)
{
    return reinterpret_cast<Character *>(list->data)->relationshipCount();
}

///////////////////////////////////////////////////////////////////////////////

CharacterNamesModel::CharacterNamesModel(QObject *parent) : QStringListModel(parent)
{
    connect(this, &CharacterNamesModel::tagsChanged, this, &CharacterNamesModel::reload);
    connect(this, &CharacterNamesModel::structureChanged, this,
            &CharacterNamesModel::availableTagsChanged);
    connect(this, &CharacterNamesModel::structureChanged, this,
            &CharacterNamesModel::allNamesChanged);

    connect(this, &QStringListModel::rowsInserted, this, &CharacterNamesModel::countChanged);
    connect(this, &QStringListModel::rowsRemoved, this, &CharacterNamesModel::countChanged);
    connect(this, &QStringListModel::modelReset, this, &CharacterNamesModel::countChanged);
}

CharacterNamesModel::~CharacterNamesModel() { }

Character *CharacterNamesModel::findCharacter(const QString &name) const
{
    if (m_structure)
        return m_structure->findCharacter(name);

    return nullptr;
}

int CharacterNamesModel::count() const
{
    return this->rowCount(QModelIndex());
}

void CharacterNamesModel::setStructure(Structure *val)
{
    if (m_structure == val)
        return;

    if (m_structure) {
        disconnect(m_structure, &Structure::characterNamesChanged, this,
                   &CharacterNamesModel::reload);
        disconnect(m_structure, &Structure::characterTagsChanged, this,
                   &CharacterNamesModel::availableTagsChanged);
        disconnect(m_structure, &Structure::characterNamesChanged, this,
                   &CharacterNamesModel::allNamesChanged);
    }

    m_structure = val;

    if (m_structure) {
        connect(m_structure, &Structure::characterNamesChanged, this, &CharacterNamesModel::reload);
        connect(m_structure, &Structure::characterTagsChanged, this,
                &CharacterNamesModel::availableTagsChanged);
        connect(m_structure, &Structure::characterNamesChanged, this,
                &CharacterNamesModel::allNamesChanged);
    }

    emit structureChanged();

    this->reload();
}

void CharacterNamesModel::setTags(const QStringList &val)
{
    if (m_tags == val)
        return;

    m_tags = val;
    for (int i = m_tags.size() - 1; i >= 0; i--) {
        m_tags[i] = m_tags[i].trimmed();
        if (m_tags[i].isEmpty())
            m_tags.removeAt(i);
    }

    emit tagsChanged();
}

void CharacterNamesModel::addTag(const QString &givenTag)
{
    const QString tag = givenTag.trimmed();
    if (tag.isEmpty())
        return;

    if (!m_tags.contains(tag, Qt::CaseInsensitive)) {
        m_tags.append(tag);
        emit tagsChanged();
    }
}

void CharacterNamesModel::removeTag(const QString &givenTag)
{
    const QString tag = givenTag.trimmed();
    if (tag.isEmpty())
        return;

    if (m_tags.contains(tag, Qt::CaseInsensitive)) {
        m_tags.removeOne(tag);
        emit tagsChanged();
    }
}

void CharacterNamesModel::toggleTag(const QString &givenTag)
{
    const QString tag = givenTag.trimmed();
    if (tag.isEmpty())
        return;

    if (hasTag(tag))
        removeTag(tag);
    else
        addTag(tag);
}

void CharacterNamesModel::clearTags()
{
    this->setTags(QStringList());
}

bool CharacterNamesModel::hasTag(const QString &givenTag) const
{
    const QString tag = givenTag.trimmed();
    return tag.isEmpty() ? false : m_tags.contains(tag, Qt::CaseInsensitive);
}

QStringList CharacterNamesModel::availableTags() const
{
    return m_structure ? m_structure->characterTags() : QStringList();
}

QStringList CharacterNamesModel::allNames() const
{
    return m_structure ? m_structure->characterNames() : QStringList();
}

void CharacterNamesModel::setSelectedCharacters(const QStringList &val)
{
    if (m_selectedCharacters == val)
        return;

    m_selectedCharacters = val;
    for (int i = m_selectedCharacters.size() - 1; i >= 0; i--) {
        m_selectedCharacters[i] = m_selectedCharacters[i].trimmed();
        if (m_selectedCharacters[i].isEmpty())
            m_selectedCharacters.removeAt(i);
    }

    emit selectedCharactersChanged();
}

void CharacterNamesModel::addToSelection(const QString &givenName)
{
    const QString name = givenName.trimmed();
    if (name.isEmpty() || !this->stringList().contains(name, Qt::CaseInsensitive)
        || m_selectedCharacters.contains(name, Qt::CaseInsensitive))
        return;

    m_selectedCharacters.append(name);
    emit selectedCharactersChanged();
}

void CharacterNamesModel::removeFromSelection(const QString &givenName)
{
    const QString name = givenName.trimmed();
    if (name.isEmpty() || !m_selectedCharacters.contains(name, Qt::CaseInsensitive))
        return;

    m_selectedCharacters.removeOne(name);
    emit selectedCharactersChanged();
}

bool CharacterNamesModel::isInSelection(const QString &givenName) const
{
    const QString name = givenName.trimmed();
    if (name.isEmpty())
        return false;

    return m_selectedCharacters.contains(name, Qt::CaseInsensitive);
}

void CharacterNamesModel::clearSelection()
{
    this->setSelectedCharacters(QStringList());
}

void CharacterNamesModel::toggleSelection(const QString &name)
{
    if (this->isInSelection(name))
        this->removeFromSelection(name);
    else
        this->addToSelection(name);
}

void CharacterNamesModel::selectAll()
{
    const QStringList names = this->stringList();
    for (const QString &name : names)
        this->addToSelection(name);
}

void CharacterNamesModel::unselectAll()
{
    const QStringList names = this->stringList();
    for (const QString &name : names)
        this->removeFromSelection(name);
}

QHash<int, QByteArray> CharacterNamesModel::roleNames() const
{
    return QHash<int, QByteArray>({ { Qt::DisplayRole, QByteArrayLiteral("modelData") } });
}

void CharacterNamesModel::reload()
{
    QStringList names = m_structure->filteredCharacterNames(m_tags);
    names = m_structure->sortCharacterNames(names);
    this->setStringList(names);
}

///////////////////////////////////////////////////////////////////////////////

class AnnotationMetaData
{
public:
    AnnotationMetaData();
    ~AnnotationMetaData();

    QJsonArray get(const QString &type);
    bool update(const QString &type, const QJsonObject &attributes);

private:
    void save();

private:
    QJsonObject m_metaData;
    QString m_metaDataFile;
};

AnnotationMetaData::AnnotationMetaData()
{
    const QString qrcFileName = QStringLiteral(":/misc/annotations_metadata.json");
    const QString revisionKey = QStringLiteral("#revision");
    m_metaDataFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                             .absoluteFilePath(QStringLiteral("annotations_metadata.json"));

    auto loadMetaData = [](const QString &fileName) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly))
            return QJsonObject();
        return QJsonDocument::fromJson(file.readAll()).object();
    };

    const QJsonObject qrcMetaData = loadMetaData(qrcFileName);
    const QJsonObject diskMetaData = loadMetaData(m_metaDataFile);
    const int qrcRevision = qrcMetaData.value(revisionKey).toInt();
    const int diskRevision = diskMetaData.value(revisionKey).toInt();

    if (qrcRevision > diskRevision) {
        // TODO: see if it is possible to merge changes from the qrcFile into the diskFile
        m_metaData = qrcMetaData;
        this->save();
    } else
        m_metaData = diskMetaData;
}

AnnotationMetaData::~AnnotationMetaData()
{
    this->save();
}

QJsonArray AnnotationMetaData::get(const QString &type)
{
    const QJsonArray info = m_metaData.value(type).toArray();
    return info;
}

bool AnnotationMetaData::update(const QString &type, const QJsonObject &attributes)
{
    QJsonArray info = m_metaData.value(type).toArray();
    if (info.isEmpty())
        return false;

    for (int i = 0; i < info.size(); i++) {
        QJsonObject attrInfo = info.at(i).toObject();
        if (attrInfo.value(QStringLiteral("cache")).toBool() == false)
            continue;

        const QString attrName = attrInfo.value(QStringLiteral("name")).toString();
        const QJsonValue attrValue = attributes.value(attrName);
        if (attrValue.isNull() || attrValue.isUndefined())
            continue;

        attrInfo.insert(QStringLiteral("default"), attrValue);
        info[i] = attrInfo;
    }

    m_metaData.insert(type, info);

    this->save();

    return true;
}

void AnnotationMetaData::save()
{
    QFile file(m_metaDataFile);
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(m_metaData).toJson());
}

Q_GLOBAL_STATIC(AnnotationMetaData, GlobalAnnotationMetaData)

Annotation::Annotation(QObject *parent)
    : QObject(parent), m_structure(qobject_cast<Structure *>(parent))
{
    connect(this, &Annotation::typeChanged, this, &Annotation::annotationChanged);
    connect(this, &Annotation::geometryChanged, this, &Annotation::annotationChanged);
    connect(this, &Annotation::attributesChanged, this, &Annotation::annotationChanged);

    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    connect(dfs, &DocumentFileSystem::auction, this, &Annotation::onDfsAuction);
}

Annotation::~Annotation()
{
    emit aboutToDelete(this);
}

void Annotation::setType(const QString &val)
{
    // Can be set only once.
    if (m_type == val || !m_type.isEmpty())
        return;

    m_type = val;
    emit typeChanged();

    const QJsonArray metaData = ::GlobalAnnotationMetaData->get(m_type);
    this->setMetaData(metaData);
}

void Annotation::setResizable(bool val)
{
    if (m_resizable == val)
        return;

    m_resizable = val;
    emit resizableChanged();
}

void Annotation::setMovable(bool val)
{
    if (m_movable == val)
        return;

    m_movable = val;
    emit movableChanged();
}

void Annotation::setGeometry(const QRectF &val)
{
    if (m_geometry == val)
        return;

    QRectF val2;
    val2.setSize(m_resizable || m_geometry.size().isEmpty() ? val.size() : m_geometry.size());
    val2.moveTopLeft(m_movable || m_geometry.topLeft().isNull() ? val.topLeft()
                                                                : m_geometry.topLeft());
    if (m_geometry == val2)
        return;

    m_geometry = val2;
    emit geometryChanged();
}

void Annotation::setAttributes(const QJsonObject &val)
{
    if (m_attributes == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "attributes");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_attributes = val;
    emit attributesChanged();

    this->polishAttributes();
}

void Annotation::setAttribute(const QString &key, const QJsonValue &value)
{
    m_attributes.insert(key, value);
    emit attributesChanged();
}

void Annotation::removeAttribute(const QString &key)
{
    m_attributes.remove(key);
    emit attributesChanged();
}

void Annotation::saveAttributesAsDefault()
{
    ::GlobalAnnotationMetaData->update(m_type, m_attributes);
}

void Annotation::setMetaData(const QJsonArray &val)
{
    // Can be set only once.
    if (m_metaData == val || !m_metaData.isEmpty())
        return;

    m_metaData = val;
    m_fileAttributes.clear();

    for (int i = 0; i < m_metaData.count(); i++) {
        const QString visibleAttr = QStringLiteral("visible");
        const QString isFileAttr = QStringLiteral("isFile");
        const QString nameAttr = QStringLiteral("name");

        QJsonObject obj = m_metaData.at(i).toObject();
        if (!obj.contains(visibleAttr)) {
            obj.insert(visibleAttr, true);
            m_metaData.replace(i, obj);
        }

        if (obj.contains(isFileAttr) && obj.value(isFileAttr).isBool())
            m_fileAttributes.append(obj.value(nameAttr).toString());
    }

    this->polishAttributes();
    emit metaDataChanged();
}

bool Annotation::removeImage(const QString &name) const
{
    if (name.isEmpty())
        return false;

    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    if (dfs->contains(name)) {
        dfs->remove(name);
        return true;
    }

    return false;
}

QString Annotation::addImage(const QString &path) const
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    const QString addedPath = dfs->add(path, QStringLiteral("annotation"));
    return dfs->relativePath(addedPath);
}

QString Annotation::addImage(const QVariant &image) const
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    const QString path = QStringLiteral("annotation/")
            + QString::number(QDateTime::currentSecsSinceEpoch()) + QStringLiteral(".jpg");
    const QString absPath = dfs->absolutePath(path, true);
    const QImage img = image.value<QImage>();
    img.save(absPath, "JPG");
    return dfs->relativePath(absPath);
}

QUrl Annotation::imageUrl(const QString &name) const
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    return QUrl::fromLocalFile(dfs->absolutePath(name));
}

void Annotation::createCopyOfFileAttributes()
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    FileManager fileManager;

    for (int i = 0; i < m_metaData.size(); i++) {
        const QJsonObject item = m_metaData.at(i).toObject();
        const QJsonValue isFileVal = item.value(QStringLiteral("isFile"));
        if (isFileVal.isBool() && isFileVal.toBool()) {
            const QString attrName = item.value(QStringLiteral("name")).toString();
            QString fileName = m_attributes.value(attrName).toString();
            if (fileName.isEmpty())
                continue;

            // Special case scenario only for images.
            const QString imageKey = QStringLiteral("image");
            if (m_type == imageKey && attrName == imageKey) {
                const QString base64Prefix = QStringLiteral("data://base64:");
                if (fileName.startsWith(base64Prefix)) {
                    QByteArray bytes = fileName.midRef(base64Prefix.length()).toLatin1();
                    bytes = QByteArray::fromBase64(bytes);
                    QBuffer buffer(&bytes);
                    buffer.open(QIODevice::ReadOnly);
                    QImage image;
                    if (image.load(&buffer, "JPG")) {
                        const QString tmpFile = dfs->absolutePath(QStringLiteral("tmp.jpg"));
                        image.save(tmpFile, "JPG");
                        fileManager.addToAutoDeleteList(tmpFile);
                        fileName = dfs->relativePath(tmpFile);
                    } else
                        fileName.clear();
                }
            }

            m_attributes.insert(attrName, dfs->duplicate(fileName, QStringLiteral("annotation")));
            emit attributesChanged();
        }
    }
}

bool Annotation::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange)
        m_structure = qobject_cast<Structure *>(this->parent());

    return QObject::event(event);
}

void Annotation::polishAttributes()
{
    bool attribsChanged = false;
    QJsonArray::const_iterator it = m_metaData.constBegin();
    QJsonArray::const_iterator end = m_metaData.constEnd();
    while (it != end) {
        const QJsonObject meta = (*it).toObject();
        if (meta.isEmpty()) {
            ++it;
            continue;
        }

        const QString key = meta.value(QStringLiteral("name")).toString();
        if (key.isEmpty()) {
            ++it;
            continue;
        }

        QJsonValue attrVal = m_attributes.value(key);
        if (attrVal.isUndefined()) {
            m_attributes.insert(key, meta.value(QStringLiteral("default")));
            attribsChanged = true;
        } else {
            const bool isNumber =
                    meta.value(QStringLiteral("type")).toString() == QStringLiteral("number");
            if (isNumber) {
                const qreal min = meta.value(QStringLiteral("min")).toDouble();
                const qreal max = meta.value(QStringLiteral("max")).toDouble();
                const qreal val = qBound(min, attrVal.toDouble(), max);
                if (val != attrVal.toDouble()) {
                    m_attributes.insert(key, val);
                    attribsChanged = true;
                }
            }
        }

        ++it;
    }

    if (attribsChanged)
        emit attributesChanged();
}

void Annotation::onDfsAuction(const QString &filePath, int *claims)
{
    if (m_fileAttributes.isEmpty() || !filePath.startsWith(QStringLiteral("annotation/")))
        return;

    for (const QString &fileAttr : qAsConst(m_fileAttributes)) {
        const QString attrFilePath = m_attributes.value(fileAttr).toString();
        if (attrFilePath == filePath)
            *claims = *claims + 1;
    }
}

///////////////////////////////////////////////////////////////////////////////

Structure::Structure(QObject *parent)
    : QObject(parent),
      m_scriteDocument(qobject_cast<ScriteDocument *>(parent)),
      m_locationHeadingsMapTimer("Structure.m_locationHeadingsMapTimer")
{
    connect(m_notes, &Notes::notesModified, this, &Structure::structureChanged);
    connect(this, &Structure::zoomLevelChanged, this, &Structure::structureChanged);
    connect(this, &Structure::groupsDataChanged, this, &Structure::structureChanged);
    connect(this, &Structure::elementCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::characterCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::annotationCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::currentElementIndexChanged, this, &Structure::structureChanged);
    connect(this, &Structure::preferredGroupCategoryChanged, this, &Structure::structureChanged);
    connect(m_attachments, &Attachments::attachmentsModified, this, &Structure::structureChanged);
    connect(this, &Structure::characterRelationshipGraphChanged, this,
            &Structure::structureChanged);

    QClipboard *clipboard = qApp->clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Structure::onClipboardDataChanged);

    // Load clipboard data after the constructor returns
    QTimer::singleShot(0, this, &Structure::onClipboardDataChanged);

    m_elementsBoundingBoxAggregator.setModel(&m_elements);
    m_elementsBoundingBoxAggregator.setAggregateFunction(
            [=](const QModelIndex &index, QVariant &value) {
                QRectF rect = value.toRectF();
                const StructureElement *element = m_elements.at(index.row());
                rect |= element->geometry();
                value = rect;
            });
    connect(&m_elementsBoundingBoxAggregator, &ModelAggregator::aggregateValueChanged, this,
            &Structure::elementsBoundingBoxChanged);

    m_annotationsBoundingBoxAggregator.setModel(&m_annotations);
    m_annotationsBoundingBoxAggregator.setAggregateFunction(
            [=](const QModelIndex &index, QVariant &value) {
                QRectF rect = value.toRectF();
                const Annotation *annot = m_annotations.at(index.row());
                rect |= annot->geometry();
                value = rect;
            });
    connect(&m_annotationsBoundingBoxAggregator, &ModelAggregator::aggregateValueChanged, this,
            &Structure::annotationsBoundingBoxChanged);

    m_elementStacks.m_structure = this;

    if (m_scriteDocument != nullptr) {
        Screenplay *screenplay = m_scriteDocument->screenplay();
        if (screenplay != nullptr) {
            connect(screenplay, &Screenplay::elementMoved, &m_elementStacks,
                    &StructureElementStacks::evaluateStacksLater);
            connect(screenplay, &Screenplay::elementInserted, [=](ScreenplayElement *ptr, int) {
                if (ptr->elementType() == ScreenplayElement::BreakElementType)
                    m_elementStacks.evaluateStacksLater();
            });
        }
    }

    this->loadDefaultGroupsData();

    m_transitions = Scrite::defaultTransitions();
    m_shots = Scrite::defaultShots();

    this->loadDefaultIndexCardFields();
    m_indexCardFields = m_defaultIndexCardFields;
}

Structure::~Structure()
{
    emit aboutToDelete(this);
}

void Structure::setCanvasWidth(qreal val)
{
    if (qFuzzyCompare(m_canvasWidth, val))
        return;

    m_canvasWidth = val;
    emit canvasWidthChanged();
}

void Structure::setCanvasHeight(qreal val)
{
    if (qFuzzyCompare(m_canvasHeight, val))
        return;

    m_canvasHeight = val;
    emit canvasHeightChanged();
}

void Structure::setCanvasGridSize(qreal val)
{
    if (qFuzzyCompare(m_canvasGridSize, val))
        return;

    m_canvasGridSize = val;
    emit canvasGridSizeChanged();
}

void Structure::setCanvasUIMode(Structure::CanvasUIMode val)
{
    if (!m_elementStacks.isEmpty())
        val = IndexCardUI;

    if (m_canvasUIMode == val)
        return;

    m_canvasUIMode = val;
    emit canvasUIModeChanged();
}

void Structure::setIndexCardContent(IndexCardContent val)
{
    if (m_indexCardContent == val)
        return;

    m_indexCardContent = val;
    emit indexCardContentChanged();
}

qreal Structure::snapToGrid(qreal val) const
{
    return Structure::snapToGrid(val, this);
}

qreal Structure::snapToGrid(qreal val, const Structure *structure, qreal defaultGridSize)
{
    if (val < 0)
        return 0;

    const qreal cgs = structure == nullptr ? defaultGridSize : structure->canvasGridSize();
    int nrGrids = qRound(val / cgs);
    return nrGrids * cgs;
}

void Structure::captureStructureAsImage(const QString &fileName)
{
    emit captureStructureAsImageRequest(fileName);
}

QQmlListProperty<Character> Structure::characters()
{
    return QQmlListProperty<Character>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &Structure::staticAppendCharacter, &Structure::staticCharacterCount,
            &Structure::staticCharacterAt, &Structure::staticClearCharacters);
}

void Structure::addCharacter(Character *ptr)
{
    if (ptr == nullptr || m_characters.indexOf(ptr) >= 0)
        return;

    Character *ch = ptr->isValid() ? this->findCharacter(ptr->name()) : nullptr;
    if (!ptr->isValid() || ch != nullptr) {
        if (ptr->parent() == this)
            GarbageCollector::instance()->add(ptr);
        return;
    }

    ptr->setParent(this);

    connect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
    connect(ptr, &Character::characterChanged, this, &Structure::structureChanged);

    m_characters.append(ptr);
    emit characterCountChanged();

    this->updateCharacterNamesShotsTransitionsAndTagsLater();
}

void Structure::removeCharacter(Character *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_characters.indexOf(ptr);
    if (index < 0)
        return;

    m_characters.removeAt(index);

    disconnect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
    disconnect(ptr, &Character::characterChanged, this, &Structure::structureChanged);

    emit characterCountChanged();

    this->updateCharacterNamesShotsTransitionsAndTagsLater();

    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

Character *Structure::characterAt(int index) const
{
    return index < 0 || index >= m_characters.size() ? nullptr : m_characters.at(index);
}

int Structure::indexOfCharacter(Character *ptr) const
{
    return ptr == nullptr ? -1 : m_characters.indexOf(ptr);
}

void Structure::setCharacters(const QList<Character *> &list)
{
    if (!m_characters.isEmpty() || list.isEmpty())
        return;

    // We dont have to capture this as an undoable action, because this method
    // is only called as a part of loading the Structure. What's the point in
    // undoing a Structure loaded from file.

    QList<Character *> list2;
    list2.reserve(list.size());

    for (Character *ptr : list) {
        if (!ptr->isValid() || this->findCharacter(ptr->name()) != nullptr) {
            GarbageCollector::instance()->add(ptr);
            continue;
        }

        ptr->setParent(this);
        connect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
        connect(ptr, &Character::characterChanged, this, &Structure::structureChanged);
        list2.append(ptr);
    }

    m_characters.assign(list2);
    emit characterCountChanged();

    this->updateCharacterNamesShotsTransitionsAndTagsLater();
}

void Structure::clearCharacters()
{
    while (m_characters.size())
        this->removeCharacter(m_characters.first());
}

QJsonArray Structure::detectCharacters() const
{
    QJsonArray ret;

    const QStringList names = this->allCharacterNames();

    for (const QString &name : names) {
        Character *character = this->findCharacter(name);

        QJsonObject item;
        item.insert("name", name);
        item.insert("added", character != nullptr);
        ret.append(item);
    }

    return ret;
}

Character *Structure::addCharacter(const QString &name)
{
    const QString name2 = name.toUpper().simplified().trimmed();
    if (name2.isEmpty())
        return nullptr;

    Character *character = this->findCharacter(name2);
    if (character == nullptr) {
        character = new Character(this);
        character->setName(name2);
        this->addCharacter(character);
    }

    return character;
}

void Structure::addCharacters(const QStringList &names)
{
    for (const QString &name : names)
        this->addCharacter(name);
}

Character *Structure::findCharacter(const QString &name) const
{
    const QString name2 = name.trimmed().toUpper();
    for (Character *character : m_characters.constList()) {
        if (character->name() == name2)
            return character;
    }

    return nullptr;
}

QList<Character *> Structure::findCharacters(const QStringList &names,
                                             bool returnAssociativeList) const
{
    QList<Character *> ret;
    for (const QString &name : names) {
        Character *character = this->findCharacter(name);
        if (returnAssociativeList || character != nullptr)
            ret << character;
    }

    return ret;
}

QQmlListProperty<StructureElement> Structure::elements()
{
    return QQmlListProperty<StructureElement>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &Structure::staticAppendElement, &Structure::staticElementCount,
            &Structure::staticElementAt, &Structure::staticClearElements);
}

void Structure::addElement(StructureElement *ptr)
{
    this->insertElement(ptr, -1);
}

static void structureAppendElement(Structure *structure, StructureElement *ptr)
{
    structure->addElement(ptr);
}
static void structureRemoveElement(Structure *structure, StructureElement *ptr)
{
    structure->removeElement(ptr);
}
static void structureInsertElement(Structure *structure, StructureElement *ptr, int index)
{
    structure->insertElement(ptr, index);
}
static StructureElement *structureElementAt(Structure *structure, int index)
{
    return structure->elementAt(index);
}
static int structureIndexOfElement(Structure *structure, StructureElement *ptr)
{
    return structure->indexOfElement(ptr);
}

void Structure::removeElement(StructureElement *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_elements.indexOf(ptr);
    if (index < 0)
        return;

    QScopedPointer<PushObjectListCommand<Structure, StructureElement>> cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if (!info->isLocked() && /* DISABLES CODE */ (false)) {
        ObjectListPropertyMethods<Structure, StructureElement> methods(
                &structureAppendElement, &structureRemoveElement, &structureInsertElement,
                &structureElementAt, structureIndexOfElement);
        cmd.reset(new PushObjectListCommand<Structure, StructureElement>(
                ptr, this, "elements", ObjectList::RemoveOperation, methods));
    }

    const Scene *scene = ptr->scene();
    const QList<SceneElement *> sceneElements = scene->findChildren<SceneElement *>();
    for (SceneElement *sceneElement : sceneElements)
        this->onAboutToRemoveSceneElement(sceneElement);
    this->updateCharacterNamesShotsTransitionsAndTagsLater();

    m_elements.removeAt(index);

    disconnect(ptr, &StructureElement::elementChanged, this, &Structure::structureChanged);
    disconnect(ptr, &StructureElement::aboutToDelete, this, &Structure::removeElement);
    disconnect(ptr, &StructureElement::sceneLocationChanged, this,
               &Structure::updateLocationHeadingMapLater);
    disconnect(ptr, &StructureElement::geometryChanged, &m_elements,
               &QObjectListModel<StructureElement *>::objectChanged);
    disconnect(ptr, &StructureElement::aboutToDelete, &m_elements,
               &QObjectListModel<StructureElement *>::objectDestroyed);
    disconnect(ptr, &StructureElement::stackIdChanged, &m_elementStacks,
               &StructureElementStacks::evaluateStacksLater);
    this->updateLocationHeadingMapLater();

    emit elementCountChanged();
    emit elementsChanged();

    this->resetCurentElementIndex();

    if (m_forceBeatBoardLayout) {
        Screenplay *screenplay = m_scriteDocument ? m_scriteDocument->screenplay()
                                                  : ScriteDocument::instance()->screenplay();
        this->placeElementsInBeatBoardLayout(screenplay);
    }

    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

void Structure::removeElements(const QList<StructureElement *> &elements)
{
    if (elements.isEmpty())
        return;

    for (StructureElement *element : elements)
        this->removeElement(element);
}

void Structure::insertElement(StructureElement *ptr, int index)
{
    if (ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    QScopedPointer<PushObjectListCommand<Structure, StructureElement>> cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if (!info->isLocked() && /* DISABLES CODE */ (false)) {
        ObjectListPropertyMethods<Structure, StructureElement> methods(
                &structureAppendElement, &structureRemoveElement, &structureInsertElement,
                &structureElementAt, structureIndexOfElement);
        cmd.reset(new PushObjectListCommand<Structure, StructureElement>(
                ptr, this, "elements", ObjectList::InsertOperation, methods));
    }

    if (index < 0 || index >= m_elements.size())
        m_elements.append(ptr);
    else
        m_elements.insert(index, ptr);

    ptr->setParent(this);

    connect(ptr, &StructureElement::elementChanged, this, &Structure::structureChanged);
    connect(ptr, &StructureElement::aboutToDelete, this, &Structure::removeElement);
    connect(ptr, &StructureElement::sceneLocationChanged, this,
            &Structure::updateLocationHeadingMapLater);
    connect(ptr, &StructureElement::geometryChanged, &m_elements,
            &QObjectListModel<StructureElement *>::objectChanged);
    connect(ptr, &StructureElement::aboutToDelete, &m_elements,
            &QObjectListModel<StructureElement *>::objectDestroyed);
    connect(ptr, &StructureElement::stackIdChanged, &m_elementStacks,
            &StructureElementStacks::evaluateStacksLater);
    this->updateLocationHeadingMapLater();

    this->onStructureElementSceneChanged(ptr);

    emit elementCountChanged();
    emit elementsChanged();

    if (this->scriteDocument() && !this->scriteDocument()->isLoading())
        this->setCurrentElementIndex(index);
}

void Structure::moveElement(StructureElement *ptr, int toRow)
{
    if (ptr == nullptr || toRow < 0 || toRow >= m_elements.size())
        return;

    const int fromRow = m_elements.indexOf(ptr);
    if (fromRow < 0)
        return;

    if (fromRow == toRow)
        return;

    m_elements.move(fromRow, toRow);
    emit elementsChanged();

    this->resetCurentElementIndex();
}

void Structure::setElements(const QList<StructureElement *> &list)
{
    if (!m_elements.isEmpty() || list.isEmpty())
        return;

    // We dont have to capture this as an undoable action, because this method
    // is only called as a part of loading the Structure. What's the point in
    // undoing a Structure loaded from file.

    for (StructureElement *element : list) {
        element->setParent(this);

        connect(element, &StructureElement::elementChanged, this, &Structure::structureChanged);
        connect(element, &StructureElement::aboutToDelete, this, &Structure::removeElement);
        connect(element, &StructureElement::sceneLocationChanged, this,
                &Structure::updateLocationHeadingMapLater);
        connect(element, &StructureElement::geometryChanged, &m_elements,
                &QObjectListModel<StructureElement *>::objectChanged);
        connect(element, &StructureElement::aboutToDelete, &m_elements,
                &QObjectListModel<StructureElement *>::objectDestroyed);
        connect(element, &StructureElement::stackIdChanged, &m_elementStacks,
                &StructureElementStacks::evaluateStacksLater);
        this->onStructureElementSceneChanged(element);
    }

    m_elements.assign(list);
    this->setCurrentElementIndex(0);
    emit elementCountChanged();
    emit elementsChanged();

    this->updateCharacterNamesShotsTransitionsAndTagsLater();
}

StructureElement *Structure::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

void Structure::clearElements()
{
    while (m_elements.size())
        this->removeElement(m_elements.first());
}

int Structure::indexOfScene(Scene *scene) const
{
    if (scene == nullptr)
        return -1;

    for (int i = 0; i < m_elements.size(); i++) {
        StructureElement *element = m_elements.at(i);
        if (element->scene() == scene)
            return i;
    }

    return -1;
}

int Structure::indexOfElement(StructureElement *element) const
{
    return m_elements.indexOf(element);
}

StructureElement *Structure::findElementBySceneID(const QString &id) const
{
    if (id.isEmpty())
        return nullptr;

    for (StructureElement *element : m_elements.constList()) {
        if (element->scene()->id() == id)
            return element;
    }

    return nullptr;
}

QRectF Structure::layoutElements(Structure::LayoutType layoutType)
{
    QRectF newBoundingRect;

    QList<StructureElement *> elementsToLayout;
    for (StructureElement *element : m_elements.constList())
        if (element->isSelected())
            elementsToLayout << element;

    if (elementsToLayout.isEmpty())
        elementsToLayout = m_elements;

    if (elementsToLayout.size() < 2)
        return newBoundingRect;

    const Screenplay *screenplay = ScriteDocument::instance()->screenplay();
    if (screenplay == nullptr)
        return newBoundingRect;

    QStringList stackIds;
    for (int i = elementsToLayout.size() - 1; i >= 0; i--) {
        StructureElement *element = elementsToLayout.at(i);
        if (element->stackId().isEmpty())
            continue;

        if (stackIds.contains(element->stackId()))
            elementsToLayout.removeAt(i);
        else
            stackIds.append(element->stackId());
    }

    auto lessThan = [screenplay](StructureElement *e1, StructureElement *e2) -> bool {
        const int pos1 = screenplay->firstIndexOfScene(e1->scene());
        const int pos2 = screenplay->firstIndexOfScene(e2->scene());
        if (pos1 >= 0 && pos2 >= 0)
            return pos1 < pos2;
        if (pos2 < 0)
            return true;
        return false;
    };
    std::sort(elementsToLayout.begin(), elementsToLayout.end(), lessThan);

    QRectF oldBoundingRect;
    for (StructureElement *element : qAsConst(elementsToLayout))
        oldBoundingRect |= QRectF(element->x(), element->y(), element->width(), element->height());

    const qreal verticalLayoutSpacing = m_canvasUIMode == IndexCardUI ? 100 : 50;
    const qreal horizontalLayoutSpacing = verticalLayoutSpacing;
    const qreal flowVerticalLayoutSpacing = m_canvasUIMode == IndexCardUI ? -125 : 20;
    const qreal flowHorizontalLayoutSpacing = m_canvasUIMode == IndexCardUI ? -125 : 20;

    int direction = 1;
    QRectF elementRect;
    for (int i = 0; i < elementsToLayout.size(); i++) {
        StructureElement *element = elementsToLayout.at(i);
        if (i == 0) {
            elementRect = QRectF(element->position(), QSize(element->width(), element->height()));
            newBoundingRect = elementRect;

            if (layoutType == HorizontalLayout || layoutType == FlowHorizontalLayout) {
                if (elementRect.left() > oldBoundingRect.center().x()) {
                    direction = -1;
                    elementRect.moveRight(oldBoundingRect.right());
                }
            } else {
                if (elementRect.top() > oldBoundingRect.center().y()) {
                    direction = -1;
                    elementRect.moveBottom(oldBoundingRect.bottom());
                }
            }

            if (direction < 0) {
                elementRect =
                        QRectF(element->position(), QSize(element->width(), element->height()));
                newBoundingRect = elementRect;
            }

            continue;
        }

        switch (layoutType) {
        case VerticalLayout:
            if (direction > 0)
                elementRect.moveTop(elementRect.bottom() + verticalLayoutSpacing);
            else
                elementRect.moveBottom(elementRect.top() - verticalLayoutSpacing);
            break;
        case HorizontalLayout:
            if (direction > 0)
                elementRect.moveLeft(elementRect.right() + horizontalLayoutSpacing);
            else
                elementRect.moveRight(elementRect.left() - horizontalLayoutSpacing);
            break;
        case FlowVerticalLayout:
            if (direction > 0)
                elementRect.moveTop(elementRect.bottom() + flowVerticalLayoutSpacing);
            else
                elementRect.moveBottom(elementRect.top() - flowVerticalLayoutSpacing);
            if (i % 2)
                elementRect.moveLeft(elementRect.right() + horizontalLayoutSpacing / 2);
            else
                elementRect.moveRight(elementRect.left() - horizontalLayoutSpacing / 2);
            break;
        case FlowHorizontalLayout:
            if (direction > 0)
                elementRect.moveLeft(elementRect.right() + flowHorizontalLayoutSpacing);
            else
                elementRect.moveRight(elementRect.left() - flowHorizontalLayoutSpacing);
            if (i % 2)
                elementRect.moveTop(elementRect.bottom() + verticalLayoutSpacing / 2);
            else
                elementRect.moveBottom(elementRect.top() - verticalLayoutSpacing / 2);
            break;
        }

        element->setPosition(elementRect.topLeft());
        newBoundingRect |= elementRect;
    }

    return newBoundingRect;
}

void Structure::setForceBeatBoardLayout(bool val)
{
    if (m_forceBeatBoardLayout == val || m_deserializationStage == BeingDeserialized)
        return;

    m_forceBeatBoardLayout = val;
    if (val && ScriteDocument::instance()->structure() == this) {
        ExecLaterTimer::call(
                "Structure::placeElementsInBeatBoardLayout", this,
                [=]() {
                    this->placeElementsInBeatBoardLayout(ScriteDocument::instance()->screenplay());
                },
                m_deserializationStage == JustDeserialized ? 1000 : 250);
    }

    emit forceBeatBoardLayoutChanged();
}

void Structure::placeElement(StructureElement *element, Screenplay *screenplay) const
{
    if (m_elements.isEmpty() || element == nullptr || m_elements.indexOf(element) < 0)
        return;

    const qreal x = 5000;
    const qreal y = 5000;
    const qreal xSpacing = 100;
    const qreal ySpacing = 250;
    const qreal elementWidthHint = 350;
    const qreal elementHeightHint = 375;

    if (m_elements.size() == 1) {
        element->setX(x);
        element->setY(y);
        return;
    }

    auto evaluateBoundingRect = [=]() {
        QRectF ret;
        for (StructureElement *e : m_elements.list()) {
            if (e == element)
                continue;
            const qreal ew = qFuzzyIsNull(e->width()) ? elementWidthHint : e->width();
            const qreal eh = qFuzzyIsNull(e->height()) ? elementHeightHint : e->height();
            ret |= QRectF(e->x(), e->y(), ew, eh);
        }
        return ret;
    };

    if (screenplay == nullptr) {
        const QRectF boundingRect = evaluateBoundingRect();
        element->setX(boundingRect.right() + xSpacing);
        element->setY(boundingRect.top());
        return;
    }

    if (m_forceBeatBoardLayout) {
        this->placeElementsInBeatBoardLayout(screenplay);
        return;
    }

    const QList<QPair<QString, QList<StructureElement *>>> beats =
            this->evaluateGroupsImpl(screenplay);
    for (const QPair<QString, QList<StructureElement *>> &beat : beats) {
        const int index = beat.second.indexOf(element);
        if (index < 0)
            continue;

        if (index == 0) {
            if (beat.second.size() == 1) {
                const QRectF boundingRect = evaluateBoundingRect();
                element->setX(boundingRect.left());
                element->setY(boundingRect.bottom() + ySpacing);
                return;
            }
        }

        StructureElement *before = beat.second.at(index - 1);
        if (beat.second.last() == element) {
            const qreal bw = qFuzzyIsNull(before->width()) ? elementWidthHint : before->width();
            element->setX(before->x() + bw + xSpacing);
            element->setY(before->y());
            return;
        }

        const qreal bh = qFuzzyIsNull(before->height()) ? elementHeightHint : before->height();
        element->setX(before->x());
        element->setY(before->y() + bh + ySpacing / 2);
        return;
    }
}

QRectF Structure::placeElementsInBeatBoardLayout(Screenplay *screenplay) const
{
    QRectF newBoundingRect;

    if (screenplay == nullptr)
        return newBoundingRect;

    const QList<QPair<QString, QList<StructureElement *>>> beats =
            this->evaluateGroupsImpl(screenplay);

    const qreal x = 5000;
    const qreal y = 5000;
    const qreal xSpacing = 100;
    const qreal ySpacing = 175;

    QMap<QString, QPointF> stackPositions;

    int episodeIndex = 0;
    QRectF elementRect(x, y, 0, 0);

    for (const QPair<QString, QList<StructureElement *>> &beat : beats) {
        if (beat.second.isEmpty())
            continue;

        const int beatEpisodeIndex = beat.second.first()->scene()->episodeIndex();
        if (episodeIndex != beatEpisodeIndex) {
            elementRect.moveTop(elementRect.top() + ySpacing);
            episodeIndex = beatEpisodeIndex;
        }

        QString lastStackId;
        QRectF beatRect;
        for (StructureElement *element : qAsConst(beat.second)) {
            const QString stackId = element->stackId();
            if (element != beat.second.first() && (stackId.isEmpty() || stackId != lastStackId))
                elementRect.moveTopLeft(elementRect.topRight() + QPointF(xSpacing, 0));

            elementRect.setWidth(element->width());
            elementRect.setHeight(element->height());
            beatRect |= elementRect;
            newBoundingRect |= elementRect;

            const QPointF elementPos = stackId.isEmpty() || !stackPositions.contains(stackId)
                    ? elementRect.topLeft()
                    : stackPositions.value(stackId);
            element->setPosition(elementPos);
            if (!stackId.isEmpty() && !stackPositions.contains(stackId))
                stackPositions.insert(stackId, elementPos);

            lastStackId = stackId;
        }

        elementRect.moveTopLeft(QPointF(x, beatRect.bottom() + ySpacing));
    }

    return newBoundingRect;
}

QJsonObject Structure::evaluateEpisodeAndGroupBoxes(Screenplay *screenplay,
                                                    const QString &category) const
{
    const QList<QPair<QString, QList<StructureElement *>>> groups =
            this->evaluateGroupsImpl(screenplay, category);

    QJsonObject ret;
    QJsonArray groupBoxes;

    for (const QPair<QString, QList<StructureElement *>> &group : groups) {
        if (group.second.isEmpty())
            continue;

        QJsonArray sceneIndexes;

        QRectF groupBox;
        for (StructureElement *element : qAsConst(group.second)) {
            groupBox |= QRectF(element->x(), element->y(), element->width(), element->height());
            sceneIndexes.append(this->indexOfElement(element));
        }

        QJsonObject groupJson;
        groupJson.insert(QStringLiteral("name"), group.first);
        groupJson.insert(QStringLiteral("sceneIndexes"), sceneIndexes);
        groupJson.insert(QStringLiteral("sceneCount"), sceneIndexes.size());

        QJsonObject geometryJson;
        geometryJson.insert(QStringLiteral("x"), groupBox.x());
        geometryJson.insert(QStringLiteral("y"), groupBox.y());
        geometryJson.insert(QStringLiteral("width"), groupBox.width());
        geometryJson.insert(QStringLiteral("height"), groupBox.height());
        groupJson.insert(QStringLiteral("geometry"), geometryJson);

        groupBoxes.append(groupJson);
    }

    QJsonArray episodeBoxes;
    if (screenplay->episodeCount() > 0) {
        QMap<QString, QPair<int, QRectF>> episodeElementsMap;
        for (StructureElement *element : m_elements.list()) {
            const QString episodeName = element->scene()->episode();
            if (episodeName.isEmpty())
                continue;

            episodeElementsMap[episodeName].first++;
            episodeElementsMap[episodeName].second |=
                    QRectF(element->x(), element->y(), element->width(), element->height());
        }

        QMap<QString, QPair<int, QRectF>>::const_iterator it = episodeElementsMap.constBegin();
        QMap<QString, QPair<int, QRectF>>::const_iterator end = episodeElementsMap.constEnd();
        while (it != end) {
            QJsonObject episodeJson;
            episodeJson.insert(QStringLiteral("name"), it.key());
            episodeJson.insert(QStringLiteral("sceneCount"), it.value().first);

            const QRectF episodeBox = it.value().second;

            QJsonObject geometryJson;
            geometryJson.insert(QStringLiteral("x"), episodeBox.x());
            geometryJson.insert(QStringLiteral("y"), episodeBox.y());
            geometryJson.insert(QStringLiteral("width"), episodeBox.width());
            geometryJson.insert(QStringLiteral("height"), episodeBox.height());
            episodeJson.insert(QStringLiteral("geometry"), geometryJson);

            episodeBoxes.append(episodeJson);

            ++it;
        }
    }

    ret.insert(QStringLiteral("groupBoxes"), groupBoxes);
    ret.insert(QStringLiteral("episodeBoxes"), episodeBoxes);

    return ret;
}

QJsonObject Structure::queryBreakElements(ScreenplayElement *breakElement) const
{
    QJsonObject ret;
    if (breakElement == nullptr || breakElement->screenplay() == nullptr
        || breakElement->elementType() != ScreenplayElement::BreakElementType)
        return ret;

    /**
     * This function queries information about elements in the given breakElement.
     * In case of episode break, it includes all Structure elements included in that episode.
     * In case of act break, it only includes all Structure elements in that act.
     */

    const Screenplay *screenplay = breakElement->screenplay();
    const QList<ScreenplayElement *> allSpElements = screenplay->getElements();
    const int breakIndex = allSpElements.indexOf(breakElement);
    if (breakIndex < 0)
        return ret;

    const QList<int> breakTypes = breakElement->breakType() == Screenplay::Episode
            ? (QList<int>() << Screenplay::Episode)
            : (QList<int>() << Screenplay::Act << Screenplay::Interval);

    QRectF boundingRect;
    QJsonArray elementIndexes;
    QList<const StructureElement *> elementsInBreak;
    for (int i = breakIndex + 1; i < allSpElements.size(); i++) {
        const ScreenplayElement *selement = allSpElements.at(i);
        if (selement->elementType() == ScreenplayElement::BreakElementType) {
            if (breakTypes.contains(selement->breakType()))
                break;

            continue;
        }

        const Scene *scene = selement->scene();
        if (scene == nullptr)
            continue; // ?????

        StructureElement *structureElement = scene->structureElement();
        if (structureElement == nullptr)
            continue; // ????

        elementsInBreak << structureElement;
        elementIndexes.append(m_elements.indexOf(structureElement));

        boundingRect |= structureElement->geometry();
    }

    ret.insert(QStringLiteral("indexes"), elementIndexes);

    QJsonObject boundingBoxJson;
    boundingBoxJson.insert(QStringLiteral("x"), boundingRect.x());
    boundingBoxJson.insert(QStringLiteral("y"), boundingRect.y());
    boundingBoxJson.insert(QStringLiteral("width"), boundingRect.width());
    boundingBoxJson.insert(QStringLiteral("height"), boundingRect.height());
    ret.insert(QStringLiteral("boundingBox"), boundingBoxJson);

    return ret;
}

QList<QPair<QString, QList<StructureElement *>>>
Structure::evaluateGroupsImpl(Screenplay *screenplay, const QString &category) const
{
    QList<QPair<QString, QList<StructureElement *>>> ret;
    if (screenplay == nullptr)
        return ret;

    bool hasEpisodeBreaks = false;

    if (category.isEmpty()) {
        QList<StructureElement *> unusedElements = m_elements.list();

        ret.append(qMakePair(QString(), QList<StructureElement *>()));

        for (int i = 0; i < screenplay->elementCount(); i++) {
            ScreenplayElement *element = screenplay->elementAt(i);
            if (element->elementType() == ScreenplayElement::BreakElementType) {
                hasEpisodeBreaks |= Screenplay::Episode == element->breakType();

                const QString beatName =
                        element->breakType() == Screenplay::Act ? element->breakTitle() : QString();
                ret.append(qMakePair(beatName, QList<StructureElement *>()));
            } else {
                Scene *scene = element->scene();
                int index = this->indexOfScene(scene);
                StructureElement *selement = this->elementAt(index);
                if (selement != nullptr) {
                    unusedElements.removeOne(selement);
                    ret.last().second.append(selement);

                    if (ret.last().first.isEmpty())
                        ret.last().first =
                                QStringLiteral("ACT ") + QString::number(element->actIndex() + 1);
                }
            }
        }

        if (!unusedElements.isEmpty())
            ret.append(qMakePair(QStringLiteral("Unused Scenes"), unusedElements));
    } else {
        auto filteredGroups = [category](const QStringList &groups) {
            QStringList ret;
            const QString slash = QStringLiteral("/");
            for (const QString &group : groups) {
                if (group.startsWith(category, Qt::CaseInsensitive)) {
                    const QString groupName =
                            Application::instance()->camelCased(group.section(slash, 1));
                    ret.append(groupName);
                }
            }
            return ret;
        };

        QHash<Scene *, int> sceneIndexMap;
        QHash<Scene *, int> actIndexMap;

        QMap<QString, QList<StructureElement *>> map;
        for (int i = 0; i < screenplay->elementCount(); i++) {
            ScreenplayElement *element = screenplay->elementAt(i);
            if (element->elementType() == ScreenplayElement::BreakElementType) {
                hasEpisodeBreaks |= Screenplay::Episode == element->breakType();
                continue;
            }

            Scene *scene = element->scene();
            const QStringList sceneGroups = filteredGroups(scene->groups());
            if (sceneGroups.isEmpty())
                continue;

            sceneIndexMap[scene] = element->elementIndex();
            actIndexMap[scene] = element->actIndex();
            const int index = this->indexOfScene(scene);
            StructureElement *selement = this->elementAt(index);
            if (selement != nullptr) {
                for (const QString &group : sceneGroups)
                    map[group].append(selement);
            }
        }

        QMap<QString, QList<StructureElement *>>::const_iterator it = map.constBegin();
        QMap<QString, QList<StructureElement *>>::const_iterator end = map.constEnd();
        while (it != end) {
            QList<StructureElement *> selements = it.value();
            std::sort(selements.begin(), selements.end(),
                      [sceneIndexMap](StructureElement *a, StructureElement *b) {
                          return sceneIndexMap.value(a->scene()) < sceneIndexMap.value(b->scene());
                      });

            QList<StructureElement *> bunch;
            for (StructureElement *selement : qAsConst(selements)) {
                if (bunch.isEmpty()
                    || (sceneIndexMap.value(selement->scene())
                                        - sceneIndexMap.value(bunch.last()->scene())
                                == 1
                        && actIndexMap.value(selement->scene())
                                == actIndexMap.value(bunch.last()->scene()))) {
                    bunch.append(selement);
                    continue;
                }

                ret.append(qMakePair(it.key(), bunch));
                bunch.clear();
                bunch.append(selement);
            }

            if (!bunch.isEmpty())
                ret.append(qMakePair(it.key(), bunch));

            ++it;
        }

        std::sort(ret.begin(), ret.end(),
                  [](const QPair<QString, QList<StructureElement *>> &a,
                     const QPair<QString, QList<StructureElement *>> &b) {
                      return a.second.size() > b.second.size();
                  });

        QList<StructureElement *> unusedElements = m_elements.list();
        for (int i = unusedElements.size() - 1; i >= 0; i--) {
            StructureElement *element = unusedElements.at(i);
            if (!element->scene() || element->scene()->isAddedToScreenplay())
                unusedElements.removeAt(i);
        }

        if (!unusedElements.isEmpty())
            ret.append(qMakePair(QStringLiteral("Unused Scenes"), unusedElements));
    }

    for (int i = ret.size() - 1; i >= 0; i--) {
        QPair<QString, QList<StructureElement *>> &item = ret[i];
        if (item.second.isEmpty())
            ret.removeAt(i);
        else {
            if (hasEpisodeBreaks) {
                const Scene *scene = item.second.first()->scene();
                const int episodeNr = scene->episodeIndex() + 1;
                item.first = QStringLiteral("EP %1: %2").arg(episodeNr).arg(item.first);
            }

            item.first = Application::instance()->camelCased(item.first);
        }
    }

    return ret;
}

bool Structure::renameCharacter(const QString &from, const QString &to, QString *errMsg)
{
    auto setError = [=](const QString &msg) {
        if (errMsg)
            *errMsg = msg;
    };

    // Run basic sanity checks on parameters
    if (from.isEmpty() || to.isEmpty()) {
        setError(QStringLiteral("Cannot rename empty character names."));
        return false;
    }

    if (from == to)
        return true;

    // Format character names properly
    const QString from2 = Application::camelCased(from.trimmed());
    const QString to2 = Application::camelCased(to.trimmed());

    // Make sure that the from character exists.
    if (!m_characterElementMap.containsCharacter(from2) && !this->findCharacter(from2)) {
        setError(QStringLiteral("Character name '%1' doesnt exist.").arg(from2));
        return false;
    }

#if 0
    // Make sure that the to character doesnt exist
    if (m_characterElementMap.containsCharacter(to2) || this->findCharacter(to2)) {
        setError(QStringLiteral("Character name '%1' already exists.").arg(to2));
        return false;
    }
#endif

    // Replace character names in all scenes
    for (StructureElement *element : m_elements.constList())
        element->renameCharacter(from2, to2);

    // Rename in notes
    if (m_notes)
        m_notes->renameCharacter(from2, to2);

    return true;
}

void Structure::scanForMuteCharacters()
{
    m_scriteDocument->setBusyMessage("Scanning for mute characters..");

    const QStringList characterNames = this->characterNames();
    for (StructureElement *element : m_elements.constList())
        element->scene()->scanMuteCharacters(characterNames);

    m_scriteDocument->clearBusyMessage();
}

QStringList Structure::standardLocationTypes()
{
    static const QStringList list = { QStringLiteral("INT"), QStringLiteral("EXT"),
                                      QStringLiteral("I/E") };
    return list;
}

QStringList Structure::standardMoments()
{
    static const QStringList list = { QStringLiteral("DAY"),
                                      QStringLiteral("NIGHT"),
                                      QStringLiteral("MORNING"),
                                      QStringLiteral("AFTERNOON"),
                                      QStringLiteral("EVENING"),
                                      QStringLiteral("LATER"),
                                      QStringLiteral("MOMENTS LATER"),
                                      QStringLiteral("CONTINUOUS"),
                                      QStringLiteral("THE NEXT DAY"),
                                      QStringLiteral("EARLIER"),
                                      QStringLiteral("MOMENTS EARLIER"),
                                      QStringLiteral("THE PREVIOUS DAY"),
                                      QStringLiteral("DAWN"),
                                      QStringLiteral("DUSK") };
    return list;
}

void Structure::setCurrentElementIndex(int val)
{
    val = qBound(-1, val, m_elements.size() - 1);
    if (m_currentElementIndex == val)
        return;

    m_currentElementIndex = val;
    emit currentElementIndexChanged();
}

void Structure::setZoomLevel(qreal val)
{
    if (qFuzzyCompare(m_zoomLevel, val))
        return;

    m_zoomLevel = val;
    emit zoomLevelChanged();
}

QStringList Structure::filteredCharacterNames(const QStringList &tags) const
{
    if (tags.isEmpty())
        return m_characterNames;

    QSet<QString> ret;

    const QList<Character *> characters = m_characters.list();
    for (Character *character : characters) {
        const QString name = character->name();
        for (const QString &tag : tags) {
            if (character->hasTag(tag)) {
                ret += name;
                break;
            }
        }
    }

    return ret.values();
}

QQmlListProperty<Annotation> Structure::annotations()
{
    return QQmlListProperty<Annotation>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &Structure::staticAppendAnnotation, &Structure::staticAnnotationCount,
            &Structure::staticAnnotationAt, &Structure::staticClearAnnotations);
}

static void structureAppendAnnotation(Structure *structure, Annotation *ptr)
{
    structure->addAnnotation(ptr);
}
static void structureRemoveAnnotation(Structure *structure, Annotation *ptr)
{
    structure->removeAnnotation(ptr);
}
static void structureInsertAnnotation(Structure *structure, Annotation *ptr, int)
{
    structure->addAnnotation(ptr);
}
static Annotation *structureAnnotationAt(Structure *structure, int index)
{
    return structure->annotationAt(index);
}
static int structureIndexOfAnnotation(Structure *, Annotation *)
{
    return -1;
}

void Structure::addAnnotation(Annotation *ptr)
{
    if (ptr == nullptr || m_annotations.indexOf(ptr) >= 0)
        return;

    QScopedPointer<PushObjectListCommand<Structure, Annotation>> cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "annotations");
    if (!info->isLocked() && /* DISABLES CODE */ (false)) {
        ObjectListPropertyMethods<Structure, Annotation> methods(
                &structureAppendAnnotation, &structureRemoveAnnotation, &structureInsertAnnotation,
                &structureAnnotationAt, structureIndexOfAnnotation);
        cmd.reset(new PushObjectListCommand<Structure, Annotation>(
                ptr, this, info->property, ObjectList::InsertOperation, methods));
    }

    m_annotations.append(ptr);

    ptr->setParent(this);
    connect(ptr, &Annotation::aboutToDelete, this, &Structure::removeAnnotation);
    connect(ptr, &Annotation::annotationChanged, this, &Structure::structureChanged);
    connect(ptr, &Annotation::geometryChanged, &m_annotations,
            &QObjectListModel<Annotation *>::objectChanged);
    connect(ptr, &Annotation::aboutToDelete, &m_annotations,
            &QObjectListModel<Annotation *>::objectDestroyed);

    emit annotationCountChanged();
}

void Structure::removeAnnotation(Annotation *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_annotations.indexOf(ptr);
    if (index < 0)
        return;

    QScopedPointer<PushObjectListCommand<Structure, Annotation>> cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "annotations");
    if (!info->isLocked() && /* DISABLES CODE */ (false)) {
        ObjectListPropertyMethods<Structure, Annotation> methods(
                &structureAppendAnnotation, &structureRemoveAnnotation, &structureInsertAnnotation,
                &structureAnnotationAt, structureIndexOfAnnotation);
        cmd.reset(new PushObjectListCommand<Structure, Annotation>(
                ptr, this, info->property, ObjectList::RemoveOperation, methods));
    }

    m_annotations.removeAt(index);

    disconnect(ptr, &Annotation::aboutToDelete, this, &Structure::removeAnnotation);
    disconnect(ptr, &Annotation::annotationChanged, this, &Structure::structureChanged);
    disconnect(ptr, &Annotation::geometryChanged, &m_annotations,
               &QObjectListModel<Annotation *>::objectChanged);
    disconnect(ptr, &Annotation::aboutToDelete, &m_annotations,
               &QObjectListModel<Annotation *>::objectDestroyed);

    emit annotationCountChanged();

    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

Annotation *Structure::annotationAt(int index) const
{
    return index < 0 || index >= m_annotations.size() ? nullptr : m_annotations.at(index);
}

bool Structure::canBringToFront(Annotation *ptr) const
{
    const int idx = ptr ? m_annotations.indexOf(ptr) : -1;
    return idx >= 0 && idx < m_annotations.size() - 1;
}

bool Structure::canSendToBack(Annotation *ptr) const
{
    const int idx = ptr ? m_annotations.indexOf(ptr) : -1;
    return idx >= 1;
}

void Structure::bringToFront(Annotation *ptr)
{
    if (ptr == nullptr || m_annotations.empty())
        return;

    if (m_annotations.last() == ptr)
        return;

    const int index = m_annotations.indexOf(ptr);
    if (index < 0)
        return;

    m_annotations.takeAt(index);
    m_annotations.append(ptr);
    emit annotationCountChanged(); // Although the count did not change, we use the same
                                   // signal to announce change in the annotations list property
}

void Structure::sendToBack(Annotation *ptr)
{
    if (ptr == nullptr || m_annotations.empty())
        return;

    if (m_annotations.first() == ptr)
        return;

    const int index = m_annotations.indexOf(ptr);
    if (index < 0)
        return;

    m_annotations.takeAt(index);
    m_annotations.prepend(ptr);
    emit annotationCountChanged(); // Although the count did not change, we use the same
    // signal to announce change in the annotations list property
}

void Structure::setAnnotations(const QList<Annotation *> &list)
{
    if (!m_annotations.isEmpty() || list.isEmpty())
        return;

    // We dont have to capture this as an undoable action, because this method
    // is only called as a part of loading the Structure. What's the point in
    // undoing a Structure loaded from file.

    for (Annotation *ptr : list) {
        ptr->setParent(this);
        connect(ptr, &Annotation::aboutToDelete, this, &Structure::removeAnnotation);
        connect(ptr, &Annotation::annotationChanged, this, &Structure::structureChanged);
        connect(ptr, &Annotation::geometryChanged, &m_annotations,
                &QObjectListModel<Annotation *>::objectChanged);
        connect(ptr, &Annotation::aboutToDelete, &m_annotations,
                &QObjectListModel<Annotation *>::objectDestroyed);
    }

    m_annotations.assign(list);
    emit annotationCountChanged();
}

void Structure::clearAnnotations()
{
    while (m_annotations.size())
        this->removeAnnotation(m_annotations.first());
}

QString Structure::defaultGroupsDataFile() const
{
    static const QString ret = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QStringLiteral("/structure_categories_and_groups.lst");
    return ret;
}

void Structure::loadDefaultGroupsData()
{
    static const QString groupsListFileName = this->defaultGroupsDataFile();

    if (!QFile::exists(groupsListFileName)) {
        QFile inFile(QStringLiteral(":/misc/structure_groups.lst"));
        inFile.open(QFile::ReadOnly);
        const QByteArray inFileData = inFile.readAll();
        QFile outFile(groupsListFileName);
        if (outFile.open(QFile::WriteOnly))
            outFile.write(inFileData);
    }

    auto reloadGroupsListFile = [=]() {
        QFile groupsListFile(groupsListFileName);
        if (groupsListFile.open(QFile::ReadOnly)) {
            const QString groupsData = groupsListFile.readAll();
            this->setGroupsData(groupsData);
        }
    };
    reloadGroupsListFile();

    QFileSystemWatcher *groupsListFileWatcher = this->findChild<QFileSystemWatcher *>();
    if (groupsListFileWatcher != nullptr)
        delete groupsListFileWatcher;

    groupsListFileWatcher = new QFileSystemWatcher(this);
    groupsListFileWatcher->addPath(groupsListFileName);
    connect(groupsListFileWatcher, &QFileSystemWatcher::fileChanged, reloadGroupsListFile);
}

/**
Groups Data is a text written in the following format.

[Save The Cat]: description
Opening Image: description
Setup
- Theme Stated
Catalyst
Debate
Break Into Two
B Story
Fun And Games
Midpoint
Bad Guys Close In
All Is Lost
Dark Knight Of The Soul
Break Into Three
Finale
- Gathering The Team
- Executing The Plan
- High Tower Surprise
- Dig Deep Down
- Execution Of The New Plan
Final Image

[A Story]
Beginning
Middle
End

[B Story]
Beginning
Middle
End

The text written in square brackets [] creates a category of groups. Everything under that
will be considered to belong that that category, until another [] is found.

Each line in a category creates a group name. Lines that start with a - create a subgroup.
Top level group names are used for visually laying out index cards on the canvas. Subgroups
are only used for annotations.

Anything that comes after : in a line is a description of that group or category.

Groups data can be created separately for each Scrite document. A default one is loaded from
groups.lst file in the same path as Application.settingsFilePath
 */
void Structure::setGroupsData(const QString &val)
{
    if (m_groupsData == val)
        return;

    struct CategoryOrGroup
    {
        QString name;
        QString label;
        QString desc;
        QString act;
        int type = -1; // -1 = category, 0 = visual group, 1 = sub-group

        CategoryOrGroup() { }
        CategoryOrGroup(const QString &name, const QString &desc = QString())
        {
            this->label = name.simplified();
            this->name = this->label.toUpper();
            this->desc = desc.simplified();
        }

        bool isValid() const { return !name.isEmpty(); }

        bool operator<(const CategoryOrGroup &other) const { return name < other.name; }

        bool operator==(const CategoryOrGroup &other) const { return name == other.name; }

        QJsonObject toJson(const QString &namePrefix = QString()) const
        {
            QJsonObject ret;
            ret.insert(QStringLiteral("name"),
                       namePrefix.isEmpty() ? this->name
                                            : (namePrefix + QStringLiteral("/") + this->name));
            ret.insert(QStringLiteral("label"), this->label);
            ret.insert(QStringLiteral("desc"), this->desc);
            if (this->type >= 0) {
                ret.insert(QStringLiteral("type"), this->type);
                ret.insert("act", this->act);
            }
            return ret;
        }

        QString toString() const
        {
            QString ret;
            QTextStream ts(&ret, QIODevice::WriteOnly);
            if (type < 0)
                ts << QStringLiteral("[") << this->label << QStringLiteral("]");
            else {
                if (!act.isEmpty())
                    ts << QStringLiteral("<") + this->act << QStringLiteral(">");
                if (type > 0)
                    ts << QStringLiteral("- ") << this->label;
                else
                    ts << this->label;
            }
            if (!this->desc.isEmpty())
                ts << QStringLiteral(": ") << this->desc;
            ts.flush();
            return ret;
        }
    };
    typedef CategoryOrGroup Category;
    typedef CategoryOrGroup Group;

    const QString sqbo = QStringLiteral("[");
    const QString sqbc = QStringLiteral("]");
    const QString dash = QStringLiteral("- ");
    const QString colon = QStringLiteral(":");
    const QString newl = QStringLiteral("\n");
    const QString abo = QStringLiteral("<");
    const QString abc = QStringLiteral(">");

    auto fromCategoryLine = [=](const QString &line) -> Category {
        Category ret;

        if (line.isEmpty())
            return ret;

        if (!line.startsWith(sqbo))
            return ret;

        const int closingBraceIndex = line.indexOf(sqbc);
        if (closingBraceIndex < 0)
            return ret;

        const QString name = line.mid(1, closingBraceIndex - 1);
        const QString desc = line.section(colon, 1);
        ret = CategoryOrGroup(name, desc);

        return ret;
    };

    auto fromGroupLine = [=](const QString &line) -> Group {
        Group ret;

        if (line.isEmpty())
            return ret;

        QString line2 = line;
        const int type = line.startsWith(dash) ? 1 : 0;
        if (type == 1)
            line2 = line.mid(2).trimmed();

        const QString field1 = line2.section(colon, 0, 0);
        const QString act =
                field1.startsWith("<") ? field1.mid(1).section(abc, 0, 0).trimmed() : QString();
        const QString name = act.isEmpty() ? field1 : field1.section(abc, 1);
        const QString desc = line2.section(colon, 1);
        ret = CategoryOrGroup(name, desc);
        ret.type = type;
        ret.act = act.toUpper();

        return ret;
    };

    QMap<Category, QList<Group>> categoryGroupsMap;

    // Parse the text and evaluate groups in it
    {
        QString val2 = val;
        QTextStream ts(&val2, QIODevice::ReadOnly);

        Category activeCategory(QStringLiteral("Default Category"));

        while (!ts.atEnd()) {
            const QString line = ts.readLine().trimmed();
            if (line.isEmpty())
                continue;

            if (line.startsWith(sqbo))
                activeCategory = fromCategoryLine(line);
            else {
                const Group group = fromGroupLine(line);
                categoryGroupsMap[activeCategory].append(group);
            }
        }
    }

    // Polish the text and write properly
    m_groupsModel = QJsonArray();
    m_groupsData.clear();

    QTextStream ts(&m_groupsData, QIODevice::WriteOnly);

    QMap<Category, QList<Group>>::iterator it = categoryGroupsMap.begin();
    QMap<Category, QList<Group>>::iterator end = categoryGroupsMap.end();
    while (it != end) {
        const Category category = it.key();
        const QList<Group> groups = it.value();
        ts << category.toString() << newl;

        for (const Group &group : groups) {
            QJsonObject groupJsonItem = group.toJson(category.name);
            groupJsonItem.insert(QStringLiteral("category"), category.label);
            m_groupsModel.append(groupJsonItem);

            ts << group.toString() << newl;
        }

        ts << newl;

        ++it;
    }

    ts.flush();

    const QList<Category> categories = categoryGroupsMap.keys();
    m_groupCategories.clear();
    m_categoryActNames.clear();

    for (const Category &category : categories) {
        m_groupCategories.append(category.name);

        const QList<Group> &groupList = categoryGroupsMap.value(category);
        QStringList acts;
        for (const Group &group : groupList) {
            if (group.act.isEmpty())
                continue;

            if (!acts.contains(group.act))
                acts.append(group.act);
        }

        m_categoryActNames[category.name] = acts;
    }

    emit groupsDataChanged();
    emit groupsModelChanged();

    QFileSystemWatcher *groupsListFileWatcher = this->findChild<QFileSystemWatcher *>();
    if (groupsListFileWatcher != nullptr)
        delete groupsListFileWatcher;

#if 0
    QFile outFile1( QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
                   QStringLiteral("/groups.json") );
    outFile1.open( QFile::WriteOnly );
    outFile1.write( QJsonDocument(m_groupsModel).toJson() );

    QFile outFile2( QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
                   QStringLiteral("/groups.lst") );
    outFile2.open( QFile::WriteOnly );
    outFile2.write( m_groupsData.toLatin1() );
#endif
}

void Structure::setPreferredGroupCategory(const QString &val)
{
    if (m_preferredGroupCategory == val)
        return;

    m_preferredGroupCategory = val;
    emit preferredGroupCategoryChanged();
}

QString Structure::presentableGroupNames(const QStringList &groups) const
{
    const QString slash = QStringLiteral("/");

    QMap<QString, QStringList> map;
    for (const QString &group : groups) {
        const QString categoryName = group.section(slash, 0, 0);
        const QString groupName = Application::instance()->camelCased(group.section(slash, 1));
        map[categoryName].append(groupName);
    }

    QMap<QString, QStringList>::iterator it = map.begin();
    QMap<QString, QStringList>::iterator end = map.end();
    QString ret;
    while (it != end) {
        if (!ret.isEmpty())
            ret += QStringLiteral("<br/>");
        ret += QStringLiteral("<b>") + Application::instance()->camelCased(it.key())
                + QStringLiteral(":</b> ") + it.value().join(QStringLiteral(", "));
        ++it;
    }

    return ret;
}

inline static QJsonArray cleanupIndexCardFieldsArray(const QJsonArray &val)
{
    QJsonArray ret;

    for (int i = 0; i < val.size(); i++) {
        const QJsonObject item = val.at(i).toObject();
        if (item.empty())
            continue;

        const QString name = item.value("name").toString().trimmed();
        if (name.isEmpty())
            continue;

        ret.append(item);
    }

    return ret;
}

void Structure::setIndexCardFields(const QJsonArray &val)
{
    const QJsonArray val2 = cleanupIndexCardFieldsArray(val);
    if (m_indexCardFields == val2)
        return;

    m_indexCardFields = val2;
    emit indexCardFieldsChanged();
}

void Structure::setDefaultIndexCardFields(const QJsonArray &val)
{
    const QJsonArray val2 = cleanupIndexCardFieldsArray(val);
    if (m_defaultIndexCardFields == val2)
        return;

    m_defaultIndexCardFields = val2;
    emit defaultIndexCardFieldsChanged();

    const QByteArray bytes = QJsonDocument(val2).toJson();
    Application::writeToFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                     + QStringLiteral("/index_card_fields.json"),
                             bytes);
}

void Structure::loadDefaultIndexCardFields()
{
    const QByteArray bytes = Application::fileContents(QStandardPaths::writableLocation(
                                                               QStandardPaths::AppDataLocation)
                                                       + QStringLiteral("/index_card_fields.json"))
                                     .toLatin1();

    m_defaultIndexCardFields = QJsonDocument::fromJson(bytes).array();
    emit defaultIndexCardFieldsChanged();
}

Annotation *Structure::createAnnotation(const QString &type)
{
    Annotation *ret = new Annotation(this);
    ret->setType(type);
    return ret;
}

void Structure::copy(QObject *elementOrAnnotation)
{
    if (elementOrAnnotation == nullptr)
        return;

    StructureElement *element = qobject_cast<StructureElement *>(elementOrAnnotation);
    Annotation *annotation = element ? nullptr : qobject_cast<Annotation *>(elementOrAnnotation);
    if (element != nullptr || annotation != nullptr) {
        QClipboard *clipboard = qApp->clipboard();

        QJsonObject objectJson = QObjectSerializer::toJson(elementOrAnnotation);
        if (element != nullptr) {
            QJsonObject sceneJson = objectJson.value(QStringLiteral("scene")).toObject();
            sceneJson.remove(QStringLiteral("id"));
            objectJson.insert(QStringLiteral("scene"), sceneJson);
        } else if (annotation != nullptr) {
            if (annotation->type() == QStringLiteral("image")) {
                const QString attrKey = QStringLiteral("attributes");
                const QString imageKey = QStringLiteral("image");

                QJsonObject attrJson = objectJson.value(attrKey).toObject();
                QString imageVal = attrJson.value(imageKey).toString();

                imageVal = [imageVal]() {
                    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
                    const QByteArray imageBytes = dfs->read(imageVal);
                    return QStringLiteral("data://base64:") + imageBytes.toBase64();
                }();

                attrJson.insert(imageKey, imageVal);
                objectJson.insert(attrKey, attrJson);
            }
        }

        QJsonObject clipboardJson;
        clipboardJson.insert(QStringLiteral("class"),
                             QString::fromLatin1(elementOrAnnotation->metaObject()->className()));
        clipboardJson.insert(QStringLiteral("data"), objectJson);
        clipboardJson.insert(QStringLiteral("app"),
                             qApp->applicationName() + QStringLiteral("-")
                                     + qApp->applicationVersion());
        clipboardJson.insert(QStringLiteral("source"), QStringLiteral("Structure"));

        const QByteArray clipboardText = QJsonDocument(clipboardJson).toJson();

        QMimeData *mimeData = new QMimeData;
        mimeData->setData(QStringLiteral("scrite/structure"), clipboardText);

        if (element != nullptr) {
            Fountain::Body fBody;
            Fountain::populateBody(element->scene(), fBody);
            mimeData->setText(Fountain::Writer(fBody).toString());
        }

        clipboard->setMimeData(mimeData);
        return;
    }
}

static inline QJsonObject fetchPasteDataFromClipboard(QString *className = nullptr)
{
    QJsonObject ret;

    ScriteDocument *sdoc = ScriteDocument::instance();
    if (sdoc->isReadOnly())
        return ret;

    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData == nullptr)
        return ret;

    QJsonObject data;

    const QString structureMimeType = QStringLiteral("scrite/structure");
    if (mimeData->hasFormat(structureMimeType)) {
        const QByteArray clipboardText = mimeData->data(structureMimeType);
        if (clipboardText.isEmpty())
            return ret;

        QJsonParseError parseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(clipboardText, &parseError);
        if (parseError.error != QJsonParseError::NoError)
            return ret;

        const QString appString =
                qApp->applicationName() + QStringLiteral("-") + qApp->applicationVersion();

        const QJsonObject clipboardJson = jsonDoc.object();
        if (clipboardJson.value(QStringLiteral("app")).toString() != appString)
            return ret; // We dont want to support copy/paste between different versions of Scrite.

        if (clipboardJson.value(QStringLiteral("source")).toString() != QStringLiteral("Structure"))
            return ret;

        data = clipboardJson.value("data").toObject();
        if (!data.isEmpty() && className)
            *className = clipboardJson.value(QStringLiteral("class")).toString();
    } else {
        const QString text = mimeData->hasText() ? mimeData->text() : QString();
        const QUrl url = mimeData->hasUrls() ? [mimeData]() {
            const QList<QUrl> urls = mimeData->urls();
            return urls.first();
        }() : QUrl(text);

        if (mimeData->hasImage()) {
            const QImage image = mimeData->imageData().value<QImage>();

            QByteArray imageBytes;
            QBuffer buffer(&imageBytes);
            buffer.open(QBuffer::WriteOnly);
            image.save(&buffer, "JPG");
            buffer.close();
            imageBytes = imageBytes.toBase64();

            const QSize imageSize = image.size().scaled(320, 320, Qt::KeepAspectRatio);

            const QString json = QStringLiteral("{"
                                                "    \"attributes\": {"
                                                "        \"backgroundColor\": \"white\","
                                                "        \"borderColor\": \"black\","
                                                "        \"borderWidth\": 0,"
                                                "        \"caption\": \"\","
                                                "        \"captionAlignment\": \"center\","
                                                "        \"captionColor\": \"black\","
                                                "        \"fillBackground\": false,"
                                                "        \"image\": \"data://base64:%1\","
                                                "        \"opacity\": 100"
                                                "    },"
                                                "    \"geometry\": {"
                                                "        \"height\": %2,"
                                                "        \"width\": %3,"
                                                "        \"x\": 5000,"
                                                "        \"y\": 5000"
                                                "    },"
                                                "    \"type\": \"image\""
                                                "}")
                                         .arg(imageBytes.constData())
                                         .arg(imageSize.height())
                                         .arg(imageSize.width());
            data = QJsonDocument::fromJson(json.toUtf8()).object();
            if (className)
                *className = QLatin1String(Annotation::staticMetaObject.className());
        } else if (url.isValid()) {
            const QString json = QStringLiteral("{"
                                                "    \"attributes\": {"
                                                "        \"url\": \"%1\""
                                                "    },"
                                                "    \"geometry\": {"
                                                "        \"height\": 350,"
                                                "        \"width\": 300,"
                                                "        \"x\": 5000,"
                                                "        \"y\": 5000"
                                                "    },"
                                                "    \"type\": \"url\""
                                                "}")
                                         .arg(url.toString());
            data = QJsonDocument::fromJson(json.toUtf8()).object();
            if (className)
                *className = QLatin1String(Annotation::staticMetaObject.className());
        } else if (mimeData->hasText()) {
            const QFont font = []() {
                QFont ret = qApp->font();
                ret.setBold(true);
                ret.setPointSize(24);
                return ret;
            }();
            const qreal textWidth = 400;
            const qreal textHeight = [textWidth, font, mimeData]() {
                QTextDocument doc;
                doc.setDefaultFont(font);
                doc.setPlainText(mimeData->text());
                doc.setTextWidth(textWidth);
                return doc.size().height();
            }();
            const QString json = QStringLiteral("{"
                                                "    \"attributes\": {"
                                                "        \"backgroundColor\": \"white\","
                                                "        \"borderColor\": \"black\","
                                                "        \"borderWidth\": 0,"
                                                "        \"fillBackground\": false,"
                                                "        \"fontFamily\": \"%1\","
                                                "        \"fontSize\": %2,"
                                                "        \"fontStyle\": ["
                                                "            \"bold\""
                                                "        ],"
                                                "        \"hAlign\": \"center\","
                                                "        \"opacity\": 100,"
                                                "        \"text\": \"%3\","
                                                "        \"textColor\": \"black\","
                                                "        \"vAlign\": \"center\""
                                                "    },"
                                                "    \"geometry\": {"
                                                "        \"height\": %4,"
                                                "        \"width\": %5,"
                                                "        \"x\": 5000,"
                                                "        \"y\": 5000"
                                                "    },"
                                                "    \"type\": \"text\""
                                                "}")
                                         .arg(font.family())
                                         .arg(font.pointSize())
                                         .arg(mimeData->text())
                                         .arg(qCeil(textHeight))
                                         .arg(qCeil(textWidth));
            data = QJsonDocument::fromJson(json.toUtf8()).object();
            if (className)
                *className = QLatin1String(Annotation::staticMetaObject.className());
        }
    }

    return data;
}

void Structure::paste(const QPointF &pos)
{
    QString className;
    QJsonObject data = fetchPasteDataFromClipboard(&className);
    if (data.isEmpty())
        return;

    QObjectFactory factory;
    factory.addClass<Annotation>();
    factory.addClass<StructureElement>();

    QObject *object = factory.create(className.toLatin1(), this);
    if (object == nullptr)
        return;

    bool success = QObjectSerializer::fromJson(data, object);
    if (!success) {
        delete object;
        return;
    }

    Annotation *annotation = qobject_cast<Annotation *>(object);
    if (annotation != nullptr) {
        annotation->createCopyOfFileAttributes();
        annotation->setObjectName(QStringLiteral("ica"));

        QRectF geometry = annotation->geometry();

        if (pos.isNull())
            geometry.moveTopLeft(geometry.topLeft() + QPointF(50, 50));
        else
            geometry.moveCenter(pos);

        annotation->setGeometry(geometry);

        this->addAnnotation(annotation);

        // We copy the newly pasted annotation once more, so that the next paste will
        // happen relative to the newly pasted annotation
        this->copy(annotation);
        return;
    }

    StructureElement *element = qobject_cast<StructureElement *>(object);
    if (element != nullptr) {
        if (pos.isNull())
            element->setPosition(element->position() + QPointF(50, 50));
        else
            element->setPosition(pos);

        this->addElement(element);

        // We copy the newly pasted annotation once more, so that the next paste will
        // happen relative to the newly pasted element
        this->copy(element);

        // Make the newly pasted element as the current item.
        this->setCurrentElementIndex(this->indexOfElement(element));

        return;
    }
}

void Structure::setCharacterRelationshipGraph(const QJsonObject &val)
{
    if (m_characterRelationshipGraph == val)
        return;

    m_characterRelationshipGraph = val;
    emit characterRelationshipGraphChanged();
}

StructureExporter *Structure::createExporter()
{
    StructureExporter *exporter = new StructureExporter(this);
    m_scriteDocument->setupExporter(exporter);
    return exporter;
}

QObject *Structure::createExporterObject()
{
    return this->createExporter();
}

void Structure::prepareForDeserialization()
{
    m_deserializationStage = BeingDeserialized;

    m_indexCardFields = QJsonArray();
}

void Structure::serializeToJson(QJsonObject &) const { }

void Structure::deserializeFromJson(const QJsonObject &json)
{
    for (Character *character : m_characters.constList())
        character->resolveRelationships();

    // Forward and reverse relationships must be a tuple. If one is deleted, the other must
    // get deleted right away. We cannot afford to have zombie relationships.
    for (Character *character : m_characters.constList()) {
        for (int i = 0; i < character->relationshipCount(); i++) {
            Relationship *ofWith = character->relationshipAt(i);
            if (ofWith->direction() == Relationship::OfWith) {
                Relationship *withOf = ofWith->with()->findRelationship(character);
                if (withOf == nullptr) {
                    ofWith->deleteLater();
                    continue;
                }

                if (withOf->direction() == Relationship::WithOf) {
                    // Ensure that if one of the relationships is destroyed, the other
                    // must destroy itself.
                    connect(withOf, &Relationship::aboutToDelete, ofWith,
                            &Relationship::deleteLater);
                    connect(ofWith, &Relationship::aboutToDelete, withOf,
                            &Relationship::deleteLater);
                } else {
                    ofWith->deleteLater();
                    withOf->deleteLater();
                }
            }
        }
    }

    // Unfortunately, I had made the mistake of declaring preferredGroupCategory
    // as a QStringList property. So files created using the old version would have
    // stored this property as a QStringList. We will now need to reinterpret it as
    // QString.
    const QJsonValue preferredGroupCategoryValue =
            json.value(QStringLiteral("preferredGroupCategory"));
    if (preferredGroupCategoryValue.isArray()) {
        const QJsonArray preferredGroupCategoryArray = preferredGroupCategoryValue.toArray();
        if (preferredGroupCategoryArray.size() >= 1)
            this->setPreferredGroupCategory(preferredGroupCategoryArray.at(0).toString());
    }

    // Previously notes was an array, because the notes property used to be
    // a list property. Now notes is an object, because it represents Notes class.
    // So, if we are loading a notes from a file created using older versions of Scrite,
    // we have to upgrade the notes to the newer format based on the Notes class.
    const QJsonValue notes = json.value(QStringLiteral("notes"));
    if (notes.isArray())
        m_notes->loadOldNotes(notes.toArray());

    this->updateCharacterNamesShotsTransitionsAndTagsLater();

    m_deserializationStage = JustDeserialized;
    if (json.value(QLatin1String("forceBeatBoardLayout")).toBool())
        this->setForceBeatBoardLayout(true);
    m_deserializationStage = FullyDeserialized;
}

bool Structure::canSetPropertyFromObjectList(const QString &propName) const
{
    if (propName == QStringLiteral("elements"))
        return m_elements.isEmpty();

    if (propName == QStringLiteral("characters"))
        return m_characters.isEmpty();

    return false;
}

void Structure::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if (propName == QStringLiteral("elements")) {
        this->setElements(qobject_list_cast<StructureElement *>(objects));
        m_elementStacks.evaluateStacksMuchLater(500);
        return;
    }

    if (propName == QStringLiteral("characters")) {
        this->setCharacters(qobject_list_cast<Character *>(objects));
        return;
    }
}

QStringList Structure::sortCharacterNames(const QStringList &givenNames) const
{
    if (givenNames.length() <= 1)
        return givenNames;

    QStringList names = givenNames;

    std::sort(names.begin(), names.end(), [=](const QString &a, const QString &b) {
        const Character *ac = this->findCharacter(a);
        const Character *bc = this->findCharacter(b);
        const int ap = ac ? ac->priority() : 0;
        const int bp = bc ? bc->priority() : 0;
        if (ap == bp)
            return a < b;
        return ap > bp;
    });

    return names;
}

void Structure::write(QTextCursor &cursor, const WriteOptions &options) const
{
    if (options.includeTextNotes || options.includeFormNotes) {
        if (options.charactersOnly) {
            const QList<Character *> characters = m_characters.sortedList(&Character::LessThan);
            const QStringList allCharacterNames = m_characterNames;
            const QStringList infoLessCharacterNames = [=]() -> QStringList {
                QStringList ret = allCharacterNames;
                for (const Character *character : characters)
                    ret.removeOne(character->name());
                return ret;
            }();
            const QString title = [=]() -> QString {
                QString ret = m_scriteDocument->screenplay()->title();
                if (ret.isEmpty())
                    ret = QLatin1String("Untitled Screenplay");
                return ret;
            }();

            if (allCharacterNames.isEmpty()) {
                cursor.insertText(QLatin1String("No characters found in this screenplay."));
            } else {
                // First, export all character names and known meta-data in one table.
                cursor.insertText(QLatin1String("Names of all characters:"));
                cursor.insertBlock();

                QTextFrame *frame = cursor.currentFrame();
                QTextListFormat charactersListFormat;
                charactersListFormat.setStyle(QTextListFormat::ListDecimal);
                charactersListFormat.setNumberSuffix(QLatin1String(".  "));
                QTextList *charactersList = cursor.insertList(charactersListFormat);

                for (int i = 0; i < allCharacterNames.size(); i++) {
                    const QString characterName = allCharacterNames.at(i);
                    cursor.insertText(characterName);
                    if (i != allCharacterNames.size() - 1)
                        cursor.insertBlock();
                }

                cursor = frame->lastCursorPosition();
                cursor.insertBlock();
                charactersList->remove(cursor.block());

                QTextBlockFormat pageBreakFormat;
                pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
                cursor.insertBlock(pageBreakFormat);
                cursor.insertText(QLatin1String("  "));

                // Then export one character per page.
                for (int i = 0; i < characters.size(); i++) {
                    Character *character = characters.at(i);

                    Character::WriteOptions options;
                    options.headingLevel = 2;
                    character->write(cursor, options);

                    if (i != characters.size() - 1) {
                        QTextBlockFormat pageBreakFormat;
                        pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
                        cursor.insertBlock(pageBreakFormat);
                        cursor.insertText(QLatin1String("  "));
                    }
                }
            }
        } else if (m_notes) {
            Notes::WriteOptions notesOptions;
            notesOptions.includeFormNotes = options.includeFormNotes;
            notesOptions.includeTextNotes = options.includeTextNotes;
            m_notes->write(cursor, notesOptions);
        }
    }
}

bool Structure::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange)
        m_scriteDocument = qobject_cast<ScriteDocument *>(this->parent());

    return QObject::event(event);
}

void Structure::timerEvent(QTimerEvent *event)
{
    if (m_locationHeadingsMapTimer.timerId() == event->timerId()) {
        m_locationHeadingsMapTimer.stop();
        this->updateLocationHeadingMap();
        return;
    }

    if (m_updateCharacterNamesShotsTransitionsAndTagsTimer.timerId() == event->timerId()) {
        m_updateCharacterNamesShotsTransitionsAndTagsTimer.stop();
        this->updateCharacterNamesShotsTransitionsAndTags();
        return;
    }

    QObject::timerEvent(event);
}

void Structure::resetCurentElementIndex()
{
    int val = m_currentElementIndex;
    if (m_elements.isEmpty())
        val = -1;
    else
        val = qBound(0, val, m_elements.size() - 1);
    m_currentElementIndex = -2;

    this->setCurrentElementIndex(val);
}

void Structure::setCanPaste(bool val)
{
    if (m_canPaste == val)
        return;

    m_canPaste = val;
    emit canPasteChanged();
}

void Structure::onClipboardDataChanged()
{
    const QJsonObject data = fetchPasteDataFromClipboard();
    this->setCanPaste(!data.isEmpty());
}

StructureElement *Structure::splitElement(StructureElement *ptr, SceneElement *element,
                                          int textPosition)
{
    /*
     * Never call this function directly. This function __must__ be called as a part of
     * Screenplay::splitElement() call.
     */
    if (ptr == nullptr)
        return nullptr;

    const int index = this->indexOfElement(ptr);
    if (index < 0)
        return nullptr;

    Scene *newScene = ptr->scene()->splitScene(element, textPosition);
    if (newScene == nullptr)
        return nullptr;

    StructureElement *newElement = new StructureElement(this);
    newElement->setScene(newScene);
    newElement->setX(ptr->x() + 300);
    newElement->setY(ptr->y() + 80);
    this->insertElement(newElement, index + 1);
    return newElement;
}

void Structure::staticAppendCharacter(QQmlListProperty<Character> *list, Character *ptr)
{
    reinterpret_cast<Structure *>(list->data)->addCharacter(ptr);
}

void Structure::staticClearCharacters(QQmlListProperty<Character> *list)
{
    reinterpret_cast<Structure *>(list->data)->clearCharacters();
}

Character *Structure::staticCharacterAt(QQmlListProperty<Character> *list, int index)
{
    return reinterpret_cast<Structure *>(list->data)->characterAt(index);
}

int Structure::staticCharacterCount(QQmlListProperty<Character> *list)
{
    return reinterpret_cast<Structure *>(list->data)->characterCount();
}

void Structure::staticAppendElement(QQmlListProperty<StructureElement> *list, StructureElement *ptr)
{
    reinterpret_cast<Structure *>(list->data)->addElement(ptr);
}

void Structure::staticClearElements(QQmlListProperty<StructureElement> *list)
{
    reinterpret_cast<Structure *>(list->data)->clearElements();
}

StructureElement *Structure::staticElementAt(QQmlListProperty<StructureElement> *list, int index)
{
    return reinterpret_cast<Structure *>(list->data)->elementAt(index);
}

int Structure::staticElementCount(QQmlListProperty<StructureElement> *list)
{
    return reinterpret_cast<Structure *>(list->data)->elementCount();
}

void Structure::updateLocationHeadingMap()
{
    QMap<QString, QList<SceneHeading *>> map;
    for (StructureElement *element : m_elements.constList()) {
        Scene *scene = element->scene();
        if (scene == nullptr || !scene->heading()->isEnabled())
            continue;

        const QString location = scene->heading()->location();
        if (location.isEmpty())
            continue;

        map[location].append(scene->heading());
    }

    m_locationHeadingsMap = map;
}

void Structure::updateLocationHeadingMapLater()
{
    m_locationHeadingsMapTimer.start(0, this);
}

void Structure::onStructureElementSceneChanged(StructureElement *element)
{
    if (element == nullptr)
        element = qobject_cast<StructureElement *>(this->sender());

    if (element == nullptr || element->scene() == nullptr)
        return;

    connect(element->scene(), &Scene::sceneElementChanged, this, &Structure::onSceneElementChanged);
    connect(element->scene(), &Scene::aboutToRemoveSceneElement, this,
            &Structure::onAboutToRemoveSceneElement);

    Scene *scene = element->scene();
    for (int i = 0; i < scene->elementCount(); i++) {
        SceneElement *element = scene->elementAt(i);
        if (!m_characterElementMap.include(element))
            if (!m_transitionElementMap.include(element))
                m_shotElementMap.include(element);
    }

    this->updateLocationHeadingMapLater();
    this->updateCharacterNamesShotsTransitionsAndTagsLater();
}

void Structure::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType)
{
    if (m_characterElementMap.include(element) || m_transitionElementMap.include(element)
        || m_shotElementMap.include(element))
        updateCharacterNamesShotsTransitionsAndTagsLater();
}

void Structure::onAboutToRemoveSceneElement(SceneElement *element)
{
    if (m_characterElementMap.remove(element) || m_transitionElementMap.remove(element)
        || m_shotElementMap.remove(element))
        updateCharacterNamesShotsTransitionsAndTagsLater();
}

void Structure::updateCharacterNamesShotsTransitionsAndTags()
{
    QStringList names = m_characterElementMap.characterNames();
    QSet<QString> tags;

    const QList<Character *> characters = m_characters.list();
    for (Character *character : characters) {
        const QString name = character->name();
        if (!names.contains(name))
            names.append(name);

        const QStringList ctags = character->tags();
        tags += QSet<QString>(ctags.begin(), ctags.end());
    }

    names = this->sortCharacterNames(names);
    if (names != m_characterNames) {
        m_characterNames = names;
        emit characterNamesChanged();
    }

    const QStringList tagValues = tags.values();
    if (tagValues != m_characterTags) {
        m_characterTags = tags.values();
        emit characterTagsChanged();
    }

    const QStringList shots = [=]() {
        QSet<QString> set = QSet<QString>::fromList(m_shotElementMap.shots());
        set += QSet<QString>::fromList(Scrite::defaultShots());
        QStringList ret = QStringList::fromSet(set);
        std::sort(ret.begin(), ret.end());
        return ret;
    }();
    if (shots != m_shots) {
        m_shots = shots;
        emit shotsChanged();
    }

    const QStringList transitions = [=]() {
        QSet<QString> set = QSet<QString>::fromList(m_transitionElementMap.transitions());
        set += QSet<QString>::fromList(Scrite::defaultTransitions());
        QStringList ret = QStringList::fromSet(set);
        std::sort(ret.begin(), ret.end());
        return ret;
    }();
    if (transitions != m_transitions) {
        m_transitions = transitions;
        emit transitionsChanged();
    }
}

void Structure::updateCharacterNamesShotsTransitionsAndTagsLater()
{
    m_updateCharacterNamesShotsTransitionsAndTagsTimer.start(0, this);
}

void Structure::staticAppendAnnotation(QQmlListProperty<Annotation> *list, Annotation *ptr)
{
    reinterpret_cast<Structure *>(list->data)->addAnnotation(ptr);
}

void Structure::staticClearAnnotations(QQmlListProperty<Annotation> *list)
{
    reinterpret_cast<Structure *>(list->data)->clearAnnotations();
}

Annotation *Structure::staticAnnotationAt(QQmlListProperty<Annotation> *list, int index)
{
    return reinterpret_cast<Structure *>(list->data)->annotationAt(index);
}

int Structure::staticAnnotationCount(QQmlListProperty<Annotation> *list)
{
    return reinterpret_cast<Structure *>(list->data)->annotationCount();
}

///////////////////////////////////////////////////////////////////////////////

StructureElementConnector::StructureElementConnector(QQuickItem *parent)
    : AbstractShapeItem(parent),
      m_computeConnectorShapeTimer("StructureElementConnector.m_updateTimer"),
      m_toElement(this, "toElement"),
      m_fromElement(this, "fromElement")
{
    this->setRenderType(OutlineOnly);
    this->setOutlineColor(Qt::black);

    const qreal dpr = this->window() ? this->window()->devicePixelRatio() : 1.0;
    this->setOutlineWidth(4 * dpr);

    connect(this, &AbstractShapeItem::contentRectChanged, this,
            &StructureElementConnector::updateArrowAndLabelPositions);
    connect(this, &AbstractShapeItem::contentRectChanged, this,
            &StructureElementConnector::canBeVisibleChanged);
    connect(this, &StructureElementConnector::fromElementChanged, this,
            &StructureElementConnector::canBeVisibleChanged);
    connect(this, &StructureElementConnector::toElementChanged, this,
            &StructureElementConnector::canBeVisibleChanged);
}

StructureElementConnector::~StructureElementConnector() { }

void StructureElementConnector::setLineType(StructureElementConnector::LineType val)
{
    if (m_lineType == val)
        return;

    m_lineType = val;
    emit lineTypeChanged();
}

void StructureElementConnector::setFromElement(StructureElement *val)
{
    if (m_fromElement == val)
        return;

    if (m_fromElement != nullptr) {
        disconnect(m_fromElement, &StructureElement::xChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_fromElement, &StructureElement::yChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_fromElement, &StructureElement::widthChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_fromElement, &StructureElement::heightChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_fromElement, &StructureElement::stackIdChanged, this,
                   &StructureElementConnector::canBeVisibleChanged);

        Scene *scene = m_fromElement->scene();
        disconnect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    m_fromElement = val;

    if (m_fromElement != nullptr) {
        connect(m_fromElement, &StructureElement::xChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_fromElement, &StructureElement::yChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_fromElement, &StructureElement::widthChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_fromElement, &StructureElement::heightChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_fromElement, &StructureElement::stackIdChanged, this,
                &StructureElementConnector::canBeVisibleChanged);

        Scene *scene = m_fromElement->scene();
        connect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    emit fromElementChanged();
    this->pickElementColor();
    this->computeConnectorShapeLater();
}

void StructureElementConnector::setToElement(StructureElement *val)
{
    if (m_toElement == val)
        return;

    if (m_toElement != nullptr) {
        disconnect(m_toElement, &StructureElement::xChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_toElement, &StructureElement::yChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_toElement, &StructureElement::widthChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_toElement, &StructureElement::heightChanged, this,
                   &StructureElementConnector::computeConnectorShapeLater);
        disconnect(m_toElement, &StructureElement::stackIdChanged, this,
                   &StructureElementConnector::canBeVisibleChanged);

        Scene *scene = m_toElement->scene();
        disconnect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    m_toElement = val;

    if (m_toElement != nullptr) {
        connect(m_toElement, &StructureElement::xChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_toElement, &StructureElement::yChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_toElement, &StructureElement::widthChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_toElement, &StructureElement::heightChanged, this,
                &StructureElementConnector::computeConnectorShapeLater);
        connect(m_toElement, &StructureElement::stackIdChanged, this,
                &StructureElementConnector::canBeVisibleChanged);

        Scene *scene = m_toElement->scene();
        connect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    emit toElementChanged();
    this->pickElementColor();
    this->computeConnectorShapeLater();
}

void StructureElementConnector::setArrowAndLabelSpacing(qreal val)
{
    if (qFuzzyCompare(m_arrowAndLabelSpacing, val))
        return;

    m_arrowAndLabelSpacing = val;
    emit arrowAndLabelSpacingChanged();

    this->updateArrowAndLabelPositions();
}

bool StructureElementConnector::canBeVisible() const
{
    return m_fromElement != nullptr && m_toElement != nullptr
            && (m_fromElement->stackId().isEmpty() || m_toElement->stackId().isEmpty()
                || m_fromElement->stackId() != m_toElement->stackId());
}

bool StructureElementConnector::intersects(const QRectF &rect) const
{
    if (m_fromElement == nullptr || m_toElement == nullptr)
        return false;

    auto getElementRect = [](StructureElement *e) {
        return QRectF(e->x(), e->y(), e->width(), e->height());
    };

    const QRectF r1 = getElementRect(m_fromElement);
    const QRectF r2 = getElementRect(m_toElement);
    return rect.isValid() && rect.isNull() ? rect.intersects(r1.united(r2)) : true;
}

QPainterPath StructureElementConnector::shape() const
{
    return m_connectorShape;
}

QPainterPath StructureElementConnector::curvedArrowPath(const QRectF &rect1, const QRectF &rect2,
                                                        const qreal arrowSize, bool fillArrow)
{
    auto splitBox = [](const QRectF &rect) -> QList<QRectF> {
        const qreal wb3 = rect.width() / 3;
        const qreal hb3 = rect.height() / 3;

        const qreal x0 = rect.left();
        const qreal x1 = rect.left() + wb3;
        const qreal x2 = rect.right() - wb3;
        const qreal x3 = rect.right();

        const qreal y0 = rect.top();
        const qreal y1 = rect.top() + hb3;
        const qreal y2 = rect.bottom() - hb3;
        const qreal y3 = rect.bottom();

        return QList<QRectF>({ QRectF(QPointF(x0, y0), QPointF(x1, y1)),
                               QRectF(QPointF(x1, y0), QPointF(x2, y1)),
                               QRectF(QPointF(x2, y0), QPointF(x3, y1)),
                               QRectF(QPointF(x0, y1), QPointF(x1, y2)),
                               QRectF(QPointF(x1, y1), QPointF(x2, y2)),
                               QRectF(QPointF(x2, y1), QPointF(x3, y2)),
                               QRectF(QPointF(x0, y2), QPointF(x1, y3)),
                               QRectF(QPointF(x1, y2), QPointF(x2, y3)),
                               QRectF(QPointF(x2, y2), QPointF(x3, y3)) });
    };

    static const QList<QPair<int, int>> boxPairs(
            { { 0, 8 }, { 1, 7 }, { 2, 6 }, { 5, 3 }, { 8, 0 }, { 7, 1 }, { 6, 2 }, { 3, 5 } });

    const QList<QRectF> rect1Splits = splitBox(rect1);
    const QList<QRectF> rect2Splits = splitBox(rect2);

    QRectF box1 = rect1;
    QRectF box2 = rect2;
    qreal dist = QLineF(box1.center(), box2.center()).length();
    for (auto boxPair : boxPairs) {
        const QRectF b1 = rect1Splits.at(boxPair.first);
        const QRectF b2 = rect2Splits.at(boxPair.second);
        const qreal bdist = QLineF(b1.center(), b2.center()).length();
        if (bdist < dist) {
            box1 = b1;
            box2 = b2;
            dist = bdist;
        }
    }

    QPainterPath path;
    static QString getBoxToBoxArrowJs = []() {
        QFile file(QStringLiteral(":/dragonman225-curved-arrows/getBoxToBoxArrow.js"));
        file.open(QFile::ReadOnly);
        return file.readAll();
    }();

    QString fnCallCode;
    {
        QTextStream ts(&fnCallCode, QIODevice::WriteOnly);
        ts << "getBoxToBoxArrow(";
        ts << box1.x() << ", " << box1.y() << ", " << box1.width() << ", " << box1.height() << ", "
           << box2.x() << ", " << box2.y() << ", " << box2.width() << ", " << box2.height() << ", ";
        ts << "{padStart: 0, padEnd: 0});";
    }

    QJSEngine jsEngine;

    const QString jsCode = getBoxToBoxArrowJs + QStringLiteral("\n") + fnCallCode;
    const QJSValue jsValue = jsEngine.evaluate(jsCode, QStringLiteral("getBoxToBoxArrow.js"));
    if (jsValue.isError()) {
        const QString errMsg = QStringLiteral("Uncaught exception at line ")
                + QString::number(jsValue.property(QStringLiteral("lineNumber")).toInt())
                + QStringLiteral(": ") + jsValue.toString();
        Application::log(errMsg);

        path.moveTo(box1.center());
        path.lineTo(box2.center());
    } else {
        const QPointF p1(jsValue.property(0).toNumber(), jsValue.property(1).toNumber());
        const QPointF cp1(jsValue.property(2).toNumber(), jsValue.property(3).toNumber());
        const QPointF cp2(jsValue.property(4).toNumber(), jsValue.property(5).toNumber());
        const QPointF p2(jsValue.property(6).toNumber(), jsValue.property(7).toNumber());

        path.moveTo(p1);
        path.cubicTo(cp1, cp2, p2);

        const qreal arrowPadding = 0; // arrowSize * 4 / path.length();
        const QPointF arrowPt = path.pointAtPercent(1.0 - arrowPadding);
        const qreal arrowAngle = path.angleAtPercent(1.0 - arrowPadding);

        QPolygonF polygon({ QPointF(-2 * arrowSize, -0.75 * arrowSize), QPointF(0, 0),
                            QPointF(-2 * arrowSize, 0.75 * arrowSize) });
        if (fillArrow)
            polygon.append(polygon.first());
        QTransform polygonTx;
        polygonTx.translate(arrowPt.x(), arrowPt.y());
        polygonTx.rotate(-arrowAngle);

        path.addPolygon(polygonTx.map(polygon));
    }

    return path;
}

void StructureElementConnector::timerEvent(QTimerEvent *te)
{
    if (m_computeConnectorShapeTimer.timerId() == te->timerId()) {
        m_computeConnectorShapeTimer.stop();
        this->computeConnectorShape();
        return;
    }

    AbstractShapeItem::timerEvent(te);
}

void StructureElementConnector::itemChange(ItemChange change, const ItemChangeData &data)
{
    if (change == ItemDevicePixelRatioHasChanged)
        this->setOutlineWidth(data.realValue * 4);

    QQuickItem::itemChange(change, data);
}

void StructureElementConnector::resetFromElement()
{
    m_fromElement = nullptr;
    emit fromElementChanged();
    this->pickElementColor();
    this->update();
}

void StructureElementConnector::resetToElement()
{
    m_toElement = nullptr;
    emit toElementChanged();
    this->pickElementColor();
    this->update();
}

void StructureElementConnector::computeConnectorShapeLater()
{
    m_computeConnectorShapeTimer.start(0, this);
}

void StructureElementConnector::pickElementColor()
{
    if (m_fromElement != nullptr && m_toElement != nullptr) {
        const QColor c1 = m_fromElement->scene()->color();
        const QColor c2 = m_toElement->scene()->color();
        QColor mix =
                QColor::fromRgbF((c1.redF() + c2.redF()) / 2.0, (c1.greenF() + c2.greenF()) / 2.0,
                                 (c1.blueF() + c2.blueF()) / 2.0);
        const qreal luma = ((0.299 * mix.redF()) + (0.587 * mix.greenF()) + (0.114 * mix.blueF()));
        if (luma > 0.5)
            mix = mix.darker();

        this->setOutlineColor(mix);
    }
}

void StructureElementConnector::updateArrowAndLabelPositions()
{
    const QPainterPath path = this->currentShape();
    if (path.isEmpty())
        return;

    const qreal pathLength = path.length();
    if (pathLength < 0 || qFuzzyCompare(pathLength, 0))
        return;

    const qreal arrowT = 0.5;
    const qreal labelT = 0.55 - (m_arrowAndLabelSpacing / pathLength);

    this->setArrowPosition(this->currentShape().pointAtPercent(arrowT));
    if (labelT < 0 || labelT > 1)
        this->setSuggestedLabelPosition(this->arrowPosition());
    else
        this->setSuggestedLabelPosition(this->currentShape().pointAtPercent(labelT));
}

void StructureElementConnector::setArrowPosition(const QPointF &val)
{
    if (m_arrowPosition == val)
        return;

    m_arrowPosition = val;
    emit arrowPositionChanged();
}

void StructureElementConnector::setSuggestedLabelPosition(const QPointF &val)
{
    if (m_suggestedLabelPosition == val)
        return;

    m_suggestedLabelPosition = val;
    emit suggestedLabelPositionChanged();
}

void StructureElementConnector::computeConnectorShape()
{
    if (m_fromElement == nullptr || m_toElement == nullptr) {
        m_connectorShape = QPainterPath();
        this->update();
        return;
    }

    if (!m_fromElement->stackId().isEmpty() && !m_toElement->stackId().isEmpty()
        && m_fromElement->stackId() == m_toElement->stackId()) {
        m_connectorShape = QPainterPath();
        this->update();
        return;
    }

    const QString futureWatcherName = QStringLiteral("futureWatcher");
    QFutureWatcher<QPainterPath> *futureWatcher = this->findChild<QFutureWatcher<QPainterPath> *>(
            futureWatcherName, Qt::FindDirectChildrenOnly);
    if (futureWatcher) {
        futureWatcher->cancel();
        futureWatcher->deleteLater();
    }

    auto getElementRect = [=](StructureElement *e) {
        return QRect(e->x(), e->y(), e->width(), e->height());
    };
    const QRectF r1 = getElementRect(m_fromElement);
    const QRectF r2 = getElementRect(m_toElement);
    const qreal arrowHeadSize = 6;

    if (m_lineType == StraightLine) {
        m_connectorShape.moveTo(r1.center());
        m_connectorShape.lineTo(r2.center());
        this->update();
        return;
    }

    /**
     * Evaluating JavaScript code to compute the curved line can be
     * time-consuming, especially if we have a lot of arrows. So we
     * are better off delegating the whole computation to a separate
     * thread.
     */
    futureWatcher = new QFutureWatcher<QPainterPath>(this);
    futureWatcher->setObjectName(futureWatcherName);
    connect(futureWatcher, &QFutureWatcher<QPainterPath>::finished, this, [=]() {
        if (!futureWatcher->isCanceled()) {
            m_connectorShape = futureWatcher->result();
            this->update();
        }
        futureWatcher->deleteLater();
    });
    futureWatcher->setFuture(QtConcurrent::run(&StructureElementConnector::curvedArrowPath, r1, r2,
                                               arrowHeadSize, false));
}

///////////////////////////////////////////////////////////////////////////////

StructureCanvasViewportFilterModel::StructureCanvasViewportFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_structure(this, "structure")
{
}

StructureCanvasViewportFilterModel::~StructureCanvasViewportFilterModel() { }

void StructureCanvasViewportFilterModel::setStructure(Structure *val)
{
    if (m_structure == val)
        return;

    m_structure = val;
    this->updateSourceModel();
    emit structureChanged();
}

void StructureCanvasViewportFilterModel::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    this->invalidateSelfLater();
}

void StructureCanvasViewportFilterModel::setType(StructureCanvasViewportFilterModel::Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();

    this->updateSourceModel();
}

void StructureCanvasViewportFilterModel::setViewportRect(const QRectF &val)
{
    if (m_viewportRect == val)
        return;

    m_viewportRect = val;
    emit viewportRectChanged();

    this->invalidateSelfLater();
}

void StructureCanvasViewportFilterModel::setComputeStrategy(
        StructureCanvasViewportFilterModel::ComputeStrategy val)
{
    if (m_computeStrategy == val)
        return;

    m_computeStrategy = val;
    emit computeStrategyChanged();
}

void StructureCanvasViewportFilterModel::setFilterStrategy(
        StructureCanvasViewportFilterModel::FilterStrategy val)
{
    if (m_filterStrategy == val)
        return;

    m_filterStrategy = val;
    emit filterStrategyChanged();

    this->invalidateSelfLater();
}

int StructureCanvasViewportFilterModel::mapFromSourceRow(int source_row) const
{
    if (this->sourceModel() == nullptr)
        return source_row;

    const QModelIndex source_index = this->sourceModel()->index(source_row, 0, QModelIndex());
    const QModelIndex filter_index = this->mapFromSource(source_index);
    return filter_index.row();
}

int StructureCanvasViewportFilterModel::mapToSourceRow(int filter_row) const
{
    if (this->sourceModel() == nullptr)
        return filter_row;

    const QModelIndex filter_index = this->index(filter_row, 0, QModelIndex());
    const QModelIndex source_index = this->mapToSource(filter_index);
    return source_index.row();
}

void StructureCanvasViewportFilterModel::setSourceModel(QAbstractItemModel *model)
{
    QAbstractItemModel *oldModel = this->sourceModel();
    if (oldModel != nullptr) {
        connect(oldModel, &QAbstractItemModel::rowsInserted, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(oldModel, &QAbstractItemModel::rowsRemoved, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(oldModel, &QAbstractItemModel::rowsMoved, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(oldModel, &QAbstractItemModel::dataChanged, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(oldModel, &QAbstractItemModel::modelReset, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
    }

    if (m_structure.isNull())
        this->QSortFilterProxyModel::setSourceModel(nullptr);
    else {
        if (m_type == AnnotationType && model == m_structure->annotationsModel())
            this->QSortFilterProxyModel::setSourceModel(model);
        else if (model == m_structure->elementsModel())
            this->QSortFilterProxyModel::setSourceModel(model);
        else
            this->QSortFilterProxyModel::setSourceModel(nullptr);
    }

    if (model != nullptr) {
        connect(model, &QAbstractItemModel::rowsInserted, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(model, &QAbstractItemModel::rowsRemoved, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(model, &QAbstractItemModel::rowsMoved, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(model, &QAbstractItemModel::dataChanged, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
        connect(model, &QAbstractItemModel::modelReset, this,
                &StructureCanvasViewportFilterModel::invalidateSelfLater);
    }
}

bool StructureCanvasViewportFilterModel::filterAcceptsRow(int source_row,
                                                          const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    if (!m_enabled || m_viewportRect.size().isEmpty())
        return true;

    const AbstractQObjectListModel *model =
            qobject_cast<AbstractQObjectListModel *>(this->sourceModel());
    if (model == nullptr)
        return true;

    const QObject *object = model->objectAt(source_row);
    if (m_computeStrategy == PreComputeStrategy) {
        if (source_row < 0 || source_row >= m_visibleSourceRows.size()
            || object != m_visibleSourceRows.at(source_row).first) {
            (const_cast<StructureCanvasViewportFilterModel *>(this))->invalidateSelfLater();
            return false;
        }

        return m_visibleSourceRows.at(source_row).second;
    }

    const QRectF objectRect = (m_type == AnnotationType)
            ? (qobject_cast<const Annotation *>(object))->geometry()
            : (qobject_cast<const StructureElement *>(object))->geometry();
    if (m_filterStrategy == ContainsStrategy)
        return m_viewportRect.contains(objectRect);
    return m_viewportRect.intersects(objectRect);
}

void StructureCanvasViewportFilterModel::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_invalidateTimer.timerId()) {
        m_invalidateTimer.stop();
        this->invalidateSelf();
    } else
        QObject::timerEvent(te);
}

void StructureCanvasViewportFilterModel::resetStructure()
{
    m_structure = nullptr;
    this->setSourceModel(nullptr);
    emit structureChanged();
}

void StructureCanvasViewportFilterModel::updateSourceModel()
{
    if (m_structure.isNull())
        this->setSourceModel(nullptr);
    else {
        if (m_type == AnnotationType)
            this->setSourceModel(m_structure->annotationsModel());
        else
            this->setSourceModel(m_structure->elementsModel());
    }
}

void StructureCanvasViewportFilterModel::invalidateSelf()
{
    m_visibleSourceRows.clear();
    const AbstractQObjectListModel *model = m_computeStrategy == OnDemandComputeStrategy
            ? nullptr
            : qobject_cast<AbstractQObjectListModel *>(this->sourceModel());
    if (model == nullptr || m_computeStrategy == OnDemandComputeStrategy) {
        this->invalidateFilter();
        return;
    }

    m_visibleSourceRows.reserve(model->objectCount());
    for (int i = 0; i < model->objectCount(); i++) {
        const QObject *object = model->objectAt(i);

        if (m_viewportRect.size().isEmpty())
            m_visibleSourceRows << qMakePair(object, true);
        else {
            const QRectF objectRect = (m_type == AnnotationType)
                    ? (qobject_cast<const Annotation *>(object))->geometry()
                    : (qobject_cast<const StructureElement *>(object))->geometry();
            if (m_filterStrategy == ContainsStrategy)
                m_visibleSourceRows << qMakePair(object, m_viewportRect.contains(objectRect));
            else
                m_visibleSourceRows << qMakePair(object, m_viewportRect.intersects(objectRect));
        }
    }

    this->invalidateFilter();
}

void StructureCanvasViewportFilterModel::invalidateSelfLater()
{
    if (m_enabled && m_computeStrategy == PreComputeStrategy)
        m_invalidateTimer.start(0, this);
    else
        m_invalidateTimer.stop();
}
