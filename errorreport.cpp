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

#include "errorreport.h"

ErrorReport::ErrorReport(QObject *parent)
            :QAbstractListModel(parent)
{

}

ErrorReport::~ErrorReport()
{
    emit aboutToDelete(this);
}

void ErrorReport::setProxyFor(ErrorReport *val)
{
    if(m_proxyFor == val)
        return;

    if(m_proxyFor == nullptr)
    {
        disconnect(m_proxyFor, &ErrorReport::aboutToDelete, this, &ErrorReport::clearProxyFor);
        disconnect(m_proxyFor, &ErrorReport::errorMessageChanged, this, &ErrorReport::updateErrorMessageFromProxy);
        disconnect(m_proxyFor, &ErrorReport::warningMessageCountChanged, this, &ErrorReport::updateWarningMessageFromProxy);
    }

    m_proxyFor = val;

    if(m_proxyFor != nullptr)
    {
        connect(m_proxyFor, &ErrorReport::aboutToDelete, this, &ErrorReport::clearProxyFor);
        connect(m_proxyFor, &ErrorReport::errorMessageChanged, this, &ErrorReport::updateErrorMessageFromProxy);
        connect(m_proxyFor, &ErrorReport::warningMessageCountChanged, this, &ErrorReport::updateWarningMessageFromProxy);

        this->setErrorMessage(m_proxyFor->errorMessage());
        this->beginResetModel();
        m_warningMessages = m_proxyFor->warningMessages();
        this->endResetModel();
        emit warningMessageCountChanged();
    }
    else
        this->clear();

    emit proxyForChanged();
}

void ErrorReport::setErrorMessage(const QString &val)
{
    if(m_errorMessage == val)
        return;

    m_errorMessage = val;
    emit errorMessageChanged();
}

void ErrorReport::clear()
{
    this->beginResetModel();
    m_warningMessages.clear();
    this->endResetModel();
    emit warningMessageCountChanged();

    this->setErrorMessage(QString());
}

void ErrorReport::addWarning(const QString &warning)
{
    this->beginInsertRows(QModelIndex(), m_warningMessages.size(), m_warningMessages.size());
    m_warningMessages << warning;
    this->endInsertRows();
    emit warningMessageCountChanged();
}

int ErrorReport::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_warningMessages.size();
}

QVariant ErrorReport::data(const QModelIndex &index, int role) const
{
    return (index.isValid() && role==WarningMessageRole) ?
                m_warningMessages.value(index.row()) : QString();
}

QHash<int, QByteArray> ErrorReport::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[WarningMessageRole] = "warningMessage";
    return roles;
}

void ErrorReport::clearProxyFor()
{
    this->setProxyFor(nullptr);
}

void ErrorReport::updateErrorMessageFromProxy()
{
    if(m_proxyFor != nullptr)
        this->setErrorMessage(m_proxyFor->errorMessage());
}

void ErrorReport::updateWarningMessageFromProxy()
{
    if(m_proxyFor != nullptr)
        this->addWarning(m_proxyFor->lastWarningMessage());
}
