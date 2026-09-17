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

#ifndef FOUNTAINEXPORTER_H
#define FOUNTAINEXPORTER_H

#include "abstractexporter.h"

class FountainExporter : public AbstractExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Screenplay/Fountain")
    Q_CLASSINFO("NameFilters", "Fountain (*.fountain)")
    Q_CLASSINFO("Description", "Exports the current screenplay to Fountain file format.")
    Q_CLASSINFO("Icon", ":/icons/exporter/fountain.png")
    // clang-format on

public:
    Q_INVOKABLE explicit FountainExporter(QObject *parent = nullptr);
    ~FountainExporter();

    // clang-format off
    Q_CLASSINFO("followStrictSyntax_FieldLabel", "Use ., @, !, > to explicitly mark scene heading, character, action and transisitions.")
    Q_CLASSINFO("followStrictSyntax_FieldEditor", "CheckBox")
    Q_PROPERTY(bool followStrictSyntax
               READ isFollowStrictSyntax
               WRITE setFollowStrictSyntax
               NOTIFY followStrictSyntaxChanged)
    // clang-format on
    void setFollowStrictSyntax(bool val);
    bool isFollowStrictSyntax() const { return m_followStrictSyntax; }
    Q_SIGNAL void followStrictSyntaxChanged();

    // clang-format off
    Q_CLASSINFO("useEmphasis_FieldLabel", "Use *, ** and _ to highlight italics, bold and underlined text.")
    Q_CLASSINFO("useEmphasis_FieldEditor", "CheckBox")
    Q_PROPERTY(bool useEmphasis
               READ isUseEmphasis
               WRITE setUseEmphasis
               NOTIFY useEmphasisChanged)
    // clang-format on
    void setUseEmphasis(bool val);
    bool isUseEmphasis() const { return m_useEmphasis; }
    Q_SIGNAL void useEmphasisChanged();

    // clang-format off
    Q_CLASSINFO("includeTitlePage_FieldLabel", "Include title page fields.")
    Q_CLASSINFO("includeTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeTitlePage
               READ isIncludeTitlePage
               WRITE setIncludeTitlePage
               NOTIFY includeTitlePageChanged)
    // clang-format on
    void setIncludeTitlePage(bool val);
    bool isIncludeTitlePage() const { return m_includeTitlePage; }
    Q_SIGNAL void includeTitlePageChanged();

    // clang-format off
    Q_CLASSINFO("includeActBreaks_FieldLabel", "Include act breaks.")
    Q_CLASSINFO("includeActBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeActBreaks
               READ isIncludeActBreaks
               WRITE setIncludeActBreaks
               NOTIFY includeActBreaksChanged)
    // clang-format on
    void setIncludeActBreaks(bool val);
    bool isIncludeActBreaks() const { return m_includeActBreaks; }
    Q_SIGNAL void includeActBreaksChanged();

    // clang-format off
    Q_CLASSINFO("includeEpisodeBreaks_FieldLabel", "Include episode breaks.")
    Q_CLASSINFO("includeEpisodeBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeEpisodeBreaks
               READ isIncludeEpisodeBreaks
               WRITE setIncludeEpisodeBreaks
               NOTIFY includeEpisodeBreaksChanged)
    // clang-format on
    void setIncludeEpisodeBreaks(bool val);
    bool isIncludeEpisodeBreaks() const { return m_includeEpisodeBreaks; }
    Q_SIGNAL void includeEpisodeBreaksChanged();

    bool canCopyToClipboard() const { return true; }
    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return false; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString fileNameExtension() const { return QStringLiteral("fountain"); }

private:
    bool m_useEmphasis = true;
    bool m_followStrictSyntax = true;
    bool m_includeTitlePage = true;
    bool m_includeActBreaks = true;
    bool m_includeEpisodeBreaks = true;
};

#endif // FOUNTAINEXPORTER_H
