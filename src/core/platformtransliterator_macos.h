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

#ifndef PLATFORMTRANSLITERATOR_MACOS_H
#define PLATFORMTRANSLITERATOR_MACOS_H

#include <QObject>

#include "languageengine.h"

struct MacOSBackendData;
class MacOSBackend : public QObject
{
    Q_OBJECT

public:
    explicit MacOSBackend(QObject *parent = nullptr);
    ~MacOSBackend();

    QList<TransliterationOption> options(int lang,
                                         const PlatformTransliterationEngine *transliterator) const;
    bool canActivate(const TransliterationOption &option,
                     PlatformTransliterationEngine *transliterator);
    bool activate(const TransliterationOption &option,
                  PlatformTransliterationEngine *transliterator);
    bool release(const TransliterationOption &option,
                 PlatformTransliterationEngine *transliterator);

    bool reload();

signals:
    void textInputSourcesChanged();

private:
    MacOSBackendData *d;
};

#endif // PLATFORMTRANSLITERATOR_MACOS_H
