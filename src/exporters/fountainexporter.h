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
    Q_CLASSINFO( "followStrictSyntax_FieldLabel", "Use ., @, !, > to explicitly mark scene heading, character, action and transisitions.")
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

    bool canCopyToClipboard() const { return true; }
    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return false; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString fileNameExtension() const { return QStringLiteral("fountain"); }

private:
    bool m_useEmphasis = true;
    bool m_followStrictSyntax = true;
};

#endif // FOUNTAINEXPORTER_H
