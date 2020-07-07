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

#ifndef ABSTRACTSCREENPLAYSUBSETREPORT_H
#define ABSTRACTSCREENPLAYSUBSETREPORT_H

#include "screenplaytextdocument.h"
#include "abstractreportgenerator.h"

class AbstractScreenplaySubsetReport : public AbstractReportGenerator,
                                       public AbstractScreenplayTextDocumentInjectionInterface
{
    Q_OBJECT
    Q_INTERFACES(AbstractScreenplayTextDocumentInjectionInterface)

public:
    ~AbstractScreenplaySubsetReport();

    bool requiresConfiguration() const { return true; }

    Q_CLASSINFO("generateTitlePage_FieldLabel", "Generate title page.")
    Q_CLASSINFO("generateTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateTitlePage READ isGenerateTitlePage WRITE setGenerateTitlePage NOTIFY generateTitlePageChanged)
    void setGenerateTitlePage(bool val);
    bool isGenerateTitlePage() const { return m_generateTitlePage; }
    Q_SIGNAL void generateTitlePageChanged();

    Q_CLASSINFO("listSceneCharacters_FieldLabel", "List characters for each scene.")
    Q_CLASSINFO("listSceneCharacters_FieldEditor", "CheckBox")
    Q_PROPERTY(bool listSceneCharacters READ isListSceneCharacters WRITE setListSceneCharacters NOTIFY listSceneCharactersChanged)
    void setListSceneCharacters(bool val);
    bool isListSceneCharacters() const { return m_listSceneCharacters; }
    Q_SIGNAL void listSceneCharactersChanged();

    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated PDF.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    Q_CLASSINFO("includeSceneIcons_FieldLabel", "Include scene icons in the generated PDF.")
    Q_CLASSINFO("includeSceneIcons_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneIcons READ isIncludeSceneIcons WRITE setIncludeSceneIcons NOTIFY includeSceneIconsChanged)
    void setIncludeSceneIcons(bool val);
    bool isIncludeSceneIcons() const { return m_includeSceneIcons; }
    Q_SIGNAL void includeSceneIconsChanged();

    Q_CLASSINFO("printEachSceneOnANewPage_FieldLabel", "Print each scene on a new page.")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool printEachSceneOnANewPage READ isPrintEachSceneOnANewPage WRITE setPrintEachSceneOnANewPage NOTIFY printEachSceneOnANewPageChanged)
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    virtual QString screenplaySubtitle() const { return QStringLiteral("Screenplay Subset"); }
    virtual bool includeScreenplayElement(const ScreenplayElement *) const { return true; }

protected:
    AbstractScreenplaySubsetReport(QObject *parent=nullptr);

    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *);

    // AbstractReportGenerator interface
    void configureTextDocumentPrinter(QTextDocumentPagedPrinter *, const QTextDocument *);

    virtual void configureScreenplayTextDocument(ScreenplayTextDocument &) { }

private:
    bool m_generateTitlePage = true;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_listSceneCharacters = false;
    Screenplay *m_screenplaySubset = nullptr;
    bool m_printEachSceneOnANewPage = false;
};

#endif // ABSTRACTSCREENPLAYSUBSETREPORT_H
