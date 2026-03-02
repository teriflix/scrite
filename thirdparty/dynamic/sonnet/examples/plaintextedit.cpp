// krazy:excludeall=spelling
/**
 * test_plaintextedit.cpp
 *
 * SPDX-FileCopyrightText: 2015 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
// Local
#include <dictionarycombobox.h>
#include <highlighter.h>
#include <spellcheckdecorator.h>

// Qt
#include <QApplication>
#include <QDebug>
#include <QPlainTextEdit>
#include <QVBoxLayout>

//@@snippet_begin(simple_email_example)
class CommentCheckDecorator : public Sonnet::SpellCheckDecorator
{
public:
    explicit CommentCheckDecorator(QPlainTextEdit *edit)
        : Sonnet::SpellCheckDecorator(edit)
    {
    }

protected:
    bool isSpellCheckingEnabledForBlock(const QString &blockText) const override
    {
        qDebug() << blockText;
        return !blockText.startsWith(QLatin1Char('#'));
    }
};
//@@snippet_end

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget window;

    Sonnet::DictionaryComboBox *comboBox = new Sonnet::DictionaryComboBox;

    //@@snippet_begin(simple_textedit_example)
    QPlainTextEdit *textEdit = new QPlainTextEdit;
    textEdit->setPlainText(
        QString::fromLatin1("This is a sample buffer. Whih this thingg will "
                            "be checkin for misstakes. Whih, Enviroment, govermant. Whih."));

    Sonnet::SpellCheckDecorator *installer = new Sonnet::SpellCheckDecorator(textEdit);
    installer->highlighter()->setCurrentLanguage(QStringLiteral("en"));
    //@@snippet_end

    QObject::connect(comboBox, &Sonnet::DictionaryComboBox::dictionaryChanged, installer->highlighter(), &Sonnet::Highlighter::setCurrentLanguage);

    QPlainTextEdit *commentTextEdit = new QPlainTextEdit;
    commentTextEdit->setPlainText(QStringLiteral("John Doe said:\n# Hello how aree you?\nI am ffine thanks"));

    installer = new CommentCheckDecorator(commentTextEdit);
    installer->highlighter()->setCurrentLanguage(QStringLiteral("en"));
    QObject::connect(comboBox, &Sonnet::DictionaryComboBox::dictionaryChanged, installer->highlighter(), &Sonnet::Highlighter::setCurrentLanguage);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(comboBox);
    layout->addWidget(textEdit);
    layout->addWidget(commentTextEdit);

    window.show();
    return app.exec();
}
