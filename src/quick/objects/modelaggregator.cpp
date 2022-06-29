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

#include "modelaggregator.h"

#include <QTimerEvent>

ModelAggregator::ModelAggregator(QObject *parent) : QObject(parent), m_model(this, "model") { }

ModelAggregator::~ModelAggregator() { }

void ModelAggregator::setModel(QAbstractItemModel *val)
{
    if (m_model == val)
        return;

    if (!m_model.isNull()) {
        disconnect(m_model, &QAbstractItemModel::rowsInserted, this,
                   &ModelAggregator::evaluateAggregateValueLater);
        disconnect(m_model, &QAbstractItemModel::rowsRemoved, this,
                   &ModelAggregator::evaluateAggregateValueLater);
        disconnect(m_model, &QAbstractItemModel::rowsMoved, this,
                   &ModelAggregator::evaluateAggregateValueLater);
        disconnect(m_model, &QAbstractItemModel::dataChanged, this,
                   &ModelAggregator::evaluateAggregateValueLater);
        disconnect(m_model, &QAbstractItemModel::modelReset, this,
                   &ModelAggregator::evaluateAggregateValueLater);
    }

    m_model = val;

    if (!m_model.isNull()) {
        connect(m_model, &QAbstractItemModel::rowsInserted, this,
                &ModelAggregator::evaluateAggregateValueLater);
        connect(m_model, &QAbstractItemModel::rowsRemoved, this,
                &ModelAggregator::evaluateAggregateValueLater);
        connect(m_model, &QAbstractItemModel::rowsMoved, this,
                &ModelAggregator::evaluateAggregateValueLater);
        connect(m_model, &QAbstractItemModel::dataChanged, this,
                &ModelAggregator::evaluateAggregateValueLater);
        connect(m_model, &QAbstractItemModel::modelReset, this,
                &ModelAggregator::evaluateAggregateValueLater);
    }

    emit modelChanged();

    if (m_rootIndex.isValid() && m_rootIndex.model() != m_model)
        this->setRootIndex(QModelIndex());

    if (m_model.isNull())
        this->setColumn(-1);
    else
        this->setColumn(qBound(0, m_column, m_model->columnCount(m_rootIndex) - 1));
}

void ModelAggregator::setRootIndex(const QModelIndex &val)
{
    if (m_rootIndex == val)
        return;

    m_rootIndex = val;
    emit rootIndexChanged();
}

void ModelAggregator::setColumn(int val)
{
    if (m_column == val)
        return;

    m_column = val;
    emit columnChanged();
}

void ModelAggregator::setInitialValue(const QVariant &val)
{
    if (m_initialValue == val)
        return;

    m_initialValue = val;
    emit initialValueChanged();
}

void ModelAggregator::setDelay(int val)
{
    if (m_delay == val)
        return;

    m_delay = val;
    emit delayChanged();
}

void ModelAggregator::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_evaluateTimer.timerId()) {
        m_evaluateTimer.stop();
        this->evaluateAggregateValue();
    } else
        QObject::timerEvent(te);
}

void ModelAggregator::setAggregateValue(const QVariant &val)
{
    if (m_aggregateValue == val)
        return;

    m_aggregateValue = val;
    emit aggregateValueChanged();
}

void ModelAggregator::resetModel()
{
    m_model = nullptr;
    emit modelChanged();
}

void ModelAggregator::evaluateAggregateValue()
{
    if (m_aggregateFunction == nullptr || m_model.isNull()) {
        this->setAggregateValue(QVariant());
        return;
    }

    QVariant avalue = m_initialValue;

    const int nrRows = m_model->rowCount(m_rootIndex);
    for (int i = 0; i < nrRows; i++) {
        const QModelIndex index = m_model->index(i, m_column, m_rootIndex);
        m_aggregateFunction(index, avalue);
    }

    if (m_finalizeFunction)
        m_finalizeFunction(avalue);

    this->setAggregateValue(avalue);
}

void ModelAggregator::evaluateAggregateValueLater()
{
    if (m_aggregateFunction != nullptr && m_model != nullptr)
        m_evaluateTimer.start(m_delay, this);
    else
        m_evaluateTimer.stop();
}
