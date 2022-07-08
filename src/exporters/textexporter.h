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

#ifndef TEXTEXPORTER_H
#define TEXTEXPORTER_H

#include "abstractexporter.h"

class TextExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Text File")
    Q_CLASSINFO("NameFilters", "Text File (*.txt)")

public:
    Q_INVOKABLE explicit TextExporter(QObject *parent = nullptr);
    ~TextExporter();

    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return true; }

    Q_CLASSINFO("maxLettersPerLine_FieldLabel", "Number of characters per line")
    Q_CLASSINFO("maxLettersPerLine_FieldEditor", "IntegerSpinBox")
    Q_CLASSINFO("maxLettersPerLine_FieldMinValue", "30")
    Q_CLASSINFO("maxLettersPerLine_FieldMaxValue", "150")
    Q_CLASSINFO("maxLettersPerLine_FieldDefaultValue", "60")
    Q_PROPERTY(int maxLettersPerLine READ maxLettersPerLine WRITE setMaxLettersPerLine NOTIFY maxLettersPerLineChanged)
    void setMaxLettersPerLine(int val);
    int maxLettersPerLine() const { return m_maxLettersPerLine; }
    Q_SIGNAL void maxLettersPerLineChanged();

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface

private:
    int m_maxLettersPerLine = 60;
};

#endif // TEXTEXPORTER_H
