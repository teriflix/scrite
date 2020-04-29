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

#ifndef TEXTEXPORTER_H
#define TEXTEXPORTER_H

#include "abstractexporter.h"

class TextExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Text File")
    Q_CLASSINFO("NameFilters", "Text File (*.txt)")

public:
    Q_INVOKABLE TextExporter(QObject *parent=nullptr);
    ~TextExporter();

    Q_PROPERTY(int maxLettersPerLine READ maxLettersPerLine WRITE setMaxLettersPerLine NOTIFY maxLettersPerLineChanged)
    void setMaxLettersPerLine(int val);
    int maxLettersPerLine() const { return m_maxLettersPerLine; }
    Q_SIGNAL void maxLettersPerLineChanged();

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface

private:
    int m_maxLettersPerLine = 80;
};

#endif // TEXTEXPORTER_H
