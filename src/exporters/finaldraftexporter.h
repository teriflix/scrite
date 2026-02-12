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

#ifndef FINALDRAFTEXPORTER_H
#define FINALDRAFTEXPORTER_H

#include "abstractexporter.h"

class FinalDraftExporter : public AbstractExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Screenplay/Final Draft")
    Q_CLASSINFO("NameFilters", "Final Draft (*.fdx)")
    Q_CLASSINFO("Description", "Exports the current screenplay to Final Draft (FDX) file format.")
    Q_CLASSINFO("Icon", ":/icons/exporter/final_draft.png")
    // clang-format on

public:
    Q_INVOKABLE explicit FinalDraftExporter(QObject *parent = nullptr);
    ~FinalDraftExporter();

    // clang-format off
    Q_CLASSINFO("markLanguagesExplicitly_FieldLabel", "Explicity mark text-fragments of different languages.")
    Q_CLASSINFO("markLanguagesExplicitly_FieldEditor", "CheckBox")
    Q_PROPERTY(bool markLanguagesExplicitly
               READ isMarkLanguagesExplicitly
               WRITE setMarkLanguagesExplicitly
               NOTIFY markLanguagesExplicitlyChanged)
    // clang-format on
    void setMarkLanguagesExplicitly(bool val);
    bool isMarkLanguagesExplicitly() const { return m_markLanguagesExplicitly; }
    Q_SIGNAL void markLanguagesExplicitlyChanged();

    // clang-format off
    Q_CLASSINFO("includeSceneSynopsis_FieldLabel", "Include scene synopsis.")
    Q_CLASSINFO("includeSceneSynopsis_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneSynopsis
               READ isIncludeSceneSynopsis
               WRITE setIncludeSceneSynopsis
               NOTIFY includeSceneSynopsisChanged)
    // clang-format on
    void setIncludeSceneSynopsis(bool val);
    bool isIncludeSceneSynopsis() const { return m_includeSceneSynopsis; }
    Q_SIGNAL void includeSceneSynopsisChanged();

    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return true; }

protected:
    bool doExport(QIODevice *device);
    QString fileNameExtension() const { return QStringLiteral("fdx"); }

private:
#ifdef Q_OS_WIN
    bool m_markLanguagesExplicitly = false;
#else
    bool m_markLanguagesExplicitly = true;
#endif
    bool m_includeSceneSynopsis = true;
};

#endif // FINALDRAFTEXPORTER_H
