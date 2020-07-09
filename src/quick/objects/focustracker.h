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

#ifndef FOCUSTRACKER_H
#define FOCUSTRACKER_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

class FocusTracker;

class FocusTrackerIndicator : public QObject
{
    Q_OBJECT

public:
    ~FocusTrackerIndicator();

    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged)
    void setTarget(QObject* val);
    QObject* target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
    void setProperty(const QString &val);
    QString property() const { return m_property; }
    Q_SIGNAL void propertyChanged();

    Q_PROPERTY(QVariant onValue READ onValue WRITE setOnValue NOTIFY onValueChanged)
    void setOnValue(const QVariant &val);
    QVariant onValue() const { return m_onValue; }
    Q_SIGNAL void onValueChanged();

    Q_PROPERTY(QVariant offValue READ offValue WRITE setOffValue NOTIFY offValueChanged)
    void setOffValue(const QVariant &val);
    QVariant offValue() const { return m_offValue; }
    Q_SIGNAL void offValueChanged();

private:
    friend class FocusTracker;
    FocusTrackerIndicator(FocusTracker *parent=nullptr);
    void apply();

private:
    QObject* m_target = nullptr;
    QVariant m_onValue;
    QQuickItem* m_item = nullptr;
    QString m_property;
    QVariant m_offValue;
    FocusTracker *m_tracker = nullptr;
};

class FocusTracker : public QObject
{
    Q_OBJECT

public:
    FocusTracker(QObject *parent=nullptr);
    ~FocusTracker();

    static FocusTracker *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QQuickItem* item READ item CONSTANT)
    QQuickItem *item() const { return m_item; }

    Q_PROPERTY(QQuickWindow* window READ window WRITE setWindow NOTIFY windowChanged)
    void setWindow(QQuickWindow* val);
    QQuickWindow* window() const { return m_window; }
    Q_SIGNAL void windowChanged();

    Q_PROPERTY(bool hasFocus READ hasFocus NOTIFY hasFocusChanged)
    bool hasFocus() const { return m_hasFocus; }
    Q_SIGNAL void hasFocusChanged();

    Q_PROPERTY(FocusTrackerIndicator* indicator READ indicator CONSTANT)
    FocusTrackerIndicator* indicator() const { return m_indicator; }

private:
    void setHasFocus(bool val);
    void evaluateHasFocus();

private:
    bool m_hasFocus = false;
    QQuickItem *m_item = nullptr;
    QQuickWindow* m_window = nullptr;
    FocusTrackerIndicator* m_indicator = new FocusTrackerIndicator(this);
};
Q_DECLARE_METATYPE(FocusTracker*)
QML_DECLARE_TYPEINFO(FocusTracker, QML_HAS_ATTACHED_PROPERTIES)

#endif // FOCUSTRACKER_H
