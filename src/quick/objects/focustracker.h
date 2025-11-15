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

#ifndef FOCUSTRACKER_H
#define FOCUSTRACKER_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

#include "qobjectproperty.h"

class FocusTracker;

class FocusTrackerIndicator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~FocusTrackerIndicator();

    // clang-format off
    Q_PROPERTY(QObject *target
               READ target
               WRITE setTarget
               NOTIFY targetChanged
               RESET resetTarget)
    // clang-format on
    void setTarget(QObject *val);
    QObject *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    // clang-format off
    Q_PROPERTY(QString property
               READ property
               WRITE setProperty
               NOTIFY propertyChanged)
    // clang-format on
    void setProperty(const QString &val);
    QString property() const { return m_property; }
    Q_SIGNAL void propertyChanged();

    // clang-format off
    Q_PROPERTY(QVariant onValue
               READ onValue
               WRITE setOnValue
               NOTIFY onValueChanged)
    // clang-format on
    void setOnValue(const QVariant &val);
    QVariant onValue() const { return m_onValue; }
    Q_SIGNAL void onValueChanged();

    // clang-format off
    Q_PROPERTY(QVariant offValue
               READ offValue
               WRITE setOffValue
               NOTIFY offValueChanged)
    // clang-format on
    void setOffValue(const QVariant &val);
    QVariant offValue() const { return m_offValue; }
    Q_SIGNAL void offValueChanged();

private:
    friend class FocusTracker;
    FocusTrackerIndicator(FocusTracker *parent = nullptr);
    void apply();
    void resetTarget();

private:
    QVariant m_onValue = true;
    QQuickItem *m_item = nullptr;
    QString m_property;
    QVariant m_offValue = false;
    FocusTracker *m_tracker = nullptr;
    QObjectProperty<QObject> m_target;
};

class FocusTracker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(FocusTracker)

public:
    explicit FocusTracker(QObject *parent = nullptr);
    ~FocusTracker();

    static FocusTracker *qmlAttachedProperties(QObject *object);

    // clang-format off
    Q_PROPERTY(QQuickItem *item
               READ item
               CONSTANT )
    // clang-format on
    QQuickItem *item() const { return m_item; }

    // clang-format off
    Q_PROPERTY(QQuickWindow *window
               READ window
               WRITE setWindow
               NOTIFY windowChanged
               RESET resetWindow)
    // clang-format on
    void setWindow(QQuickWindow *val);
    QQuickWindow *window() const { return m_window; }
    Q_SIGNAL void windowChanged();

    enum FocusEvaluationMethod { StandardFocusEvaluation, ExclusiveFocusEvaluation };
    Q_ENUM(FocusEvaluationMethod)

    // clang-format off
    Q_PROPERTY(FocusEvaluationMethod evaluationMethod
               READ evaluationMethod
               WRITE setEvaluationMethod
               NOTIFY evaluationMethodChanged)
    // clang-format on
    void setEvaluationMethod(FocusEvaluationMethod val);
    FocusEvaluationMethod evaluationMethod() const { return m_evaluationMethod; }
    Q_SIGNAL void evaluationMethodChanged();

    // clang-format off
    Q_PROPERTY(bool hasFocus
               READ hasFocus
               NOTIFY hasFocusChanged)
    // clang-format on
    bool hasFocus() const { return m_hasFocus; }
    Q_SIGNAL void hasFocusChanged();

    // clang-format off
    Q_PROPERTY(FocusTrackerIndicator *indicator
               READ indicator
               CONSTANT )
    // clang-format on
    FocusTrackerIndicator *indicator() const { return m_indicator; }

private:
    void resetWindow();
    void setHasFocus(bool val);
    void evaluateHasFocus();

private:
    bool m_hasFocus = false;
    QQuickItem *m_item = nullptr;
    FocusTrackerIndicator *m_indicator = new FocusTrackerIndicator(this);
    FocusEvaluationMethod m_evaluationMethod = StandardFocusEvaluation;
    QObjectProperty<QQuickWindow> m_window;
};

class FocusInspector : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    // Returns true of item or any child of item has focus
    Q_INVOKABLE static bool hasFocus(QQuickItem *item);
};

#endif // FOCUSTRACKER_H
