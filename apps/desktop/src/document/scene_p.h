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

#ifndef SCENE_P_H
#define SCENE_P_H

#include "scene.h"
#include "undoredo.h"

#include <QDateTime>
#include <QByteArray>
#include <QTextLayout>

///////////////////////////////////////////////////////////////////////////////
// AbstractSceneUndoCommand — base for all surgical undo commands
///////////////////////////////////////////////////////////////////////////////

class AbstractSceneUndoCommand : public QUndoCommand
{
public:
    virtual ~AbstractSceneUndoCommand() { }

    static bool hasCurrent() { return current != nullptr; }

protected:
    static AbstractSceneUndoCommand *current;

    explicit AbstractSceneUndoCommand(Scene *scene);

    Scene *scene();

    void setSceneId(const QString &id);
    QString sceneId() const { return m_sceneId; }

private:
    friend class PushSceneUndoCommand;

    QString m_sceneId;
    QPointer<Scene> m_scene;
};

///////////////////////////////////////////////////////////////////////////////
// PushSceneUndoCommand — RAII pusher for AbstractSceneUndoCommand subclasses
///////////////////////////////////////////////////////////////////////////////

class PushSceneUndoCommand
{
public:
    PushSceneUndoCommand(AbstractSceneUndoCommand *cmd)
    {
        m_enabled = !AbstractSceneUndoCommand::hasCurrent();
        m_command = cmd;
    }
    ~PushSceneUndoCommand()
    {
        if (m_command != nullptr && m_enabled && UndoHub::active() != nullptr
            && !m_command->isObsolete())
            UndoHub::active()->push(m_command);
        else
            delete m_command;
    }

private:
    bool m_enabled = false;
    AbstractSceneUndoCommand *m_command = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// SceneUndoCommand — whole-scene undo via serialization
///////////////////////////////////////////////////////////////////////////////

class SceneUndoCommand : public AbstractSceneUndoCommand
{
public:
    explicit SceneUndoCommand(Scene *scene, bool allowMerging = true,
                              const QString &text = QStringLiteral("Scene Capture"));
    ~SceneUndoCommand();

    enum { ID = UndoStack::SceneCommandID };
    int id() const { return ID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

private:
    QByteArray toByteArray(Scene *scene) const;
    Scene *fromByteArray(const QByteArray &bytes) const;

private:
    bool m_allowMerging = true;
    bool m_captured = false;
    QByteArray m_after;
    QByteArray m_before;
    QDateTime m_timestamp;
};

///////////////////////////////////////////////////////////////////////////////
// SceneHeadingUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneHeadingUndoCommand : public AbstractSceneUndoCommand
{
public:
    explicit SceneHeadingUndoCommand(SceneHeading *sceneHeading);
    ~SceneHeadingUndoCommand() { }

    int id() const { return UndoStack::SceneHeadingCommandID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

private:
    struct
    {
        QString moment;
        QString location;
        QString locationType;
        bool enabled = true;

        void save(const SceneHeading *heading)
        {
            if (heading) {
                enabled = heading->isEnabled();
                location = heading->location();
                locationType = heading->locationType();
                moment = heading->moment();
            }
        }

        void load(SceneHeading *heading)
        {
            if (heading) {
                heading->setEnabled(enabled);
                heading->setMoment(moment);
                heading->setLocation(location);
                heading->setLocationType(locationType);
            }
        }
    } m_before, m_after;
    bool m_inited = false;
};

///////////////////////////////////////////////////////////////////////////////
// AbstractSceneElementUndoCommand — base for element-level property commands
///////////////////////////////////////////////////////////////////////////////

class AbstractSceneElementUndoCommand : public AbstractSceneUndoCommand
{
protected:
    explicit AbstractSceneElementUndoCommand(SceneElement *sceneElement);

    SceneElement *lookupSceneElement();

    QString m_sceneElementId;
    QPointer<SceneElement> m_sceneElement;
    bool m_inited = false;
};

///////////////////////////////////////////////////////////////////////////////
// AbstractSceneElementLifetimeUndoCommand — base for insert/remove commands
///////////////////////////////////////////////////////////////////////////////

class AbstractSceneElementLifetimeUndoCommand : public AbstractSceneElementUndoCommand
{
protected:
    // Use when the element already belongs to a scene (e.g. remove).
    explicit AbstractSceneElementLifetimeUndoCommand(SceneElement *sceneElement);
    // Use when the element does not yet belong to a scene (e.g. before insertElementAt).
    explicit AbstractSceneElementLifetimeUndoCommand(Scene *scene, SceneElement *sceneElement);

    // Captures all user-visible element properties so the element can be fully reconstructed.
    struct ElementData
    {
        QString id;
        SceneElement::Type type = SceneElement::Action;
        QString text;
        Qt::Alignment alignment = Qt::Alignment(0);
        QVector<QTextLayout::FormatRange> textFormats;

        void capture(const SceneElement *element)
        {
            if (element) {
                id = element->id();
                type = element->type();
                text = element->text();
                alignment = element->alignment();
                textFormats = element->textFormats();
            }
        }
    };

    // Creates a fresh SceneElement from saved data. Must be called with
    // AbstractSceneUndoCommand::current set (rollback guard active) so that the
    // property setters do not push nested undo commands.
    static SceneElement *reconstructElement(const ElementData &data);
};

///////////////////////////////////////////////////////////////////////////////
// SceneElementTypeUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneElementTypeUndoCommand : public AbstractSceneElementUndoCommand
{
public:
    explicit SceneElementTypeUndoCommand(SceneElement *sceneElement);
    ~SceneElementTypeUndoCommand() { }

    int id() const { return UndoStack::SceneElementTypeCommandID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

private:
    SceneElement::Type m_before = SceneElement::Action;
    SceneElement::Type m_after = SceneElement::Action;
};

///////////////////////////////////////////////////////////////////////////////
// SceneElementAlignmentUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneElementAlignmentUndoCommand : public AbstractSceneElementUndoCommand
{
public:
    explicit SceneElementAlignmentUndoCommand(SceneElement *sceneElement);
    ~SceneElementAlignmentUndoCommand() { }

    int id() const { return UndoStack::SceneElementAlignmentCommandID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

private:
    Qt::Alignment m_before = Qt::Alignment(0);
    Qt::Alignment m_after = Qt::Alignment(0);
};

///////////////////////////////////////////////////////////////////////////////
// SceneElementTextFormatsUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneElementTextFormatsUndoCommand : public AbstractSceneElementUndoCommand
{
public:
    explicit SceneElementTextFormatsUndoCommand(SceneElement *sceneElement);
    ~SceneElementTextFormatsUndoCommand() { }

    int id() const { return UndoStack::SceneElementTextFormatsCommandID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

private:
    QVector<QTextLayout::FormatRange> m_before;
    QVector<QTextLayout::FormatRange> m_after;
};

///////////////////////////////////////////////////////////////////////////////
// SceneInsertElementUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneInsertElementUndoCommand : public AbstractSceneElementLifetimeUndoCommand
{
public:
    explicit SceneInsertElementUndoCommand(Scene *scene, SceneElement *element, int index);
    ~SceneInsertElementUndoCommand() { }

    int id() const { return UndoStack::SceneInsertElementCommandID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *) { return false; }

private:
    int m_index = -1;
    ElementData m_elementData;
};

///////////////////////////////////////////////////////////////////////////////
// SceneRemoveElementUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneRemoveElementUndoCommand : public AbstractSceneElementLifetimeUndoCommand
{
public:
    explicit SceneRemoveElementUndoCommand(Scene *scene, SceneElement *element, int index);
    ~SceneRemoveElementUndoCommand() { }

    int id() const { return UndoStack::SceneRemoveElementCommandID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *) { return false; }

private:
    int m_index = -1;
    ElementData m_elementData;
};

///////////////////////////////////////////////////////////////////////////////
// SceneElementTextUndoCommand
///////////////////////////////////////////////////////////////////////////////

class SceneElementTextUndoCommand : public AbstractSceneElementUndoCommand
{
    friend class SceneElement;
    static bool enabled;

public:
    SceneElementTextUndoCommand(SceneElement *sceneElement);
    ~SceneElementTextUndoCommand();

    enum { ID = UndoStack::SceneElementTextCommandID };
    int id() const { return ID; }
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

private:
    int m_oldCursorPosition = -1, m_newCursorPosition = -1;
    QString m_oldText, m_newText;
    qint64 m_timestamp = 0;
};

#endif // SCENE_P_H
