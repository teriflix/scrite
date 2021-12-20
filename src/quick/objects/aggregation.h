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
    Aggregation(QObject *parent = nullptr);
    ~Aggregation();

    Q_INVOKABLE QObject *find(QObject *object, const QString &className,
                              const QString &objectName = QString()) const;
    Q_INVOKABLE ErrorReport *findErrorReport(QObject *object) const;
    Q_INVOKABLE ProgressReport *findProgressReport(QObject *object) const;
};

#endif // AGGREGATION_H
