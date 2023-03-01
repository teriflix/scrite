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

#ifndef ABSTRACTDEVICEIO_H
#define ABSTRACTDEVICEIO_H

#include <QObject>

#include "errorreport.h"
#include "scritedocument.h"
#include "progressreport.h"
#include "qobjectproperty.h"

class QIODevice;

class AbstractDeviceIO : public QObject
{
    Q_OBJECT

public:
    ~AbstractDeviceIO();

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    void setFileName(const QString &val);
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    Q_PROPERTY(ScriteDocument* document READ document WRITE setDocument NOTIFY documentChanged RESET resetDocument)
    void setDocument(ScriteDocument *val);
    ScriteDocument *document() const { return m_document; }
    Q_SIGNAL void documentChanged();

protected:
    AbstractDeviceIO(QObject *parent = nullptr);
    virtual QString polishFileName(const QString &fileName) const;
    virtual QString fileNameExtension() const { return QString(); }
    void resetDocument();

    ProgressReport *progress() const { return m_progressReport; }
    ErrorReport *error() const { return m_errorReport; }

private:
    QString m_fileName;
    ErrorReport *m_errorReport = new ErrorReport(this);
    QObjectProperty<ScriteDocument> m_document;
    ProgressReport *m_progressReport = new ProgressReport(this);
};

#endif // ABSTRACTDEVICEIO_H
