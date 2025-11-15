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

#ifndef ERRORREPORT_H
#define ERRORREPORT_H

#include <QQmlEngine>
#include <QJsonObject>
#include <QAbstractListModel>

#include "qobjectproperty.h"

class ErrorReport : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit ErrorReport(QObject *parent = nullptr);
    ~ErrorReport();
    Q_SIGNAL void aboutToDelete(ErrorReport *val);

    // clang-format off
    Q_PROPERTY(ErrorReport *proxyFor
               READ proxyFor
               WRITE setProxyFor
               NOTIFY proxyForChanged
               RESET resetProxyFor)
    // clang-format on
    void setProxyFor(ErrorReport *val);
    ErrorReport *proxyFor() const { return m_proxyFor; }
    Q_SIGNAL void proxyForChanged();

    // clang-format off
    Q_PROPERTY(QString errorMessage
               READ errorMessage
               NOTIFY errorMessageChanged)
    // clang-format on
    void setErrorMessage(const QString &val, const QJsonObject &details = QJsonObject());
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject details
               READ details
               NOTIFY errorMessageChanged)
    // clang-format on
    QJsonObject details() const { return m_details; }

    // clang-format off
    Q_PROPERTY(bool hasError
               READ hasError
               NOTIFY errorMessageChanged)
    // clang-format on
    bool hasError() const { return !m_errorMessage.isEmpty(); }

    // clang-format off
    Q_PROPERTY(int warningMessageCount
               READ warningMessageCount
               NOTIFY warningMessageCountChanged)
    // clang-format on
    int warningMessageCount() const { return m_warningMessages.size(); }
    Q_SIGNAL void warningMessageCountChanged();

    // clang-format off
    Q_PROPERTY(QString lastWarningMessage
               READ lastWarningMessage
               NOTIFY warningMessageCountChanged)
    // clang-format on
    QString lastWarningMessage() const
    {
        return m_warningMessages.empty() ? QString() : m_warningMessages.last();
    }

    // clang-format off
    Q_PROPERTY(QStringList warningMessages
               READ warningMessages
               NOTIFY warningMessageCountChanged)
    // clang-format on
    QStringList warningMessages() const { return m_warningMessages; }

    Q_INVOKABLE void clear();
    void addWarning(const QString &warning);

    // QAbstractItemModel interface
    enum Role { WarningMessageRole = Qt::DisplayRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void resetProxyFor();
    void updateErrorMessageFromProxy();
    void updateWarningMessageFromProxy();

private:
    QJsonObject m_details;
    QString m_errorMessage;
    QStringList m_warningMessages;
    QObjectProperty<ErrorReport> m_proxyFor;
};

QML_DECLARE_TYPE(ErrorReport)

#endif // ERRORREPORT_H
