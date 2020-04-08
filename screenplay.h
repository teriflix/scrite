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

#ifndef SCREENPLAY_H
#define SCREENPLAY_H

#include "scene.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QQmlListProperty>

class Screenplay;
class ScriteDocument;

class ScreenplayElement : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE ScreenplayElement(QObject *parent=nullptr);
    ~ScreenplayElement();
    Q_SIGNAL void aboutToDelete(ScreenplayElement *element);

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged STORED false)
    void setScreenplay(Screenplay *val);
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(QString sceneID READ sceneID WRITE setSceneFromID NOTIFY sceneChanged)
    void setSceneFromID(const QString &val);
    QString sceneID() const;

    Q_PROPERTY(Scene* scene READ scene NOTIFY sceneChanged STORED false)
    void setScene(Scene *val);
    Scene* scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)
    void setExpanded(bool val);
    bool isExpanded() const { return m_expanded; }
    Q_SIGNAL void expandedChanged();

    Q_PROPERTY(QJsonValue userData READ userData WRITE setUserData NOTIFY userDataChanged STORED false)
    void setUserData(const QJsonValue &val);
    QJsonValue userData() const { return m_userData; }
    Q_SIGNAL void userDataChanged();

    Q_SIGNAL void elementChanged();

protected:
    bool event(QEvent *event);

private:
    Scene* m_scene;
    bool m_expanded;
    QString m_sceneID;
    QJsonValue m_userData;
    Screenplay* m_screenplay;
};

class Screenplay : public QAbstractListModel
{
    Q_OBJECT

public:
    Screenplay(QObject *parent=nullptr);
    ~Screenplay();

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument* scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
    void setAuthor(const QString &val);
    QString author() const { return m_author; }
    Q_SIGNAL void authorChanged();

    Q_PROPERTY(QString contact READ contact WRITE setContact NOTIFY contactChanged)
    void setContact(const QString &val);
    QString contact() const { return m_contact; }
    Q_SIGNAL void contactChanged();

    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    void setVersion(const QString &val);
    QString version() const { return m_version; }
    Q_SIGNAL void versionChanged();

    Q_PROPERTY(QQmlListProperty<ScreenplayElement> elements READ elements NOTIFY elementsChanged)
    QQmlListProperty<ScreenplayElement> elements();
    Q_INVOKABLE void addElement(ScreenplayElement *ptr);
    Q_INVOKABLE void insertAt(ScreenplayElement *ptr, int index);
    Q_INVOKABLE void removeElement(ScreenplayElement *ptr);
    Q_INVOKABLE void moveElement(ScreenplayElement *ptr, int toRow);
    Q_INVOKABLE ScreenplayElement *elementAt(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();

    Q_INVOKABLE int indexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(ScreenplayElement *element) const;

    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(int currentElementIndex READ currentElementIndex WRITE setCurrentElementIndex NOTIFY currentElementIndexChanged)
    void setCurrentElementIndex(int val);
    int currentElementIndex() const { return m_currentElementIndex; }
    Q_SIGNAL void currentElementIndexChanged();

    Q_PROPERTY(Scene* activeScene READ activeScene WRITE setActiveScene NOTIFY activeSceneChanged STORED false)
    void setActiveScene(Scene* val);
    Scene* activeScene() const { return m_activeScene; }
    Q_SIGNAL void activeSceneChanged();

    Q_INVOKABLE QJsonArray search(const QString &text, int flags=0) const;

    // QAbstractItemModel interface
    enum Roles { ScreenplayElementRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

protected:
    bool event(QEvent *event);

private:
    QString m_title;
    QString m_author;
    QString m_contact;
    QString m_version;
    ScriteDocument *m_scriteDocument;

    static void staticAppendElement(QQmlListProperty<ScreenplayElement> *list, ScreenplayElement *ptr);
    static void staticClearElements(QQmlListProperty<ScreenplayElement> *list);
    static ScreenplayElement* staticElementAt(QQmlListProperty<ScreenplayElement> *list, int index);
    static int staticElementCount(QQmlListProperty<ScreenplayElement> *list);
    QList<ScreenplayElement *> m_elements;
    int m_currentElementIndex;
    Scene* m_activeScene;
};

#endif // SCREENPLAY_H
