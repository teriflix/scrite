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

#ifndef ABSTRACTTEXTDOCUMENTEXPORTER_H
#define ABSTRACTTEXTDOCUMENTEXPORTER_H

#include "abstractexporter.h"

class AbstractTextDocumentExporter : public AbstractExporter
{
    Q_OBJECT

public:
    ~AbstractTextDocumentExporter();

    Q_CLASSINFO("listSceneCharacters_FieldLabel", "List characters for each scene")
    Q_CLASSINFO("listSceneCharacters_FieldEditor", "CheckBox")
    Q_PROPERTY(bool listSceneCharacters READ isListSceneCharacters WRITE setListSceneCharacters NOTIFY listSceneCharactersChanged)
    void setListSceneCharacters(bool val);
    bool isListSceneCharacters() const { return m_listSceneCharacters; }
    Q_SIGNAL void listSceneCharactersChanged();

    virtual bool usePageBreaks() const { return false; }
    virtual bool isIncludeSceneNumbers() const { return false; }

    bool requiresConfiguration() const { return true; }

protected:
    AbstractTextDocumentExporter(QObject *parent=nullptr);
    void generate(QTextDocument *textDocument, const qreal pageWidth);

private:
    bool m_listSceneCharacters = false;
};

#endif // ABSTRACTTEXTDOCUMENTEXPORTER_H
