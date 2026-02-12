/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <glib.h>
#include <ibus.h>

#include "platformtransliterator_linux.h"
#include "languageengine.h"
#include "utils.h"
#include "timeprofiler.h"

#include <QGuiApplication>
#include <QScopeGuard>

Q_GLOBAL_STATIC(LinuxIBusBackend, Backend)

PlatformTransliterationEngine::PlatformTransliterationEngine(QObject *parent)

    : AbstractTransliterationEngine(parent)
{
    connect(::Backend, &LinuxIBusBackend::activeEnginesChanged, this,
            &PlatformTransliterationEngine::capacityChanged);
    connect(::Backend, &LinuxIBusBackend::activeEnginesChanged, this,
            &PlatformTransliterationEngine::defaultLanguageChanged);
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

int PlatformTransliterationEngine::defaultLanguage() const
{
    return ::Backend->defaultLanguage();
}

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("Linux IBus");
}

int PlatformTransliterationEngine::activateDefaultLanguage()
{
    int code = ::Backend->activateDefaultLanguage();
    if (code >= 0)
        return code;

    const QList<TransliterationOption> systemLanguageOptions =
            this->options(QLocale::system().language());
    if (systemLanguageOptions.isEmpty())
        return -1; // No clue what to do now.

    if (this->activate(systemLanguageOptions.first()))
        return QLocale::English;

    return -1;
}

QList<TransliterationOption> PlatformTransliterationEngine::options(int lang) const
{
    return ::Backend->options(lang, this);
}

bool PlatformTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return option.transliteratorObject == this && ::Backend->canActivate(option, this);
}

bool PlatformTransliterationEngine::activate(const TransliterationOption &option)
{
    return ::Backend->activate(option, this);
}

QString PlatformTransliterationEngine::transliterateWord(const QString &word,
                                                         const TransliterationOption &option) const
{
    // No need to implement this, because platform transliterators don't offer in-app
    // transliterations.
    Q_UNUSED(option);
    return word;
}

///////////////////////////////////////////////////////////////////////////////

void PlatformLanguageObserver::setupObservation()
{
    this->setActiveLanguageCode(::Backend->activeLanguage());

    connect(::Backend, &LinuxIBusBackend::activeLangugeChanged, this,
            [=]() { this->setActiveLanguageCode(::Backend->activeLanguage()); });
}

///////////////////////////////////////////////////////////////////////////////

struct LinuxIBusBackendData
{
    IBusBus *bus = nullptr;
    QList<IBusEngineDesc *> allEngines;

    QStringList engineIds;
    QList<IBusEngineDesc *> engines;

    LinuxIBusBackendData()
    {
        ibus_init();

        bus = ibus_bus_new();
        if (bus) {
            GList *all_engines = ibus_bus_list_engines(bus);
            for (GList *l = all_engines; l != nullptr; l = l->next) {
                IBusEngineDesc *engine = static_cast<IBusEngineDesc *>(l->data);
                if (engine)
                    allEngines.append(engine);
            }
        }
    }
    ~LinuxIBusBackendData()
    {
        while (!allEngines.isEmpty()) {
            g_object_unref(allEngines.takeFirst());
        }
        if (bus)
            g_object_unref(bus);
    }
};

static void global_engine_changed_callback(IBusBus *bus, const gchar *engine_name,
                                           gpointer user_data)
{
    Q_UNUSED(bus)
    Q_UNUSED(engine_name)
    Q_UNUSED(user_data)

    emit ::Backend->activeLangugeChanged();
}

inline QString qstr(const gchar *string)
{
    return QString::fromLatin1(string);
}

inline QString engine_id(IBusEngineDesc *engine)
{
    return ibus_engine_desc_get_name(engine);
}

inline QString engine_name(IBusEngineDesc *engine)
{
    return ibus_engine_desc_get_longname(engine);
}

inline QLocale engine_locale(IBusEngineDesc *engine)
{
    return QLocale(qstr(ibus_engine_desc_get_language(engine)));
}

inline int engine_language(IBusEngineDesc *engine)
{
    return engine_locale(engine).language();
}

LinuxIBusBackend::LinuxIBusBackend(QObject *parent) : QObject(parent)
{
    d = new LinuxIBusBackendData;

    // Connect to the global-engine-changed signal
    if (d->bus) {
        g_signal_connect(d->bus, "global-engine-changed",
                         G_CALLBACK(global_engine_changed_callback), this);
    }

    qApp->installEventFilter(this);
    connect(qApp, &QGuiApplication::aboutToQuit, this, &LinuxIBusBackend::activateDefaultLanguage);

    this->reload();
}

LinuxIBusBackend::~LinuxIBusBackend()
{
    delete d;
}

int LinuxIBusBackend::defaultLanguage() const
{
    return QLocale::system().language();
}

int LinuxIBusBackend::activateDefaultLanguage() const
{
    if (!d->engines.isEmpty()) {
        int lang = QLocale::system().language();
        for (IBusEngineDesc *engine : qAsConst(d->engines)) {
            if (engine_language(engine) == lang) {
                if (ibus_bus_set_global_engine(d->bus, ibus_engine_desc_get_name(engine)))
                    return lang;
                break;
            }
        }
    }

    return -1;
}

int LinuxIBusBackend::activeLanguage() const
{
    if (!d->engines.isEmpty()) {
        IBusEngineDesc *engine = ibus_bus_get_global_engine(d->bus);
        return engine_language(engine);
    }

    return -1;
}

QList<TransliterationOption>
LinuxIBusBackend::options(int lang, const PlatformTransliterationEngine *transliterator) const
{
    QList<TransliterationOption> ret;

    for (IBusEngineDesc *engine : qAsConst(d->engines)) {
        if (engine_language(engine) == lang) {
            ret << TransliterationOption((QObject *)transliterator, lang, engine_id(engine),
                                         engine_name(engine), false);
        }
    }

    return ret;
}

bool LinuxIBusBackend::canActivate(const TransliterationOption &option,
                                   PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(transliterator)

    auto it = std::find_if(d->engines.begin(), d->engines.end(), [option](IBusEngineDesc *engine) {
        return option.id == engine_id(engine);
    });
    return it != d->engines.end();
}

bool LinuxIBusBackend::activate(const TransliterationOption &option,
                                PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(transliterator)

    auto it = std::find_if(d->engines.begin(), d->engines.end(), [option](IBusEngineDesc *engine) {
        return option.id == engine_id(engine);
    });
    if (it != d->engines.end()) {
        IBusEngineDesc *engine = *it;
        return ibus_bus_set_global_engine(d->bus, ibus_engine_desc_get_name(engine));
    }

    return false;
}

bool LinuxIBusBackend::release(const TransliterationOption &option,
                               PlatformTransliterationEngine *transliterator)
{
    Q_UNUSED(option)
    Q_UNUSED(transliterator)
    return true;
}

bool LinuxIBusBackend::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ApplicationStateChange && object == qApp) {
        if (qApp->applicationState() == Qt::ApplicationActive) {
            if (this->reload())
                emit activeEnginesChanged();
        } else
            this->activateDefaultLanguage();
    }

    return false;
}

bool LinuxIBusBackend::reload()
{
    QStringList engineIds;

    IBusConfig *config = ibus_bus_get_config(d->bus);
    if (config != nullptr) {
        auto configCleanup = qScopeGuard([config]() { g_object_unref(config); });

        GVariant *value = ibus_config_get_value(config, "general", "preload-engines");
        if (value != nullptr) {
            auto valueCleanup = qScopeGuard([value]() { g_variant_unref(value); });

            GVariantIter iter;
            const gchar *engine_id = nullptr;
            g_variant_iter_init(&iter, value);

            while (g_variant_iter_next(&iter, "s", &engine_id)) {
                engineIds << QString::fromLatin1(engine_id);
            }
        }
    }

    if (d->engineIds != engineIds) {
        d->engineIds = engineIds;
        d->engines.clear();

        for (const QString &engineName : qAsConst(d->engineIds)) {
            auto it = std::find_if(d->allEngines.begin(), d->allEngines.end(),
                                   [engineName](IBusEngineDesc *engine) {
                                       return engine_id(engine) == engineName;
                                   });
            if (it != d->allEngines.end())
                d->engines.append(*it);
        }

        return true;
    }

    return false;
}
