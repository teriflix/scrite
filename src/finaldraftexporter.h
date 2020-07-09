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

#ifndef FINALDRAFTEXPORTER_H
#define FINALDRAFTEXPORTER_H

#include "abstractexporter.h"

class FinalDraftExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Final Draft")
    Q_CLASSINFO("NameFilters", "Final Draft (*.fdx)")

public:
    Q_INVOKABLE FinalDraftExporter(QObject *parent=nullptr);
    ~FinalDraftExporter();

    Q_CLASSINFO("markLanguagesExplicitly_FieldLabel", "Explicity mark text-fragments of different languages.")
    Q_CLASSINFO("markLanguagesExplicitly_FieldEditor", "CheckBox")
    Q_PROPERTY(bool markLanguagesExplicitly READ isMarkLanguagesExplicitly WRITE setMarkLanguagesExplicitly NOTIFY markLanguagesExplicitlyChanged)
    void setMarkLanguagesExplicitly(bool val);
    bool isMarkLanguagesExplicitly() const {return m_markLanguagesExplicitly; }
    Q_SIGNAL void markLanguagesExplicitlyChanged();

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
};

#endif // FINALDRAFTEXPORTER_H
