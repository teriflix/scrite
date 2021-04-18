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

#ifndef STORYTHREADS_H
#define STORYTHREADS_H

#include "scene.h"
#include "structure.h"
#include "screenplay.h"
#include "scritedocument.h"
#include "objectlistpropertymodel.h"

class StoryThreads;

class StoryThread : public ObjectListPropertyModel<Scene*>
{
    Q_OBJECT

public:
    typedef ObjectListPropertyModel<Scene*> SuperClass;

    StoryThread(QObject *parent=nullptr);
    ~StoryThread();

    Q_PROPERTY(StoryThreads* allThreads READ allThreads CONSTANT)
    StoryThreads *allThreads() const { return m_allThreads; }

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay CONSTANT)
    Screenplay *screenplay() const { return m_screenplay; }

    Q_PROPERTY(bool screenplayNeedsSync READ screenplayNeedsSync NOTIFY screenplayNeedsSyncChanged)
    bool screenplayNeedsSync() const { return m_screenplayNeedsSync; }
    Q_SIGNAL void screenplayNeedsSyncChanged();

    // shadow methods calling base class ones, because the
    // base template class cannot have meta-object
    Q_INVOKABLE void addScene(Scene *scene);
    Q_INVOKABLE void removeScene(Scene *scene);
    Q_INVOKABLE int  indexOfScene(Scene *scene) const;
    Q_INVOKABLE void moveScene(int fromRow, int toRow);
    Q_INVOKABLE void moveSceneUp(Scene *scene);
    Q_INVOKABLE void moveSceneDown(Scene *scene);
    Q_INVOKABLE bool canMoveSceneUp(Scene *scene) const;
    Q_INVOKABLE bool canMoveSceneDown(Scene *scene) const;
    Q_INVOKABLE void syncScreenplay();

private:
    void setScreenplayNeedsSync();

private:
    QString m_name;
    Screenplay *m_screenplay = new Screenplay(this);
    StoryThreads *m_allThreads = nullptr;
    bool m_screenplayNeedsSync = false;
};

class StoryThreads : public ObjectListPropertyModel<StoryThread*>
{
    Q_OBJECT

public:
    typedef ObjectListPropertyModel<StoryThread*> SuperClass;

    StoryThreads(QObject *parent=nullptr);
    ~StoryThreads();

    Q_INVOKABLE StoryThread *newThreadBySceneColor(const QColor &color);
    Q_INVOKABLE StoryThread *newThreadByCharacter(const QString &character);
    Q_INVOKABLE StoryThread *newThreadByLocation(const QString &location, bool exact=false);
    Q_INVOKABLE StoryThread *newThreadByMoment(const QString &moment);
    Q_INVOKABLE StoryThread *newThread(const QString &name);

private:
    QList<Scene*> findScenes(std::function<bool(Scene*)> filterFunc);

private:
    ScriteDocument *m_document = nullptr;
};

#endif // STORYTHREADS_H
