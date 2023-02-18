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

#ifndef CONTEXTMENUEVENT_H
#define CONTEXTMENUEVENT_H

#include <QQmlEngine>

class QQuickItem;

class ContextMenuEvent : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(ContextMenuEvent)

public:
    ContextMenuEvent(QObject *parent = nullptr);
    ~ContextMenuEvent();

    static ContextMenuEvent *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const { return m_active; }
    Q_SIGNAL void activeChanged();

    enum Mode { GlobalEventFilterMode, ItemEventFilterMode };
    Q_ENUM(Mode)

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    void setMode(Mode val);
    Mode mode() const { return m_mode; }
    Q_SIGNAL void modeChanged();

signals:
    void popup(const QPointF &mouse);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void setupEventFilter();

private:
    bool m_active = true;
    QQuickItem *m_item = nullptr;
    Mode m_mode = ItemEventFilterMode;
    QObject *m_eventFilterTarget = nullptr;
};

#endif // CONTEXTMENUEVENT_H
