/*  This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2009 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LANGUAGEFILTER_H
#define LANGUAGEFILTER_H

#include "sonnetcore_export.h"
#include <tokenizer_p.h>

#include <QString>

#include <memory>

namespace Sonnet
{
class LanguageFilterPrivate;

/*!
 * \brief Deternmines language for fragments of text.
 *
 * This class takes fragments produced by supplied tokenizer and provides additional information:
 * language used in each fragment and if there is spell and grammar checker suitable for the fragment.
 *
 * \internal
 */
class SONNETCORE_EXPORT LanguageFilter : public AbstractTokenizer
{
public:
    /*! Creates language filter for given tokenizer. LanguageFilter takes complete ownership of given tokenizer.
    This means that no source's methods should be called anymore.
    */
    LanguageFilter(AbstractTokenizer *source);
    /*!
     */
    LanguageFilter(const LanguageFilter &other);

    ~LanguageFilter() override;

    /*! Language for token last returned by next() */
    QString language() const;

    /*! Returns true if there is spellchecker installed for last token's language  */
    bool isSpellcheckable() const;

    void setBuffer(const QString &buffer) override;
    bool hasNext() const override;
    Token next() override;
    QString buffer() const override;
    void replace(int position, int len, const QString &newWord) override;

private:
    std::unique_ptr<LanguageFilterPrivate> const d;
};
}
#endif
