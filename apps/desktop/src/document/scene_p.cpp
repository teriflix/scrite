/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "scene_p.h"
#include "scritedocument.h"
#include "languageengine.h"

#include <QScopedValueRollback>

///////////////////////////////////////////////////////////////////////////////
// Static member initializations
///////////////////////////////////////////////////////////////////////////////

AbstractSceneUndoCommand *AbstractSceneUndoCommand::current = nullptr;
bool SceneElementTextUndoCommand::enabled = true;

///////////////////////////////////////////////////////////////////////////////
// SceneUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneUndoCommand::SceneUndoCommand(Scene *scene, bool allowMerging, const QString &text)
    : AbstractSceneUndoCommand(scene),
      m_allowMerging(allowMerging),
      m_timestamp(QDateTime::currentDateTime())
{
    Scene *s = this->scene();
    if (s != nullptr && s->isUndoRedoEnabled() && !s->inUndoCapture()) {
        m_before = this->toByteArray(s);
        this->setText(text);
        m_captured = true;
    }
}

SceneUndoCommand::~SceneUndoCommand() { }

void SceneUndoCommand::undo()
{
    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    if (this->fromByteArray(m_before) == nullptr)
        this->setObsolete(true);
    m_allowMerging = false;
}

void SceneUndoCommand::redo()
{
    Scene *s = this->scene();
    if (s == nullptr)
        return;

    if (!m_captured) {
        this->setObsolete(true);
        return;
    }

    if (m_after.isEmpty()) {
        // First redo after push: capture the after-state.
        m_after = this->toByteArray(s);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    if (this->fromByteArray(m_after) == nullptr)
        this->setObsolete(true);
    m_allowMerging = false;
}

bool SceneUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (!m_allowMerging || this->id() != other->id())
        return false;

    const SceneUndoCommand *cmd = static_cast<const SceneUndoCommand *>(other);
    if (!cmd->m_allowMerging || cmd->sceneId() != this->sceneId())
        return false;

    const qint64 timegap = qAbs(m_timestamp.msecsTo(cmd->m_timestamp));
    static const qint64 minTimegap = UndoHub::instance()->mergeTimeGap();
    if (timegap < minTimegap) {
        m_after = cmd->m_after;
        m_timestamp = cmd->m_timestamp;
        return true;
    }

    return false;
}

QByteArray SceneUndoCommand::toByteArray(Scene *scene) const
{
    return scene->toByteArray();
}

Scene *SceneUndoCommand::fromByteArray(const QByteArray &bytes) const
{
    return Scene::fromByteArray(bytes);
}

///////////////////////////////////////////////////////////////////////////////
// AbstractSceneUndoCommand
///////////////////////////////////////////////////////////////////////////////

AbstractSceneUndoCommand::AbstractSceneUndoCommand(Scene *scene) : m_scene(scene)
{
    if (m_scene != nullptr)
        m_sceneId = m_scene->id();
}

Scene *AbstractSceneUndoCommand::scene()
{
    if (m_scene != nullptr)
        return m_scene;

    if (m_sceneId.isEmpty()) {
        this->setObsolete(true);
        return nullptr;
    }

    const StructureElement *structureElement =
            ScriteDocument::instance()->structure()->findElementBySceneID(m_sceneId);
    if (structureElement == nullptr) {
        this->setObsolete(true);
        return nullptr;
    }

    m_scene = structureElement->scene();
    return m_scene;
}

void AbstractSceneUndoCommand::setSceneId(const QString &id)
{
    if (m_sceneId.isEmpty())
        m_sceneId = id;
}

///////////////////////////////////////////////////////////////////////////////
// SceneHeadingUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneHeadingUndoCommand::SceneHeadingUndoCommand(SceneHeading *sceneHeading)
    : AbstractSceneUndoCommand(sceneHeading ? sceneHeading->scene() : nullptr)
{
    if (sceneHeading) {
        m_before.save(sceneHeading);
        this->setText(QStringLiteral("Scene Heading"));
    }
}

void SceneHeadingUndoCommand::undo()
{
    Scene *scene = this->scene();
    if (scene == nullptr)
        return;

    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    m_before.load(scene->heading());
}

void SceneHeadingUndoCommand::redo()
{
    Scene *scene = this->scene();
    if (scene == nullptr)
        return;

    if (m_inited) {
        QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current,
                                                               this);
        m_after.load(scene->heading());
    } else {
        m_after.save(scene->heading());
        m_inited = true;
    }
}

bool SceneHeadingUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (this->id() == other->id()) {
        const SceneHeadingUndoCommand *other2 = static_cast<const SceneHeadingUndoCommand *>(other);
        m_after = other2->m_after;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// AbstractSceneElementUndoCommand
///////////////////////////////////////////////////////////////////////////////

AbstractSceneElementUndoCommand::AbstractSceneElementUndoCommand(SceneElement *sceneElement)
    : AbstractSceneUndoCommand(sceneElement ? sceneElement->scene() : nullptr),
      m_sceneElement(sceneElement)
{
}

SceneElement *AbstractSceneElementUndoCommand::lookupSceneElement()
{
    if (!m_sceneElement.isNull())
        return m_sceneElement.data();

    Scene *scene = this->scene();
    if (scene == nullptr)
        return nullptr;

    m_sceneElement = scene->findElementById(m_sceneElementId);
    if (m_sceneElement.isNull()) {
        this->setObsolete(true);
        return nullptr;
    }

    return m_sceneElement.data();
}

///////////////////////////////////////////////////////////////////////////////
// AbstractSceneElementLifetimeUndoCommand
///////////////////////////////////////////////////////////////////////////////

AbstractSceneElementLifetimeUndoCommand::AbstractSceneElementLifetimeUndoCommand(
        SceneElement *sceneElement)
    : AbstractSceneElementUndoCommand(sceneElement)
{
}

AbstractSceneElementLifetimeUndoCommand::AbstractSceneElementLifetimeUndoCommand(
        Scene *scene, SceneElement *sceneElement)
    : AbstractSceneElementUndoCommand(sceneElement)
{
    // The element has no scene parent yet; wire up the scene ID so that
    // AbstractSceneUndoCommand::scene() can resolve it via ID lookup later.
    if (scene != nullptr)
        this->setSceneId(scene->id());
}

SceneElement *AbstractSceneElementLifetimeUndoCommand::reconstructElement(const ElementData &data)
{
    SceneElement *element = new SceneElement(nullptr);
    element->setId(data.id);
    element->setType(data.type);
    element->setText(data.text);
    element->setAlignment(data.alignment);
    element->setTextFormats(data.textFormats);
    return element;
}

///////////////////////////////////////////////////////////////////////////////
// SceneElementTypeUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneElementTypeUndoCommand::SceneElementTypeUndoCommand(SceneElement *sceneElement)
    : AbstractSceneElementUndoCommand(sceneElement)
{
    if (sceneElement) {
        m_before = sceneElement->type();
        this->setText(QStringLiteral("Element Type"));
    }
}

void SceneElementTypeUndoCommand::undo()
{
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    element->setType(m_before);
}

void SceneElementTypeUndoCommand::redo()
{
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (m_inited) {
        QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current,
                                                               this);
        element->setType(m_after);
    } else {
        m_sceneElementId = element->id();
        m_after = element->type();
        m_inited = true;
    }
}

bool SceneElementTypeUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (this->id() == other->id()) {
        const SceneElementTypeUndoCommand *other2 =
                static_cast<const SceneElementTypeUndoCommand *>(other);
        if (other2->m_sceneElementId == m_sceneElementId) {
            m_after = other2->m_after;
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// SceneElementAlignmentUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneElementAlignmentUndoCommand::SceneElementAlignmentUndoCommand(SceneElement *sceneElement)
    : AbstractSceneElementUndoCommand(sceneElement)
{
    if (sceneElement) {
        m_before = sceneElement->alignment();
        this->setText(QStringLiteral("Element Alignment"));
    }
}

void SceneElementAlignmentUndoCommand::undo()
{
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    element->setAlignment(m_before);
}

void SceneElementAlignmentUndoCommand::redo()
{
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (m_inited) {
        QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current,
                                                               this);
        element->setAlignment(m_after);
    } else {
        m_sceneElementId = element->id();
        m_after = element->alignment();
        m_inited = true;
    }
}

bool SceneElementAlignmentUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (this->id() == other->id()) {
        const SceneElementAlignmentUndoCommand *other2 =
                static_cast<const SceneElementAlignmentUndoCommand *>(other);
        if (other2->m_sceneElementId == m_sceneElementId) {
            m_after = other2->m_after;
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// SceneElementTextFormatsUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneElementTextFormatsUndoCommand::SceneElementTextFormatsUndoCommand(SceneElement *sceneElement)
    : AbstractSceneElementUndoCommand(sceneElement)
{
    if (sceneElement) {
        m_before = sceneElement->textFormats();
        this->setText(QStringLiteral("Element Text Formats"));
    }
}

void SceneElementTextFormatsUndoCommand::undo()
{
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    element->setTextFormats(m_before);
}

void SceneElementTextFormatsUndoCommand::redo()
{
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (m_inited) {
        QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current,
                                                               this);
        element->setTextFormats(m_after);
    } else {
        m_sceneElementId = element->id();
        m_after = element->textFormats();
        m_inited = true;
    }
}

bool SceneElementTextFormatsUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (this->id() == other->id()) {
        const SceneElementTextFormatsUndoCommand *other2 =
                static_cast<const SceneElementTextFormatsUndoCommand *>(other);
        if (other2->m_sceneElementId == m_sceneElementId) {
            m_after = other2->m_after;
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// SceneInsertElementUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneInsertElementUndoCommand::SceneInsertElementUndoCommand(Scene *scene, SceneElement *element,
                                                             int index)
    : AbstractSceneElementLifetimeUndoCommand(scene, element), m_index(index)
{
    if (element) {
        m_elementData.capture(element);
        m_sceneElementId = m_elementData.id;
        this->setText(QStringLiteral("Insert Element"));
    }
}

void SceneInsertElementUndoCommand::undo()
{
    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    this->scene()->removeElement(element);
    m_sceneElement = nullptr; // element is now GC'd; future redo() recreates from m_elementData
}

void SceneInsertElementUndoCommand::redo()
{
    if (!m_inited) {
        // First redo: element was just inserted by insertElementAt(); just record it.
        m_inited = true;
        return;
    }

    // Subsequent redo: recreate the element from saved data and re-insert it.
    Scene *s = this->scene();
    if (s == nullptr) {
        this->setObsolete(true);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    SceneElement *newElement = reconstructElement(m_elementData);
    s->insertElementAt(newElement, m_index);
    m_sceneElement = newElement; // re-arm lookupSceneElement for a future undo()
}

///////////////////////////////////////////////////////////////////////////////
// SceneRemoveElementUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneRemoveElementUndoCommand::SceneRemoveElementUndoCommand(Scene *scene, SceneElement *element,
                                                             int index)
    : AbstractSceneElementLifetimeUndoCommand(element), m_index(index)
{
    Q_UNUSED(scene) // AbstractSceneElementLifetimeUndoCommand(element) already derives the scene
    if (element) {
        m_elementData.capture(element);
        m_sceneElementId = m_elementData.id;
        this->setText(QStringLiteral("Remove Element"));
    }
}

void SceneRemoveElementUndoCommand::undo()
{
    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    Scene *s = this->scene();
    if (s == nullptr) {
        this->setObsolete(true);
        return;
    }

    // Recreate the element from saved data and re-insert at the original index.
    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    SceneElement *newElement = reconstructElement(m_elementData);
    s->insertElementAt(newElement, m_index);
    m_sceneElement = newElement; // arm lookupSceneElement for subsequent redo()
}

void SceneRemoveElementUndoCommand::redo()
{
    if (!m_inited) {
        // First redo: element was just removed by removeElement(); just record it.
        m_inited = true;
        m_sceneElement = nullptr; // element is being GC'd by removeElement()
        return;
    }

    // Subsequent redo: the element was re-inserted by undo(); remove it again.
    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);
    this->scene()->removeElement(element);
    m_sceneElement = nullptr; // element is now GC'd; future undo() recreates from m_elementData
}

///////////////////////////////////////////////////////////////////////////////
// SceneElementTextUndoCommand
///////////////////////////////////////////////////////////////////////////////

SceneElementTextUndoCommand::SceneElementTextUndoCommand(SceneElement *sceneElement)
    : AbstractSceneElementUndoCommand(sceneElement)
{
    if (!enabled || sceneElement == nullptr || sceneElement->scene() == nullptr
        || !sceneElement->scene()->isUndoRedoEnabled() || sceneElement->scene()->inUndoCapture()) {
        this->setObsolete(true);
        return;
    }

    m_sceneElementId = sceneElement->id();
    m_oldCursorPosition = sceneElement->scene()->cursorPosition();
    m_oldText = sceneElement->text();
    m_timestamp = QDateTime::currentMSecsSinceEpoch();
    this->setText(QStringLiteral("Element Text"));
}

SceneElementTextUndoCommand::~SceneElementTextUndoCommand() { }

void SceneElementTextUndoCommand::undo()
{
    if (!m_inited) {
        this->setObsolete(true);
        return;
    }

    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);

    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    element->setText(m_oldText);
    element->scene()->sceneReset(m_oldCursorPosition);
}

void SceneElementTextUndoCommand::redo()
{
    QScopedValueRollback<AbstractSceneUndoCommand *> __crb(AbstractSceneUndoCommand::current, this);

    SceneElement *element = this->lookupSceneElement();
    if (element == nullptr)
        return;

    if (!m_inited) {
        // First redo: capture after-state (IDs already set in constructor).
        m_newText = element->text();
        // scene->cursorPosition() is still the pre-edit value here: the
        // qScopeGuard in SceneDocumentBinder::onContentsChange() that calls
        // scene->setCursorPosition(from + charsAdded) fires only when
        // onContentsChange() returns, which is after this redo() call.
        // Derive the correct post-edit cursor position from the text-length delta
        // instead; this is accurate for all common editing operations (typing,
        // backspace, delete, forward-selection replace).
        m_newCursorPosition = m_oldCursorPosition + (m_newText.length() - m_oldText.length());
        this->setText("[" + this->sceneId() + "]: " + m_oldText + " -> " + m_newText);
        m_inited = true;
    } else {
        Scene *scene = element->scene();
        element->setText(m_newText);
        emit scene->sceneReset(m_newCursorPosition);
    }
}

bool SceneElementTextUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (ID != other->id())
        return false;

    const SceneElementTextUndoCommand *cmd =
            static_cast<const SceneElementTextUndoCommand *>(other);

    if (qAbs(cmd->m_newCursorPosition - m_newCursorPosition) >= 2)
        return false;

    if (qAbs(cmd->m_timestamp - m_timestamp) > 5000)
        return false;

    if (LanguageEngine::fastSentenceCount(m_newText)
        == LanguageEngine::fastSentenceCount(cmd->m_newText)) {
        m_newText = cmd->m_newText;
        m_newCursorPosition = cmd->m_newCursorPosition;
        this->setText("[" + this->sceneId() + "]: " + m_oldText + " -> " + m_newText);
        return true;
    }

    return false;
}
