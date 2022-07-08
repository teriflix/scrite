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
    QML_UNCREATABLE("Instantiation from QML not allowed.")
    QML_SINGLETON

public:
    explicit Aggregation(QObject *parent = nullptr);
    ~Aggregation();

    Q_INVOKABLE static QObject *find(QObject *object, const QString &className,
                                     const QString &objectName = QString());
    Q_INVOKABLE static ErrorReport *findErrorReport(QObject *object);
    Q_INVOKABLE static ProgressReport *findProgressReport(QObject *object);
};

#endif // AGGREGATION_H
