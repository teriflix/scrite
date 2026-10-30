/****************************************************************************
**
** Copyright (C) 2026 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "secrets.h"
#include "simplecrypt.h"

#include <QTimer>
#include <QSettings>
#include <QCoreApplication>

#include <qtkeychain/keychain.h>

#include "restapikey/restapikey.h"

// Key used with SimpleCrypt to obfuscate secrets, wherever they are kept
#define SECRETS_CRYPT_KEY REST_CRYPT_KEY

// The keychain entry that probe() reads, to find out if there is a keychain at all
#define SECRETS_PROBE_SERVICE "Scrite"
#define SECRETS_PROBE_KEY "probe"

Secrets::Secrets(QObject *parent) : QObject(parent)
{
    // The name is set after construction, but before the event loop gets a turn
    QTimer::singleShot(0, this, &Secrets::probe);
}

Secrets::~Secrets() { }

void Secrets::setName(const QString &val)
{
    if (m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void Secrets::load(const QString &key)
{
    PendingCall call;
    call.isStore = false;
    call.key = key;
    this->run(call);
}

void Secrets::store(const QString &key, const QString &value)
{
    PendingCall call;
    call.isStore = true;
    call.key = key;
    call.value = value;
    this->run(call);
}

void Secrets::probe()
{
    // Jobs have no parent, and delete themselves when done. That way a job that was started by an
    // instance which goes away right after, as a settings page does, still runs to the end.
    QKeychain::ReadPasswordJob *job =
            new QKeychain::ReadPasswordJob(QStringLiteral(SECRETS_PROBE_SERVICE));
    job->setKey(QStringLiteral(SECRETS_PROBE_KEY));

    // Not finding the entry is fine. It means that there is a keychain to look in.
    connect(job, &QKeychain::Job::finished, this, [this, job]() {
        const bool noKeychain = job->error() == QKeychain::NoBackendAvailable
                || job->error() == QKeychain::NotImplemented;
        this->setStoreType(noKeychain ? LocalEncrypted : Keychain);
        this->setReady(true);
        this->runPendingCalls();
    });

    job->start();
}

void Secrets::setReady(bool val)
{
    if (m_ready == val)
        return;

    m_ready = val;
    emit readyChanged();
}

void Secrets::setStoreType(StoreType val)
{
    if (m_storeType == val)
        return;

    m_storeType = val;
    emit storeTypeChanged();
}

void Secrets::setErrorMessage(const QString &val)
{
    if (m_errorMessage == val)
        return;

    m_errorMessage = val;
    emit errorMessageChanged();
}

void Secrets::runPendingCalls()
{
    const QList<PendingCall> calls = m_pendingCalls;
    m_pendingCalls.clear();

    for (const PendingCall &call : calls)
        this->run(call);
}

void Secrets::run(const PendingCall &call)
{
    if (!m_ready) {
        m_pendingCalls.append(call);
        return;
    }

    if (m_name.isEmpty()) {
        this->setErrorMessage(QStringLiteral("Secrets needs a name before it can keep anything."));
        if (call.isStore)
            emit stored(call.key, false);
        else
            emit loaded(call.key, QString());
        return;
    }

    if (m_storeType == Keychain) {
        if (call.isStore)
            this->storeInKeychain(call.key, call.value);
        else
            this->loadFromKeychain(call.key);
    } else {
        if (call.isStore)
            this->storeInSettings(call.key, call.value);
        else
            this->loadFromSettings(call.key);
    }
}

void Secrets::loadFromKeychain(const QString &key)
{
    QKeychain::ReadPasswordJob *job = new QKeychain::ReadPasswordJob(this->service());
    job->setKey(key);

    connect(job, &QKeychain::Job::finished, this, [this, job, key]() {
        // An entry that was never stored is not an error
        if (job->error() == QKeychain::NoError) {
            this->setErrorMessage(QString());
            emit loaded(key, this->decrypt(job->textData()));
        } else if (job->error() == QKeychain::EntryNotFound) {
            this->setErrorMessage(QString());
            emit loaded(key, QString());
        } else {
            this->setErrorMessage(job->errorString());
            emit loaded(key, QString());
        }
    });

    job->start();
}

void Secrets::storeInKeychain(const QString &key, const QString &value)
{
    QKeychain::Job *job = nullptr;
    if (value.isEmpty()) {
        job = new QKeychain::DeletePasswordJob(this->service());
    } else {
        QKeychain::WritePasswordJob *writeJob = new QKeychain::WritePasswordJob(this->service());
        writeJob->setTextData(this->encrypt(value));
        job = writeJob;
    }
    job->setKey(key);

    connect(job, &QKeychain::Job::finished, this, [this, job, key, value]() {
        // Removing an entry that was never stored is not an error
        const bool ok = job->error() == QKeychain::NoError
                || (value.isEmpty() && job->error() == QKeychain::EntryNotFound);
        this->setErrorMessage(ok ? QString() : job->errorString());
        emit stored(key, ok);
    });

    job->start();
}

void Secrets::loadFromSettings(const QString &key)
{
    QSettings *settings = this->settings();
    const QString cipherText = settings
            ? settings->value(this->service() + QLatin1Char('/') + key).toString()
            : QString();

    QTimer::singleShot(0, this, [this, key, cipherText]() {
        this->setErrorMessage(QString());
        emit loaded(key, this->decrypt(cipherText));
    });
}

void Secrets::storeInSettings(const QString &key, const QString &value)
{
    QSettings *settings = this->settings();
    if (settings != nullptr) {
        const QString path = this->service() + QLatin1Char('/') + key;
        if (value.isEmpty())
            settings->remove(path);
        else
            settings->setValue(path, this->encrypt(value));
    }

    QTimer::singleShot(0, this, [this, key, settings]() {
        this->setErrorMessage(settings ? QString()
                                       : QStringLiteral("The settings of the application "
                                                        "are not available."));
        emit stored(key, settings != nullptr);
    });
}

QString Secrets::service() const
{
    return QStringLiteral("Scrite/") + m_name;
}

QString Secrets::encrypt(const QString &plainText) const
{
    if (plainText.isEmpty())
        return QString();

    SimpleCrypt sc(SECRETS_CRYPT_KEY);
    return sc.encryptToString(plainText);
}

QString Secrets::decrypt(const QString &cipherText) const
{
    if (cipherText.isEmpty())
        return QString();

    SimpleCrypt sc(SECRETS_CRYPT_KEY);
    const QString ret = sc.decryptToString(cipherText);
    return sc.lastError() == SimpleCrypt::ErrorNoError ? ret : QString();
}

QSettings *Secrets::settings() const
{
    return qobject_cast<QSettings *>(qApp->property("settings").value<QObject *>());
}
