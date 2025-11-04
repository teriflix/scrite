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

#ifndef PLATFORMTRANSLITERATOR_WINDOWS_H
#define PLATFORMTRANSLITERATOR_WINDOWS_H

#include <QObject>

#include "languageengine.h"

struct WindowsBackendData;
class WindowsBackend : public QObject
{
    Q_OBJECT

public:
    explicit WindowsBackend(QObject *parent = nullptr);
    ~WindowsBackend();

    QList<TransliterationOption> options(int lang,
                                         const PlatformTransliterationEngine *transliterator) const;
    bool canActivate(const TransliterationOption &option, PlatformTransliterationEngine *transliterator);
    bool activate(const TransliterationOption &option, PlatformTransliterationEngine *transliterator);
    bool release(const TransliterationOption &option, PlatformTransliterationEngine *transliterator);

    bool eventFilter(QObject *object, QEvent *event);

signals:
    void textInputSourcesChanged();

private:
    bool reload();

private:
    WindowsBackendData *d;
};

#endif // PLATFORMTRANSLITERATOR_WINDOWS_H
