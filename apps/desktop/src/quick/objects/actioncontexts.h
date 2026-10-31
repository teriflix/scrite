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

#ifndef ACTIONCONTEXTS_H
#define ACTIONCONTEXTS_H

#include <QPointer>
#include <QQmlEngine>

#include "scene.h"
#include "notes.h"
#include "structure.h"
#include "screenplay.h"
#include "actionhost.h"

/**
 * The contexts that hosts of menus and toolbars give to the actions that are triggered from them.
 * One class for each kind of place, whose properties follow the host, and so the delegates that
 * are made for a host. Properties clear themselves when their objects are destroyed.
 *
 * Contexts for new places are added here.
 */

// Context menu of scenes in the screenplay editor.
class ScreenplayElementsContext : public AbstractActionContext
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayElementsContext(QObject *parent = nullptr);
    ~ScreenplayElementsContext() override;

    // clang-format off
    Q_PROPERTY(ScreenplayElement* element
               READ element
               WRITE setElement
               NOTIFY elementChanged)
    // clang-format on
    void setElement(ScreenplayElement *val);
    ScreenplayElement *element() const { return m_element; }
    Q_SIGNAL void elementChanged();

    // clang-format off
    Q_PROPERTY(SceneGroup* sceneGroup
               READ sceneGroup
               WRITE setSceneGroup
               NOTIFY sceneGroupChanged)
    // clang-format on
    void setSceneGroup(SceneGroup *val);
    SceneGroup *sceneGroup() const { return m_sceneGroup; }
    Q_SIGNAL void sceneGroupChanged();

private:
    void onElementDestroyed();
    void onSceneGroupDestroyed();

private:
    QPointer<SceneGroup> m_sceneGroup;
    QPointer<ScreenplayElement> m_element;
};

// Context menu of acts, episodes and intervals in the screenplay editor.
class ScreenplayBreakContext : public AbstractActionContext
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayBreakContext(QObject *parent = nullptr);
    ~ScreenplayBreakContext() override;

    // clang-format off
    Q_PROPERTY(ScreenplayElement* element
               READ element
               WRITE setElement
               NOTIFY elementChanged)
    // clang-format on
    void setElement(ScreenplayElement *val);
    ScreenplayElement *element() const { return m_element; }
    Q_SIGNAL void elementChanged();

private:
    void onElementDestroyed();

private:
    QPointer<ScreenplayElement> m_element;
};

// The menu of a scene in the screenplay editor.
class SceneContext : public AbstractActionContext
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SceneContext(QObject *parent = nullptr);
    ~SceneContext() override;

    // clang-format off
    Q_PROPERTY(Scene* scene
               READ scene
               WRITE setScene
               NOTIFY sceneChanged)
    // clang-format on
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

private:
    void onSceneDestroyed();

private:
    QPointer<Scene> m_scene;
};

// The New Note menu in the Notebook.
class NotesContext : public AbstractActionContext
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotesContext(QObject *parent = nullptr);
    ~NotesContext() override;

    // clang-format off
    Q_PROPERTY(Notes* notes
               READ notes
               WRITE setNotes
               NOTIFY notesChanged)
    // clang-format on
    void setNotes(Notes *val);
    Notes *notes() const { return m_notes; }
    Q_SIGNAL void notesChanged();

private:
    void onNotesDestroyed();

private:
    QPointer<Notes> m_notes;
};

// The toolbar of Markdown notes in the Notebook.
class NoteContext : public AbstractActionContext
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NoteContext(QObject *parent = nullptr);
    ~NoteContext() override;

    // clang-format off
    Q_PROPERTY(Note* note
               READ note
               WRITE setNote
               NOTIFY noteChanged)
    // clang-format on
    void setNote(Note *val);
    Note *note() const { return m_note; }
    Q_SIGNAL void noteChanged();

private:
    void onNoteDestroyed();

private:
    QPointer<Note> m_note;
};

#endif // ACTIONCONTEXTS_H
