// krazy:excludeall=spelling
/**
 * test_textedit.cpp
 *
 * SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
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
#include <QTextEdit>
#include <QVBoxLayout>

//@@snippet_begin(simple_email_example)
class MailSpellCheckDecorator : public Sonnet::SpellCheckDecorator
{
public:
    explicit MailSpellCheckDecorator(QTextEdit *edit)
        : Sonnet::SpellCheckDecorator(edit)
    {
    }

protected:
    bool isSpellCheckingEnabledForBlock(const QString &blockText) const override
    {
        qDebug() << blockText;
        return !blockText.startsWith(QLatin1Char('>'));
    }
};
//@@snippet_end

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget window;

    Sonnet::DictionaryComboBox *comboBox = new Sonnet::DictionaryComboBox;

    //@@snippet_begin(simple_textedit_example)
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setText(
        QString::fromLatin1("This is a sample buffer. Whih this thingg will "
                            "be checkin for misstakes. Whih, Enviroment, govermant. Whih."));

    Sonnet::SpellCheckDecorator *installer = new Sonnet::SpellCheckDecorator(textEdit);
    installer->highlighter()->setCurrentLanguage(QStringLiteral("en_US"));
    //@@snippet_end

    QObject::connect(comboBox, &Sonnet::DictionaryComboBox::dictionaryChanged, installer->highlighter(), &Sonnet::Highlighter::setCurrentLanguage);

    QTextEdit *mailTextEdit = new QTextEdit;
    mailTextEdit->setText(QStringLiteral("John Doe said:\n> Hello how aree you?\nI am ffine thanks"));

    installer = new MailSpellCheckDecorator(mailTextEdit);
    installer->highlighter()->setCurrentLanguage(QStringLiteral("en_US"));
    QObject::connect(comboBox, &Sonnet::DictionaryComboBox::dictionaryChanged, installer->highlighter(), &Sonnet::Highlighter::setCurrentLanguage);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(comboBox);
    layout->addWidget(textEdit);
    layout->addWidget(mailTextEdit);

    window.show();
    return app.exec();
}
