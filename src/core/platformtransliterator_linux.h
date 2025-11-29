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
#include <QList>
#include "languageengine.h"

class PlatformTransliterationEngine;
struct LinuxBackendData;

class LinuxBackend : public QObject
{
    Q_OBJECT

public:
    explicit LinuxBackend(QObject *parent = nullptr);
    virtual ~LinuxBackend();

    int defaultLanguage() const;
    int activateDefaultLanguage();

    QList<TransliterationOption> options(int lang,
                                         const PlatformTransliterationEngine *transliterator) const;
    bool canActivate(const TransliterationOption &option);
    bool activate(const TransliterationOption &option);

signals:
    void inputSourcesChanged();
    void defaultLanguageChanged();

private slots:
    void onGlobalEngineChanged(const QString &name);

private:
    void reload();
    void updateCurrentEngine();
    LinuxBackendData *d;
};

#endif // PLATFORMTRANSLITERATOR_LINUX_H
