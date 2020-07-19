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

#ifndef ERRORREPORT_H
#define ERRORREPORT_H

#include <QAbstractListModel>

#include "qobjectproperty.h"

class ErrorReport : public QAbstractListModel
{
    Q_OBJECT

public:
    ErrorReport(QObject *parent=nullptr);
    ~ErrorReport();
    Q_SIGNAL void aboutToDelete(ErrorReport *val);

    Q_PROPERTY(ErrorReport* proxyFor READ proxyFor WRITE setProxyFor NOTIFY proxyForChanged RESET resetProxyFor)
    void setProxyFor(ErrorReport* val);
    ErrorReport* proxyFor() const { return m_proxyFor; }
    Q_SIGNAL void proxyForChanged();

    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage NOTIFY errorMessageChanged)
    void setErrorMessage(const QString &val);
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorMessageChanged)
    bool hasError() const { return !m_errorMessage.isEmpty(); }

    Q_PROPERTY(int warningMessageCount READ warningMessageCount NOTIFY warningMessageCountChanged)
    int warningMessageCount() const { return m_warningMessages.size(); }
    Q_SIGNAL void warningMessageCountChanged();

    Q_PROPERTY(QString lastWarningMessage READ lastWarningMessage NOTIFY warningMessageCountChanged)
    QString lastWarningMessage() const { return m_warningMessages.empty() ? QString() : m_warningMessages.last(); }

    Q_PROPERTY(QStringList warningMessages READ warningMessages NOTIFY warningMessageCountChanged)
    QStringList warningMessages() const { return m_warningMessages; }

    Q_INVOKABLE void clear();
    void addWarning(const QString &warning);

    // QAbstractItemModel interface
    enum Role { WarningMessageRole=Qt::DisplayRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

private:
    void resetProxyFor();
    void updateErrorMessageFromProxy();
    void updateWarningMessageFromProxy();

private:
    QString m_errorMessage;
    QStringList m_warningMessages;
    QObjectProperty<ErrorReport> m_proxyFor;
};

#endif // ERRORREPORT_H
