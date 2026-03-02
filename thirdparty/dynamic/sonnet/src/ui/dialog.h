/*
 * dialog.h
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2009-2010 Michel Ludwig <michel.ludwig@kdemail.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_DIALOG_H
#define SONNET_DIALOG_H

#include "sonnetui_export.h"
#include <QDialog>

#include <memory>

class QModelIndex;

namespace Sonnet
{
class BackgroundChecker;
class DialogPrivate;
/*!
 * \class Sonnet::Dialog
 * \inheaderfile Sonnet/Dialog
 * \inmodule SonnetUi
 *
 * \brief Spellcheck dialog.
 *
 * \code
 * Sonnet::Dialog dlg = new Sonnet::Dialog(
 *       new Sonnet::BackgroundChecker(this), this);
 * //connect signals
 * ...
 * dlg->setBuffer( someText );
 * dlg->show();
 * \endcode
 *
 * You can change buffer inside a slot connected to done() signal
 * and spellcheck will continue with new data automatically.
 */
class SONNETUI_EXPORT Dialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     */
    Dialog(BackgroundChecker *checker, QWidget *parent);
    ~Dialog() override;

    /*!
     */
    [[nodiscard]] QString originalBuffer() const;
    /*!
     */
    [[nodiscard]] QString buffer() const;

    /*!
     */
    void show();
    /*!
     */
    void activeAutoCorrect(bool _active);

    // Hide warning about done(), which is a slot in QDialog and a signal here.
    using QDialog::done;

    /*!
     * Controls whether an (indefinite) progress dialog is shown when the spell
     * checking takes longer than the given time to complete. By default no
     * progress dialog is shown. If the progress dialog is set to be shown, no
     * time consuming operation (for example, showing a notification message) should
     * be performed in a slot connected to the 'done' signal as this might trigger
     * the progress dialog unnecessarily.
     *
     * \a timeout time after which the progress dialog should appear; a negative
     *                value can be used to hide it
     * \since 4.4
     */
    void showProgressDialog(int timeout = 500);

    /*!
     * Controls whether a message box indicating the completion of the spell checking
     * is shown or not. By default it is not shown.
     *
     * \since 4.4
     */
    void showSpellCheckCompletionMessage(bool b = true);

    /*!
     * Controls whether the spell checking is continued after the replacement of a
     * misspelled word has been performed. By default it is continued.
     *
     * \since 4.4
     */
    void setSpellCheckContinuedAfterReplacement(bool b);

public Q_SLOTS:
    /*!
     */
    void setBuffer(const QString &);

Q_SIGNALS:
    /*!
     * The dialog won't be closed if you setBuffer() in slot connected to this signal
     * Also emitted after stop() signal
     * \since 5.65
     */
    void spellCheckDone(const QString &newBuffer);
    /*!
     */
    void misspelling(const QString &word, int start);
    /*!
     */
    void replace(const QString &oldWord, int start, const QString &newWord);

    /*!
     */
    void stop();
    /*!
     */
    void cancel();
    /*!
     */
    void autoCorrect(const QString &currentWord, const QString &replaceWord);

    /*!
     * Signal sends when spell checking is finished/stopped/completed
     * \since 4.1
     */
    void spellCheckStatus(const QString &);

    /*!
     * Emitted when the user changes the language used for spellchecking,
     * which is shown in a combobox of this dialog.
     *
     * \a dictionary the new language the user selected
     * \since 4.1
     */
    void languageChanged(const QString &language);

private Q_SLOTS:
    SONNETUI_NO_EXPORT void slotMisspelling(const QString &word, int start);

    SONNETUI_NO_EXPORT void slotDone();

    SONNETUI_NO_EXPORT void slotFinished();

    SONNETUI_NO_EXPORT void slotCancel();

    SONNETUI_NO_EXPORT void slotAddWord();

    SONNETUI_NO_EXPORT void slotReplaceWord();

    SONNETUI_NO_EXPORT void slotReplaceAll();

    SONNETUI_NO_EXPORT void slotSkip();

    SONNETUI_NO_EXPORT void slotSkipAll();

    SONNETUI_NO_EXPORT void slotSuggest();

    SONNETUI_NO_EXPORT void slotChangeLanguage(const QString &);

    SONNETUI_NO_EXPORT void slotSelectionChanged(const QModelIndex &);

    SONNETUI_NO_EXPORT void slotAutocorrect();

    SONNETUI_NO_EXPORT void setGuiEnabled(bool b);

    SONNETUI_NO_EXPORT void setProgressDialogVisible(bool b);

private:
    SONNETUI_NO_EXPORT void updateDialog(const QString &word);

    SONNETUI_NO_EXPORT void fillDictionaryComboBox();

    SONNETUI_NO_EXPORT void updateDictionaryComboBox();

    SONNETUI_NO_EXPORT void fillSuggestions(const QStringList &suggs);

    SONNETUI_NO_EXPORT void initConnections();

    SONNETUI_NO_EXPORT void initGui();

    SONNETUI_NO_EXPORT void continueChecking();

private:
    DialogPrivate *const d;
    Q_DISABLE_COPY(Dialog)
};
}

#endif
