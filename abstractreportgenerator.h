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

#ifndef ABSTRACTREPORTGENERATOR_H
#define ABSTRACTREPORTGENERATOR_H

#include "abstractdeviceio.h"
#include "garbagecollector.h"

#include <QEventLoop>
#include <QTextDocument>

class QPdfWriter;
class QTextDocumentWriter;

class AbstractReportGenerator : public AbstractDeviceIO
{
    Q_OBJECT

public:
    ~AbstractReportGenerator();
    Q_SIGNAL void aboutToDelete(AbstractReportGenerator *gen);

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

    Q_INVOKABLE virtual bool supportsFormat(Format) const { return true; }

    Q_PROPERTY(QString name READ name CONSTANT)
    QString name() const;

    Q_PROPERTY(bool requiresConfiguration READ requiresConfiguration CONSTANT)
    virtual bool requiresConfiguration() const { return false; }

    Q_INVOKABLE bool setConfigurationValue(const QString &name, const QVariant &value);
    Q_INVOKABLE QVariant getConfigurationValue(const QString &name) const;
    Q_INVOKABLE QJsonObject configurationFormInfo() const;

    Q_INVOKABLE bool generate();
    Q_INVOKABLE void discard() { GarbageCollector::instance()->add(this); }

protected:
    // AbstractDeviceIO interface
    QString polishFileName(const QString &fileName) const;

protected:
    AbstractReportGenerator(QObject *parent=nullptr);
    virtual bool doGenerate(QTextDocument *) { return false; }
    virtual void configureWriter(QTextDocumentWriter *, const QTextDocument *) const { }
    virtual void configureWriter(QPdfWriter *, const QTextDocument *) const { }

    virtual bool canDirectPrintToPdf() const { return false; }
    virtual bool directPrintToPdf(QPdfWriter *) { return false; }

private:
    Format m_format = AdobePDF;
    QEventLoop m_eventLoop;
};

#endif // ABSTRACTREPORTGENERATOR_H
