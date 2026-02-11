/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "aggregation.h"

#include "utils.h"

Aggregation::Aggregation(QObject *parent) : QObject(parent) { }

Aggregation::~Aggregation() { }

Aggregation *Aggregation::qmlAttachedProperties(QObject *object)
{
    return new Aggregation(object);
}

ErrorReport *Aggregation::errorReport(QObject *object)
{
    return object->findChild<ErrorReport *>(QString(), Qt::FindDirectChildrenOnly);
}

ProgressReport *Aggregation::progressReport(QObject *object)
{
    return object->findChild<ProgressReport *>(QString(), Qt::FindDirectChildrenOnly);
}

QObject *Aggregation::firstChildByType(const QString &typeName)
{
    return Utils::Object::firstChildByType(this->parent(), typeName);
}

QObject *Aggregation::firstParentByType(const QString &typeName)
{
    return Utils::Object::firstParentByType(this->parent(), typeName);
}

QObject *Aggregation::firstSiblingByType(const QString &typeName)
{
    return Utils::Object::firstSiblingByType(this->parent(), typeName);
}

QObject *Aggregation::firstChildByName(const QString &objectName)
{
    return Utils::Object::firstChildByName(this->parent(), objectName);
}

QObject *Aggregation::firstParentByName(const QString &objectName)
{
    return Utils::Object::firstParentByName(this->parent(), objectName);
}

QObject *Aggregation::firstSiblingByName(const QString &objectName)
{
    return Utils::Object::firstSiblingByName(this->parent(), objectName);
}
