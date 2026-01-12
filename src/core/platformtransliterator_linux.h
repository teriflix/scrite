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

#ifndef PLATFORMTRANSLITERATOR_LINUX_H
#define PLATFORMTRANSLITERATOR_LINUX_H

#include <QObject>

#include "languageengine.h"

struct LinuxIBusBackendData;
class LinuxIBusBackend : public QObject
{
    Q_OBJECT

public:
    LinuxIBusBackend(QObject *parent = nullptr);
    ~LinuxIBusBackend();

    int defaultLanguage() const;
    int activateDefaultLanguage() const;

    int activeLanguage() const;

    QList<TransliterationOption> options(int lang,
                                         const PlatformTransliterationEngine *transliterator) const;
    bool canActivate(const TransliterationOption &option,
                     PlatformTransliterationEngine *transliterator);
    bool activate(const TransliterationOption &option,
                  PlatformTransliterationEngine *transliterator);
    bool release(const TransliterationOption &option,
                 PlatformTransliterationEngine *transliterator);

    bool eventFilter(QObject *object, QEvent *event);

signals:
    void activeEnginesChanged();
    void activeLangugeChanged();

private:
    bool reload();

private:
    LinuxIBusBackendData *d;
};

#endif // PLATFORMTRANSLITERATOR_LINUX_H
