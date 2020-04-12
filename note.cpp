/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "note.h"
#include "structure.h"
#include "scene.h"

Note::Note(QObject *parent)
     : QObject(parent),
       m_structure(qobject_cast<Structure*>(parent)),
       m_character(qobject_cast<Character*>(parent))
{
    connect(this, &Note::headingChanged, this, &Note::noteChanged);
    connect(this, &Note::contentChanged, this, &Note::noteChanged);
}

Note::~Note()
{

}

void Note::setHeading(const QString &val)
{
    if(m_heading == val)
        return;

    m_heading = val;
    emit headingChanged();
}

void Note::setContent(const QString &val)
{
    if(m_content == val)
        return;

    m_content = val;
    emit contentChanged();
}

void Note::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

bool Note::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        m_structure = qobject_cast<Structure*>(this->parent());
        m_character = qobject_cast<Character*>(this->parent());
    }

    return QObject::event(event);
}

