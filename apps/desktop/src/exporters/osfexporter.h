/****************************************************************************
**
** Copyright (C) 2024 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OSFEXPORTER_H
#define OSFEXPORTER_H

#include "abstractexporter.h"

class OsfExporter : public AbstractExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Screenplay/Open Screenplay Format")
    Q_CLASSINFO("NameFilters", "Open Screenplay Format (*.xml)")
    Q_CLASSINFO("Description", "Exports the current screenplay to Open Screenplay Format (OSF) file format.")
    Q_CLASSINFO("Icon", ":/icons/exporter/osf.png")
    // clang-format on

public:
    Q_INVOKABLE explicit OsfExporter(QObject *parent = nullptr);
    ~OsfExporter();

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

    // clang-format off
    Q_CLASSINFO("includeSceneNotes_FieldLabel", "Include scene notes.")
    Q_CLASSINFO("includeSceneNotes_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNotes
               READ isIncludeSceneNotes
               WRITE setIncludeSceneNotes
               NOTIFY includeSceneNotesChanged)
    // clang-format on
    void setIncludeSceneNotes(bool val);
    bool isIncludeSceneNotes() const { return m_includeSceneNotes; }
    Q_SIGNAL void includeSceneNotesChanged();

    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return true; }

protected:
    bool doExport(QIODevice *device);
    QString fileNameExtension() const { return QStringLiteral("xml"); }

private:
    bool m_includeSceneSynopsis = true;
    bool m_includeSceneNotes = true;
};

#endif // OSFEXPORTER_H
