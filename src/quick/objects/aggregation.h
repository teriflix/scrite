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

#ifndef AGGREGATION_H
#define AGGREGATION_H

#include <QQmlEngine>

#include "errorreport.h"
#include "progressreport.h"

class Aggregation : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(Aggregation)

public:
    explicit Aggregation(QObject *parent = nullptr);
    ~Aggregation();

    static Aggregation *qmlAttachedProperties(QObject *object);

    Q_INVOKABLE static ErrorReport *errorReport(QObject *object);
    Q_INVOKABLE static ProgressReport *progressReport(QObject *object);

    Q_INVOKABLE QObject *firstChildByType(const QString &typeName);
    Q_INVOKABLE QObject *firstParentByType(const QString &typeName);
    Q_INVOKABLE QObject *firstSiblingByType(const QString &typeName);

    Q_INVOKABLE QObject *firstChildByName(const QString &objectName);
    Q_INVOKABLE QObject *firstParentByName(const QString &objectName);
    Q_INVOKABLE QObject *firstSiblingByName(const QString &objectName);
};

#endif // AGGREGATION_H
