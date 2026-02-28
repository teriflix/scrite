/*
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_CONFIGVIEW_H
#define SONNET_CONFIGVIEW_H

#include <QWidget>

#include "sonnetui_export.h"

#include <memory>

namespace Sonnet
{
class ConfigViewPrivate;

/*!
 * \class Sonnet::ConfigView
 * \inheaderfile Sonnet/ConfigView
 * \inmodule SonnetUi
 *
 * \brief The sonnet ConfigView.
 */
class SONNETUI_EXPORT ConfigView : public QWidget
{
    Q_OBJECT

    /*!
     * \property Sonnet::ConfigView::language
     */
    Q_PROPERTY(QString language READ language WRITE setLanguage)

    /*!
     * \property Sonnet::ConfigView::ignoreList
     */
    Q_PROPERTY(QStringList ignoreList READ ignoreList WRITE setIgnoreList)

    /*!
     * \property Sonnet::ConfigView::preferredLanguages
     */
    Q_PROPERTY(QStringList preferredLanguages READ preferredLanguages WRITE setPreferredLanguages)

    /*!
     * \property Sonnet::ConfigView::backgroundCheckingButtonShown
     */
    Q_PROPERTY(bool backgroundCheckingButtonShown READ backgroundCheckingButtonShown WRITE setBackgroundCheckingButtonShown)

    /*!
     * \property Sonnet::ConfigView::showNoBackendFound
     */
    Q_PROPERTY(bool showNoBackendFound READ noBackendFoundVisible WRITE setNoBackendFoundVisible)
public:
    /*!
     */
    explicit ConfigView(QWidget *parent = nullptr);
    ~ConfigView() override;

    /*!
     */
    [[nodiscard]] bool backgroundCheckingButtonShown() const;
    /*!
     */
    [[nodiscard]] bool noBackendFoundVisible() const;
    /*!
     */
    [[nodiscard]] QStringList preferredLanguages() const;
    /*!
     */
    [[nodiscard]] QString language() const;
    /*!
     */
    [[nodiscard]] QStringList ignoreList() const;

public Q_SLOTS:
    /*!
     */
    void setNoBackendFoundVisible(bool show);
    /*!
     */
    void setBackgroundCheckingButtonShown(bool);
    /*!
     */
    void setPreferredLanguages(const QStringList &ignoreList);
    /*!
     */
    void setLanguage(const QString &language);
    /*!
     */
    void setIgnoreList(const QStringList &ignoreList);

Q_SIGNALS:
    /*!
     */
    void configChanged();

private:
    std::unique_ptr<ConfigViewPrivate> const d;
};
}

#endif
