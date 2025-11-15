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

#ifndef STRUCTUREEXPORTER_H
#define STRUCTUREEXPORTER_H

#include "abstractexporter.h"

class StructureExporter : public AbstractExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Structure/Screenplay Structure")
    Q_CLASSINFO("NameFilters", "Adobe PDF (*.pdf)")
    Q_CLASSINFO("Description", "Exports the contents of the entire structure canvas as a single page PDF file.")
    Q_CLASSINFO("Icon", ":/icons/exporter/structure_pdf.png")
    // clang-format on

public:
    Q_INVOKABLE explicit StructureExporter(QObject *parent = nullptr);
    ~StructureExporter();

    // AbstractExporter interface
    bool requiresConfiguration() const { return true; }

    // clang-format off
    Q_CLASSINFO("insertTitleCard_FieldLabel", "Include title card in the generated PDF.")
    Q_CLASSINFO("insertTitleCard_FieldEditor", "CheckBox")
    Q_PROPERTY(bool insertTitleCard
               READ isInsertTitleCard
               WRITE setInsertTitleCard
               NOTIFY insertTitleCardChanged)
    // clang-format on
    void setInsertTitleCard(bool val);
    bool isInsertTitleCard() const { return m_insertTitleCard; }
    Q_SIGNAL void insertTitleCardChanged();

    // clang-format off
    Q_CLASSINFO("enableHeaderFooter_FieldLabel", "Include header & footer in the generated PDF.")
    Q_CLASSINFO("enableHeaderFooter_FieldEditor", "CheckBox")
    Q_PROPERTY(bool enableHeaderFooter
               READ isEnableHeaderFooter
               WRITE setEnableHeaderFooter
               NOTIFY enableHeaderFooterChanged)
    // clang-format on
    void setEnableHeaderFooter(bool val);
    bool isEnableHeaderFooter() const { return m_enableHeaderFooter; }
    Q_SIGNAL void enableHeaderFooterChanged();

    // clang-format off
    Q_CLASSINFO("preferFeaturedImage_FieldLabel", "Include featured image for scene, if available.")
    Q_CLASSINFO("preferFeaturedImage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool preferFeaturedImage
               READ isPreferFeaturedImage
               WRITE setPreferFeaturedImage
               NOTIFY preferFeaturedImageChanged)
    // clang-format on
    void setPreferFeaturedImage(bool val);
    bool isPreferFeaturedImage() const { return m_preferFeaturedImage; }
    Q_SIGNAL void preferFeaturedImageChanged();

    // clang-format off
    Q_CLASSINFO("watermark_FieldLabel", "Watermark text, if enabled.")
    Q_CLASSINFO("watermark_FieldEditor", "TextBox")
    Q_CLASSINFO("watermark_IsPersistent", "false")
    Q_CLASSINFO("watermark_Feature", "watermark")
    Q_PROPERTY(QString watermark
               READ watermark
               WRITE setWatermark
               NOTIFY watermarkChanged)
    // clang-format on
    void setWatermark(const QString &val);
    QString watermark() const { return m_watermark; }
    Q_SIGNAL void watermarkChanged();

    // clang-format off
    Q_CLASSINFO("comment_FieldLabel", "Comment text for use in title card.")
    Q_CLASSINFO("comment_FieldEditor", "TextBox")
    Q_CLASSINFO("comment_IsPersistent", "false")
    Q_PROPERTY(QString comment
               READ comment
               WRITE setComment
               NOTIFY commentChanged)
    // clang-format on
    void setComment(const QString &val);
    QString comment() const { return m_comment; }
    Q_SIGNAL void commentChanged();

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString fileNameExtension() const { return QStringLiteral("pdf"); }

private:
    bool m_insertTitleCard = true;
    bool m_enableHeaderFooter = true;
    QString m_comment;
    QString m_watermark;
    bool m_preferFeaturedImage = false;
};

#endif // STRUCTUREEXPORTER_H
