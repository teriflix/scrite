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

#ifndef ABSTRACTREPORTGENERATOR_H
#define ABSTRACTREPORTGENERATOR_H

#include "abstractdeviceio.h"

#include <QEventLoop>
#include <QTextDocument>

class AbstractReportGenerator : public AbstractDeviceIO
{
    Q_OBJECT

public:
    ~AbstractReportGenerator();

    enum Format
    {
        AdobePDF,
        OpenDocumentFormat
    };
    Q_ENUM(Format)
    Q_PROPERTY(Format format READ format WRITE setFormat NOTIFY formatChanged STORED false)
    void setFormat(Format val);
    Format format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_INVOKABLE bool generate();

    Q_PROPERTY(bool requiresConfiguration READ requiresConfiguration CONSTANT)
    virtual bool requiresConfiguration() const { return false; }

    Q_INVOKABLE bool setConfigurationValue(const QString &name, const QVariant &value) {
        return this->setProperty(qPrintable(name),value);
    }
    Q_INVOKABLE QVariant getConfigurationValue(const QString &name) const {
        return this->property(qPrintable(name));
    }

    Q_INVOKABLE QJsonObject configurationFormInfo() const;
    Q_INVOKABLE void accept() { m_eventLoop.exit(0); }
    Q_INVOKABLE void reject() { m_eventLoop.exit(1); }
    Q_INVOKABLE bool isInExec() const { return m_eventLoop.isRunning(); }
    Q_INVOKABLE bool exec() { return m_eventLoop.exec() == 0; }

protected:
    // AbstractDeviceIO interface
    QString polishFileName(const QString &fileName) const;

protected:
    AbstractReportGenerator(QObject *parent=nullptr);
    virtual bool doGenerate(QTextDocument *document) = 0;

private:
    Format m_format;
    QEventLoop m_eventLoop;
};

#endif // ABSTRACTREPORTGENERATOR_H
