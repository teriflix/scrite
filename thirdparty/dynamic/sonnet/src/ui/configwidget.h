/*
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_CONFIGWIDGET_H
#define SONNET_CONFIGWIDGET_H

#include "sonnetui_export.h"
#include <QWidget>

#include <memory>

namespace Sonnet
{
class ConfigWidgetPrivate;

/*!
 * \class Sonnet::ConfigWidget
 * \inheaderfile Sonnet/ConfigWidget
 * \inmodule SonnetUi
 *
 * \brief The sonnet ConfigWidget.
 */
class SONNETUI_EXPORT ConfigWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     */
    explicit ConfigWidget(QWidget *parent);
    ~ConfigWidget() override;

    /*!
     */
    [[nodiscard]] bool backgroundCheckingButtonShown() const;

    /*!
     * Sets the language/dictionary that will be selected by default
     * in this config widget.
     * This overrides the setting in the config file.
     *
     * \a language the language which will be selected by default.
     * \since 4.1
     */
    void setLanguage(const QString &language);

    /*!
     * Get the currently selected language for spell checking.  Returns an empty string if
     * Sonnet was built without any spellchecking plugins.
     *
     * Returns the language currently selected in the language combobox
     * \since 4.1
     */
    [[nodiscard]] QString language() const;

public Q_SLOTS:
    /*!
     */
    void save();
    /*!
     */
    void setBackgroundCheckingButtonShown(bool);
    /*!
     */
    void slotDefault();
protected Q_SLOTS:
    /*!
     */
    void slotIgnoreWordRemoved();
    /*!
     */
    void slotIgnoreWordAdded();
private Q_SLOTS:
    SONNETUI_NO_EXPORT void slotUpdateButton(const QString &text);
    SONNETUI_NO_EXPORT void slotSelectionChanged();

Q_SIGNALS:
    /*!
     * Signal sends when config was changed
     * \since 4.1
     */
    void configChanged();

private:
    SONNETUI_NO_EXPORT void setFromGui();

private:
    std::unique_ptr<ConfigWidgetPrivate> const d;
};
}

#endif
