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

#include "platformtransliterator_linux.h"
#include "languageengine.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

Q_GLOBAL_STATIC(LinuxBackend, Backend)

namespace {
const QString IBUS_SERVICE = QStringLiteral("org.freedesktop.IBus");
const QString IBUS_PATH = QStringLiteral("/org/freedesktop/IBus");
const QString IBUS_INTERFACE = QStringLiteral("org.freedesktop.IBus");

struct IBusEngineDesc
{
    QString name;
    QString longname;
    QString description;
    QString language;
    QString license;
    QString author;
    QString icon;
    QString layout;

    bool operator==(const IBusEngineDesc &other) const { return this->name == other.name; }
};

// Custom QDBusArgument operators for IBusEngineDesc
const QDBusArgument &operator>>(const QDBusArgument &argument, IBusEngineDesc &desc)
{
    argument.beginStructure();
    argument >> desc.name >> desc.longname >> desc.description >> desc.language >> desc.license
            >> desc.author >> desc.icon >> desc.layout;
    argument.endStructure();
    return argument;
}
}

Q_DECLARE_METATYPE(IBusEngineDesc)
Q_DECLARE_METATYPE(QList<IBusEngineDesc>)

struct LinuxBackendData
{
    QList<IBusEngineDesc> activeEngines;
    int defaultLanguageCode = QLocale::English;
};

///////////////////////////////////////////////////////////////////////////////
// PlatformTransliterationEngine Implementation
///////////////////////////////////////////////////////////////////////////////

PlatformTransliterationEngine::PlatformTransliterationEngine(QObject *parent)
    : AbstractTransliterationEngine(parent)
{
    connect(::Backend, &LinuxBackend::inputSourcesChanged, this,
            &PlatformTransliterationEngine::capacityChanged);
    connect(::Backend, &LinuxBackend::defaultLanguageChanged, this,
            &PlatformTransliterationEngine::defaultLanguageChanged);
}

PlatformTransliterationEngine::~PlatformTransliterationEngine() { }

QString PlatformTransliterationEngine::name() const
{
    return QStringLiteral("Linux (IBus)");
}

int PlatformTransliterationEngine::defaultLanguage() const
{
    return ::Backend->defaultLanguage();
}

int PlatformTransliterationEngine::activateDefaultLanguage()
{
    return ::Backend->activateDefaultLanguage();
}

QList<TransliterationOption> PlatformTransliterationEngine::options(int lang) const
{
    return ::Backend->options(lang, this);
}

bool PlatformTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return option.transliteratorObject == this && ::Backend->canActivate(option);
}

bool PlatformTransliterationEngine::activate(const TransliterationOption &option)
{
    return ::Backend->activate(option);
}

QString PlatformTransliterationEngine::transliterateWord(const QString &word,
                                                         const TransliterationOption &option) const
{
    Q_UNUSED(option);
    return word;
}

///////////////////////////////////////////////////////////////////////////////
// LinuxBackend Implementation
///////////////////////////////////////////////////////////////////////////////

LinuxBackend::LinuxBackend(QObject *parent) : QObject(parent), d(new LinuxBackendData)
{
    qRegisterMetaType<IBusEngineDesc>();
    qRegisterMetaType<QList<IBusEngineDesc>>();

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qWarning() << "Cannot connect to the D-Bus session bus.";
        return;
    }

    QDBusReply<bool> reply = bus.interface()->isServiceRegistered(IBUS_SERVICE);
    if (!reply.isValid() || !reply.value()) {
        qWarning() << "IBus service not available on the session bus.";
        return;
    }

    // Connect to the signal that indicates a change in the global input engine
    bus.connect(IBUS_SERVICE, IBUS_PATH, IBUS_INTERFACE, "global-engine-changed", this,
                SLOT(onGlobalEngineChanged(QString)));

    reload();
}

LinuxBackend::~LinuxBackend()
{
    delete d;
}

void LinuxBackend::reload()
{
    QDBusInterface bus_iface(IBUS_SERVICE, IBUS_PATH, IBUS_INTERFACE,
                             QDBusConnection::sessionBus());
    if (!bus_iface.isValid()) {
        qWarning() << "Failed to create IBus interface:" << bus_iface.lastError().message();
        return;
    }

    QDBusReply<QList<IBusEngineDesc>> reply = bus_iface.call("ListActiveEngines");

    if (!reply.isValid()) {
        qWarning() << "Failed to call ListActiveEngines:" << reply.error().message();
        return;
    }

    const auto newEngines = reply.value();
    if (d->activeEngines != newEngines) {
        d->activeEngines = newEngines;
        emit inputSourcesChanged();
    }

    updateCurrentEngine();
}

void LinuxBackend::updateCurrentEngine()
{
    auto setDefaultToEnglish = [this]() {
        if (d->defaultLanguageCode != QLocale::English) {
            d->defaultLanguageCode = QLocale::English;
            emit defaultLanguageChanged();
        }
    };

    QDBusInterface bus_iface(IBUS_SERVICE, IBUS_PATH, IBUS_INTERFACE,
                             QDBusConnection::sessionBus());
    if (!bus_iface.isValid()) {
        qWarning() << "Failed to create IBus interface, falling back to English:"
                   << bus_iface.lastError().message();
        setDefaultToEnglish();
        return;
    }

    QDBusReply<IBusEngineDesc> reply = bus_iface.call("GetGlobalEngine");

    if (!reply.isValid()) {
        qWarning() << "Failed to call GetGlobalEngine, falling back to English:"
                   << reply.error().message();
        setDefaultToEnglish();
        return;
    }

    const IBusEngineDesc &currentEngine = reply.value();
    QString langCode = currentEngine.language.left(2);
    int newLang = langCode.isEmpty() ? QLocale::English : QLocale(langCode).language();

    if (d->defaultLanguageCode != newLang) {
        d->defaultLanguageCode = newLang;
        emit defaultLanguageChanged();
    }
}

void LinuxBackend::onGlobalEngineChanged(const QString &name)
{
    Q_UNUSED(name);
    // The global engine has changed, so we need to update our cached default language.
    updateCurrentEngine();
}

int LinuxBackend::defaultLanguage() const
{
    return d->defaultLanguageCode;
}

int LinuxBackend::activateDefaultLanguage()
{
    int langCode = d->defaultLanguageCode >= 0 ? d->defaultLanguageCode : QLocale::English;
    auto defaultOptions = options(langCode, nullptr);
    if (!defaultOptions.isEmpty()) {
        // Attempt to activate the first available option for the default language.
        if (activate(defaultOptions.first())) {
            return langCode;
        }
    }
    return -1;
}

QList<TransliterationOption>
LinuxBackend::options(int lang, const PlatformTransliterationEngine *transliterator) const
{
    QList<TransliterationOption> engineOptions;
    QLocale locale(static_cast<QLocale::Language>(lang));
    QString langCode = locale.bcp47Name().left(2);

    for (const IBusEngineDesc &desc : qAsConst(d->activeEngines)) {
        if (desc.language.startsWith(langCode)) {
            engineOptions << TransliterationOption(
                    { (QObject *)transliterator, lang, desc.name, desc.longname, false });
        }
    }
    return engineOptions;
}

bool LinuxBackend::canActivate(const TransliterationOption &option)
{
    for (const auto &engine : qAsConst(d->activeEngines)) {
        if (engine.name == option.id) {
            return true;
        }
    }
    return false;
}

bool LinuxBackend::activate(const TransliterationOption &option)
{
    if (option.id.isEmpty()) {
        return false;
    }

    QDBusInterface bus_iface(IBUS_SERVICE, IBUS_PATH, IBUS_INTERFACE,
                             QDBusConnection::sessionBus());
    if (!bus_iface.isValid()) {
        qWarning() << "Failed to create IBus interface:" << bus_iface.lastError().message();
        return false;
    }

    QDBusReply<void> reply = bus_iface.call("SetGlobalEngine", option.id);

    if (!reply.isValid()) {
        qWarning() << "Failed to call SetGlobalEngine:" << reply.error().message();
        return false;
    }

    return true;
}
