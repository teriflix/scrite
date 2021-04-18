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

#include "storythreads.h"

StoryThread::StoryThread(QObject *parent)
    : ObjectListPropertyModel<Scene*>(parent)
{
    m_allThreads = qobject_cast<StoryThreads*>(parent);

    connect(this, &QAbstractListModel::rowsInserted,
            this, &StoryThread::setScreenplayNeedsSync);
    connect(this, &QAbstractListModel::rowsRemoved,
            this, &StoryThread::setScreenplayNeedsSync);
    connect(this, &QAbstractListModel::modelReset,
            this, &StoryThread::setScreenplayNeedsSync);
    connect(this, &QAbstractListModel::rowsMoved,
            this, &StoryThread::setScreenplayNeedsSync);
    connect(this, &QAbstractListModel::dataChanged,
            this, &StoryThread::setScreenplayNeedsSync);
}

StoryThread::~StoryThread()
{

}

void StoryThread::setName(const QString &val)
{
    if(m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void StoryThread::addScene(Scene *scene)
{
    this->SuperClass::append(scene);
}

void StoryThread::removeScene(Scene *scene)
{
    this->SuperClass::removeAt( this->SuperClass::indexOf(scene) );
}

int StoryThread::indexOfScene(Scene *scene) const
{
    return this->SuperClass::indexOf(scene);
}

void StoryThread::moveScene(int fromRow, int toRow)
{
    this->SuperClass::move(fromRow, toRow);
}

void StoryThread::moveSceneUp(Scene *scene)
{
    int index = this->SuperClass::indexOf(scene);
    if(index <= 0)
        return;

    this->SuperClass::move(index, index-1);
}

void StoryThread::moveSceneDown(Scene *scene)
{
    int index = this->SuperClass::indexOf(scene);
    if(index < 0 || index >= this->objectCount()-1)
        return;

    this->SuperClass::move(index, index+1);
}

bool StoryThread::canMoveSceneUp(Scene *scene) const
{
    return this->SuperClass::indexOf(scene) <= 0;
}

bool StoryThread::canMoveSceneDown(Scene *scene) const
{
    return [=](int index) -> bool {
        return index < 0 || index >= this->objectCount()-1;
    }(this->SuperClass::indexOf(scene));
}

void StoryThread::syncScreenplay()
{
    if(!m_screenplayNeedsSync)
        return;

    m_screenplay->clearElements();

    QList<Scene*> &scenes = this->SuperClass::list();
    QList<QObject*> elements;
    for(Scene *scene : qAsConst(scenes))
    {
        ScreenplayElement *element = new ScreenplayElement(m_screenplay);
        element->setScene(scene);
        elements.append(element);
    }

    m_screenplay->setPropertyFromObjectList(QStringLiteral("elements"), elements);

    m_screenplayNeedsSync = false;
    emit screenplayNeedsSyncChanged();
}

void StoryThread::setScreenplayNeedsSync()
{
    if(m_screenplayNeedsSync)
        return;

    m_screenplayNeedsSync = true;
    emit screenplayNeedsSyncChanged();
}

////////////////////////////////////////////////////////////////////////////////

StoryThreads::StoryThreads(QObject *parent)
    : ObjectListPropertyModel<StoryThread*>(parent)
{
    m_document = qobject_cast<ScriteDocument*>(parent);
    if(m_document == nullptr)
        m_document = ScriteDocument::instance();
}

StoryThreads::~StoryThreads()
{

}

StoryThread *StoryThreads::newThreadBySceneColor(const QColor &color)
{
    const QList<Scene*> scenes = this->findScenes([color](Scene *scene) -> bool {
        return scene->color() == color;
    });

    StoryThread *thread = new StoryThread(this);
    thread->assign(scenes);
    thread->setName( QStringLiteral("Scenes of color %1").arg(color.name()) );
    this->append(thread);
    return thread;
}

StoryThread *StoryThreads::newThreadByCharacter(const QString &character)
{
    const QList<Scene*> scenes = this->findScenes([character](Scene *scene) -> bool {
        return scene->hasCharacters() && scene->hasCharacter(character);
    });

    StoryThread *thread = new StoryThread(this);
    thread->assign(scenes);
    thread->setName( QStringLiteral("Scenes of %1").arg(character) );
    this->append(thread);
    return thread;
}

StoryThread *StoryThreads::newThreadByLocation(const QString &location, bool exact)
{
    const QList<Scene*> scenes = this->findScenes([location,exact](Scene *scene) -> bool {
        if(scene->heading()->isEnabled()) {
            if(exact)
                return scene->heading()->location() == location;
            return scene->heading()->location().contains(location);
        }
        return false;
    });

    StoryThread *thread = new StoryThread(this);
    thread->assign(scenes);
    if(exact)
        thread->setName( QStringLiteral("Scenes at %1").arg(location) );
    else
        thread->setName( QStringLiteral("Scenes near %1").arg(location) );
    this->append(thread);
    return thread;
}

StoryThread *StoryThreads::newThreadByMoment(const QString &moment)
{
    const QList<Scene*> scenes = this->findScenes([moment](Scene *scene) -> bool {
        return scene->heading()->isEnabled() && scene->heading()->moment() == moment;
    });

    StoryThread *thread = new StoryThread(this);
    thread->assign(scenes);
    thread->setName( QStringLiteral("Scenes at %1").arg(moment) );
    this->append(thread);
    return thread;
}

StoryThread *StoryThreads::newThread(const QString &name)
{
    StoryThread *thread = new StoryThread(this);
    if(name.isEmpty())
        thread->setName( QStringLiteral("Unnamed Thread") );
    else
        thread->setName(name);
    this->append(thread);
    return thread;
}

QList<Scene *> StoryThreads::findScenes(std::function<bool (Scene *)> filterFunc)
{
    Structure *structure = m_document->structure();
    Screenplay *screenplay = m_document->screenplay();
    int spOffset = screenplay->elementCount();

    QVector< QPair<Scene*,int> > scenes;
    scenes.reserve(structure->elementCount());

    for(int i=0; i<structure->elementCount(); i++)
    {
        StructureElement *element = structure->elementAt(i);
        Scene *scene = element->scene();
        if( filterFunc(scene) )
        {
            int index = screenplay->firstIndexOfScene(scene);
            if(index < 0)
                index = spOffset + i;
            scenes.append(qMakePair(scene,index));
        }
    }

    std::sort(scenes.begin(), scenes.end(),
              [](const QPair<Scene*,int> &p1, const QPair<Scene*,int> &p2) {
        return p1.second < p2.second;
    });

    QList<Scene*> ret;
    ret.reserve(scenes.size());
    for(auto p : qAsConst(scenes))
        ret.append(p.first);

    return ret;
}
