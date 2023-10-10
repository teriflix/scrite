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

#ifndef SCREENPLAYADAPTER_H
#define SCREENPLAYADAPTER_H

#include <QTimer>
#include <QPointer>
#include <QQmlEngine>
#include <QIdentityProxyModel>

#include "qobjectproperty.h"

class Scene;
class Screenplay;
class SceneElement;
class ScreenplayElement;

#define MAX_ELEMENT_COUNT 16777216 // 2^24

class ScreenplayAdapter : public QIdentityProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayAdapter(QObject *parent = nullptr);
    ~ScreenplayAdapter();

    Q_PROPERTY(QObject *source READ source WRITE setSource NOTIFY sourceChanged RESET resetSource)
    void setSource(QObject *val);
    QObject *source() const { return m_source; }
    Q_SIGNAL void sourceChanged();

    Q_PROPERTY(bool isSourceScene READ isSourceScene NOTIFY sourceChanged)
    bool isSourceScene() const;

    Q_PROPERTY(bool isSourceScreenplay READ isSourceScreenplay NOTIFY sourceChanged)
    bool isSourceScreenplay() const;

    Q_PROPERTY(Screenplay *screenplay READ screenplay NOTIFY sourceChanged)
    Screenplay *screenplay() const;

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    void setCurrentIndex(int val);
    int currentIndex() const { return m_currentIndex; }
    Q_SIGNAL void currentIndexChanged(int val);

    Q_PROPERTY(ScreenplayElement *currentElement READ currentElement NOTIFY currentElementChanged)
    ScreenplayElement *currentElement() const { return m_currentElement; }
    Q_SIGNAL void currentElementChanged();

    Q_PROPERTY(Scene *currentScene READ currentScene NOTIFY currentElementChanged)
    Scene *currentScene() const;

    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_SIGNAL void elementCountChanged();

    Q_PROPERTY(
            bool hasNonStandardScenes READ hasNonStandardScenes NOTIFY hasNonStandardScenesChanged)
    bool hasNonStandardScenes() const;
    Q_SIGNAL void hasNonStandardScenesChanged();

    Q_PROPERTY(int wordCount READ wordCount NOTIFY wordCountChanged)
    int wordCount() const;
    Q_SIGNAL void wordCountChanged();

    Q_PROPERTY(bool heightHintsAvailable READ isHeightHintsAvailable NOTIFY heightHintsAvailableChanged)
    bool isHeightHintsAvailable() const;
    Q_SIGNAL void heightHintsAvailableChanged();

    Q_PROPERTY(int initialLoadTreshold READ initialLoadTreshold WRITE setInitialLoadTreshold NOTIFY
                       initialLoadTresholdChanged)
    void setInitialLoadTreshold(int val);
    int initialLoadTreshold() const { return m_initialLoadTreshold; }
    Q_SIGNAL void initialLoadTresholdChanged();

    Q_INVOKABLE ScreenplayElement *splitElement(ScreenplayElement *ptr, SceneElement *element,
                                                int textPosition);
    Q_INVOKABLE ScreenplayElement *mergeElementWithPrevious(ScreenplayElement *ptr);
    Q_INVOKABLE int previousSceneElementIndex();
    Q_INVOKABLE int nextSceneElementIndex();
    Q_INVOKABLE QVariant at(int row) const;
    Q_INVOKABLE void refresh();

    enum Roles {
        IdRole = Qt::UserRole,
        ScreenplayElementRole,
        ScreenplayElementTypeRole,
        BreakTypeRole,
        SceneRole,
        ModelDataRole
    };
    Q_ENUM(Roles)
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    void fetchMore(const QModelIndex &parent);
    bool canFetchMore(const QModelIndex &parent) const;

private:
    void setCurrentIndexInternal(int val);
    void setCurrentElement(ScreenplayElement *val);
    QVariant data(ScreenplayElement *element, int row, int role) const;

    void resetSource();
    void clearCurrentIndex();
    void continueFetchingMore();
    void updateCurrentIndexAndCount();

private:
    int m_adapterRowCount = MAX_ELEMENT_COUNT;
    int m_currentIndex = -1;
    int m_initialLoadTreshold = -1;
    QPointer<QTimer> m_fetchMoreTimer;
    QObjectProperty<QObject> m_source;
    QObjectProperty<ScreenplayElement> m_currentElement;
};

#endif // SCREENPLAYADAPTER_H
