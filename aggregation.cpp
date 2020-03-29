/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "aggregation.h"

Aggregation::Aggregation(QObject *parent)
            :QObject(parent)
{

}

Aggregation::~Aggregation()
{

}

QObject *Aggregation::find(QObject *object, const QString &className, const QString &objectName) const
{
    if(object == nullptr)
        return nullptr;

    QObjectList children = object->children();
    Q_FOREACH(QObject *child, children)
    {
        if(child->inherits(qPrintable(className)))
        {
            if(objectName.isEmpty())
                return child;

            if(child->objectName() == objectName)
                return child;
        }
    }

    return nullptr;
}


ErrorReport *Aggregation::findErrorReport(QObject *object) const
{
    return object->findChild<ErrorReport*>();
}

ProgressReport *Aggregation::findProgressReport(QObject *object) const
{
    return object->findChild<ProgressReport*>();
}
