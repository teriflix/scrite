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

#include "actioncontexts.h"

ScreenplayElementsContext::ScreenplayElementsContext(QObject *parent)
    : AbstractActionContext(QStringLiteral("ScreenplayElements"), parent)
{
}

ScreenplayElementsContext::~ScreenplayElementsContext() { }

void ScreenplayElementsContext::setElement(ScreenplayElement *val)
{
    if (m_element == val)
        return;

    if (m_element != nullptr)
        disconnect(m_element, &QObject::destroyed, this,
                   &ScreenplayElementsContext::onElementDestroyed);

    m_element = val;

    if (m_element != nullptr)
        connect(m_element, &QObject::destroyed, this,
                &ScreenplayElementsContext::onElementDestroyed);

    emit elementChanged();
}

void ScreenplayElementsContext::setSceneGroup(SceneGroup *val)
{
    if (m_sceneGroup == val)
        return;

    if (m_sceneGroup != nullptr)
        disconnect(m_sceneGroup, &QObject::destroyed, this,
                   &ScreenplayElementsContext::onSceneGroupDestroyed);

    m_sceneGroup = val;

    if (m_sceneGroup != nullptr)
        connect(m_sceneGroup, &QObject::destroyed, this,
                &ScreenplayElementsContext::onSceneGroupDestroyed);

    emit sceneGroupChanged();
}

void ScreenplayElementsContext::onElementDestroyed()
{
    m_element = nullptr;
    emit elementChanged();
}

void ScreenplayElementsContext::onSceneGroupDestroyed()
{
    m_sceneGroup = nullptr;
    emit sceneGroupChanged();
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayBreakContext::ScreenplayBreakContext(QObject *parent)
    : AbstractActionContext(QStringLiteral("ScreenplayBreak"), parent)
{
}

ScreenplayBreakContext::~ScreenplayBreakContext() { }

void ScreenplayBreakContext::setElement(ScreenplayElement *val)
{
    if (m_element == val)
        return;

    if (m_element != nullptr)
        disconnect(m_element, &QObject::destroyed, this,
                   &ScreenplayBreakContext::onElementDestroyed);

    m_element = val;

    if (m_element != nullptr)
        connect(m_element, &QObject::destroyed, this, &ScreenplayBreakContext::onElementDestroyed);

    emit elementChanged();
}

void ScreenplayBreakContext::onElementDestroyed()
{
    m_element = nullptr;
    emit elementChanged();
}

///////////////////////////////////////////////////////////////////////////////

SceneContext::SceneContext(QObject *parent)
    : AbstractActionContext(QStringLiteral("Scene"), parent)
{
}

SceneContext::~SceneContext() { }

void SceneContext::setScene(Scene *val)
{
    if (m_scene == val)
        return;

    if (m_scene != nullptr)
        disconnect(m_scene, &QObject::destroyed, this, &SceneContext::onSceneDestroyed);

    m_scene = val;

    if (m_scene != nullptr)
        connect(m_scene, &QObject::destroyed, this, &SceneContext::onSceneDestroyed);

    emit sceneChanged();
}

void SceneContext::onSceneDestroyed()
{
    m_scene = nullptr;
    emit sceneChanged();
}

///////////////////////////////////////////////////////////////////////////////

NotesContext::NotesContext(QObject *parent)
    : AbstractActionContext(QStringLiteral("Notes"), parent)
{
}

NotesContext::~NotesContext() { }

void NotesContext::setNotes(Notes *val)
{
    if (m_notes == val)
        return;

    if (m_notes != nullptr)
        disconnect(m_notes, &QObject::destroyed, this, &NotesContext::onNotesDestroyed);

    m_notes = val;

    if (m_notes != nullptr)
        connect(m_notes, &QObject::destroyed, this, &NotesContext::onNotesDestroyed);

    emit notesChanged();
}

void NotesContext::onNotesDestroyed()
{
    m_notes = nullptr;
    emit notesChanged();
}

///////////////////////////////////////////////////////////////////////////////

NoteContext::NoteContext(QObject *parent)
    : AbstractActionContext(QStringLiteral("Note"), parent)
{
}

NoteContext::~NoteContext() { }

void NoteContext::setNote(Note *val)
{
    if (m_note == val)
        return;

    if (m_note != nullptr)
        disconnect(m_note, &QObject::destroyed, this, &NoteContext::onNoteDestroyed);

    m_note = val;

    if (m_note != nullptr)
        connect(m_note, &QObject::destroyed, this, &NoteContext::onNoteDestroyed);

    emit noteChanged();
}

void NoteContext::onNoteDestroyed()
{
    m_note = nullptr;
    emit noteChanged();
}
