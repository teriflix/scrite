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

#ifndef FINALDRAFTEXPORTER_H
#define FINALDRAFTEXPORTER_H

#include "abstractexporter.h"

class FinalDraftExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Final Draft")
    Q_CLASSINFO("NameFilters", "Final Draft (*.fdx)")

public:
    Q_INVOKABLE explicit FinalDraftExporter(QObject *parent = nullptr);
    ~FinalDraftExporter();

    Q_CLASSINFO("markLanguagesExplicitly_FieldLabel", "Explicity mark text-fragments of different languages.")
    Q_CLASSINFO("markLanguagesExplicitly_FieldEditor", "CheckBox")
    Q_PROPERTY(bool markLanguagesExplicitly READ isMarkLanguagesExplicitly WRITE setMarkLanguagesExplicitly NOTIFY markLanguagesExplicitlyChanged)
    void setMarkLanguagesExplicitly(bool val);
    bool isMarkLanguagesExplicitly() const { return m_markLanguagesExplicitly; }
    Q_SIGNAL void markLanguagesExplicitlyChanged();

    Q_CLASSINFO("useScriteFonts_FieldLabel", "Use language fonts as specified in Settings > Fonts panel.")
    Q_CLASSINFO("useScriteFonts_FieldNote", "Please make sure that the font selected in Settings > Fonts panel is instaled in your computer and is available for all users.")
    Q_CLASSINFO("useScriteFonts_FieldEditor", "CheckBox")
    Q_PROPERTY(bool useScriteFonts READ isUseScriteFonts WRITE setUseScriteFonts NOTIFY useScriteFontsChanged)
    void setUseScriteFonts(bool val);
    bool isUseScriteFonts() const { return m_useScriteFonts; }
    Q_SIGNAL void useScriteFontsChanged();

    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return true; }

protected:
    bool doExport(QIODevice *device);
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface

private:
#ifdef Q_OS_WIN
    bool m_markLanguagesExplicitly = false;
#else
    bool m_markLanguagesExplicitly = true;
#endif
    bool m_useScriteFonts = false;
};

#endif // FINALDRAFTEXPORTER_H
