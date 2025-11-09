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

#include "languageengine.h"

/**
 * I'd love some contribution to this. I can't seem to find standard APIs on Linux
 * Desktop OSes to query and switch input methods for various languages. It would be
 * awesome if someone can build this interface for Scrite!
 */

PlatformTransliterationEngine::PlatformTransliterationEngine(QObject *parent)
    : AbstractTransliterationEngine(parent)
{
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("Linux");
}

int PlatformTransliterationEngine::activateDefaultLanguage()
{
    return QLocale::English;
}

QList<TransliterationOption> PlatformTransliterationEngine::options(int lang) const
{
    Q_UNUSED(lang)
    return {};
}

bool PlatformTransliterationEngine::canActivate(const TransliterationOption &option)
{
    Q_UNUSED(option)
    return false;
}

bool PlatformTransliterationEngine::activate(const TransliterationOption &option)
{
    Q_UNUSED(option)
    return false;
}

QString PlatformTransliterationEngine::transliterateWord(const QString &word,
                                                         const TransliterationOption &option) const
{
    // No need to implement this, because platform transliterators don't offer in-app
    // transliterations.
    Q_UNUSED(option);
    return word;
}
