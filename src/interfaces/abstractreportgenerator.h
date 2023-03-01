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

#ifndef ABSTRACTREPORTGENERATOR_H
#define ABSTRACTREPORTGENERATOR_H

#include "abstractdeviceio.h"
#include "garbagecollector.h"

#include <QTextDocument>

class QPrinter;
class QPdfWriter;
class QTextDocumentWriter;
class QTextDocumentPagedPrinter;

class AbstractReportGenerator : public AbstractDeviceIO
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~AbstractReportGenerator();
    Q_SIGNAL void aboutToDelete(AbstractReportGenerator *gen);

    enum Format { AdobePDF, OpenDocumentFormat };
    Q_ENUM(Format)
    Q_PROPERTY(Format format READ format WRITE setFormat NOTIFY formatChanged STORED false)
    void setFormat(Format val);
    Format format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(QString title READ title CONSTANT)
    QString title() const;

    Q_PROPERTY(QString description READ description CONSTANT)
    QString description() const;

    Q_PROPERTY(bool singlePageReport READ isSinglePageReport CONSTANT)
    virtual bool isSinglePageReport() const { return false; }

    Q_PROPERTY(bool featureEnabled READ isFeatureEnabled NOTIFY featureEnabledChanged)
    bool isFeatureEnabled() const;
    Q_SIGNAL void featureEnabledChanged();

    Q_CLASSINFO("watermark_FieldGroup", "Basic")
    Q_CLASSINFO("watermark_FieldLabel", "Watermark text, if enabled. (PDF Only)")
    Q_CLASSINFO("watermark_FieldEditor", "TextBox")
    Q_PROPERTY(QString watermark READ watermark WRITE setWatermark NOTIFY watermarkChanged)
    void setWatermark(const QString &val);
    QString watermark() const { return m_watermark; }
    Q_SIGNAL void watermarkChanged();

    Q_CLASSINFO("comment_FieldGroup", "Basic")
    Q_CLASSINFO("comment_FieldLabel", "Comment text for use with header & footer. (PDF Only)")
    Q_CLASSINFO("comment_FieldEditor", "TextBox")
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
    void setComment(const QString &val);
    QString comment() const { return m_comment; }
    Q_SIGNAL void commentChanged();

    Q_INVOKABLE virtual bool supportsFormat(AbstractReportGenerator::Format) const { return true; }

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
    QString fileNameExtension() const;

protected:
    AbstractReportGenerator(QObject *parent = nullptr);
    virtual bool usePdfWriter() const;
    virtual bool doGenerate(QTextDocument *) { return false; }
    virtual void configureWriter(QTextDocumentWriter *, const QTextDocument *) const { }
    virtual void configureWriter(QPdfWriter *, const QTextDocument *) const { }
    virtual void configureWriter(QPrinter *, const QTextDocument *) const { }
    virtual void configureTextDocumentPrinter(QTextDocumentPagedPrinter *, const QTextDocument *) {
    }

    virtual bool canDirectPrintToPdf() const { return false; }
    virtual bool directPrintToPdf(QPdfWriter *) { return false; }
    virtual bool directPrintToPdf(QPrinter *) { return false; }

    virtual bool canDirectExportToOdf() const { return false; }
    virtual bool directExportToOdf(QIODevice *) { return false; }
    virtual void polishFormInfo(QJsonObject &) const { return; }

private:
    Format m_format = AdobePDF;
    QString m_comment;
    QString m_watermark;
};

#endif // ABSTRACTREPORTGENERATOR_H
