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

#ifndef PDFEXPORTABLEGRAPHICSSCENE_H
#define PDFEXPORTABLEGRAPHICSSCENE_H

#include <QGraphicsScene>

class PdfExportableGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    PdfExportableGraphicsScene(QObject *parent=nullptr);
    ~PdfExportableGraphicsScene();

    Q_PROPERTY(QString pdfTitle READ pdfTitle WRITE setPdfTitle NOTIFY pdfTitleChanged)
    void setPdfTitle(QString val);
    QString pdfTitle() const { return m_pdfTitle; }
    Q_SIGNAL void pdfTitleChanged();

    bool exportToPdf(const QString &fileName);
    bool exportToPdf(QIODevice *device);

private:
    QString m_pdfTitle;
};

#endif // PDFEXPORTABLEGRAPHICSSCENE_H
