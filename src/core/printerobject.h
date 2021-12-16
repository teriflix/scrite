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

#ifndef PRINTEROBJECT_H
#define PRINTEROBJECT_H

#include <QPrinter>

class PrinterObject : public QObject, public QPrinter
{
    Q_OBJECT

public:
    PrinterObject(QObject *parent = nullptr) : QObject(parent) { }
    ~PrinterObject() { }
};

#endif // PRINTEROBJECT_H
