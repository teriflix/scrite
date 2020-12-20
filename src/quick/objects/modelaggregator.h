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

#ifndef MODELAGGREGATOR_H
#define MODELAGGREGATOR_H

#include <QAbstractItemModel>

#include "qobjectproperty.h"
#include "execlatertimer.h"

class ModelAggregator : public QObject
{
    Q_OBJECT

public:
    ModelAggregator(QObject *parent = nullptr);
    ~ModelAggregator();

    typedef std::function<void(const QModelIndex &,QVariant &)> AggregateFunction;
    void setAggregateFunction(AggregateFunction val) { m_aggregateFunction = val; }
    AggregateFunction aggregateFunction() const { return m_aggregateFunction; }

    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel RESET resetModel NOTIFY modelChanged)
    void setModel(QAbstractItemModel* val);
    QAbstractItemModel* model() const { return m_model; }
    Q_SIGNAL void modelChanged();

    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    void setRootIndex(const QModelIndex &val);
    QModelIndex rootIndex() const { return m_rootIndex; }
    Q_SIGNAL void rootIndexChanged();

    Q_PROPERTY(int column READ column WRITE setColumn NOTIFY columnChanged)
    void setColumn(int val);
    int column() const { return m_column; }
    Q_SIGNAL void columnChanged();

    Q_PROPERTY(QVariant aggregateValue READ aggregateValue NOTIFY aggregateValueChanged)
    QVariant aggregateValue() const { return m_aggregateValue; }
    Q_SIGNAL void aggregateValueChanged();

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

protected:
    void timerEvent(QTimerEvent *te);

private:
    void setAggregateValue(const QVariant &val);
    void resetModel();

    void evaluateAggregateValue();
    void evaluateAggregateValueLater();

private:
    int m_delay = 0;
    int m_column = -1;
    QModelIndex m_rootIndex;
    QVariant m_aggregateValue;
    ExecLaterTimer m_evaluateTimer;
    AggregateFunction m_aggregateFunction;
    QObjectProperty<QAbstractItemModel> m_model;
};

#endif // MODELAGGREGATOR_H
