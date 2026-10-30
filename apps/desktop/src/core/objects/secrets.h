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

#ifndef SECRETS_H
#define SECRETS_H

#include <QList>
#include <QObject>
#include <QString>
#include <QQmlEngine>

class QSettings;

/**
 * Keeps secrets, such as API keys, for whoever declares it. Every instance has a name, which
 * is the namespace of its secrets: what one name stores can't be loaded by another by accident.
 *
 * Secrets are kept in the keychain of the operating system whenever it works. Only when the
 * system has no keychain to offer (a Linux session without a secret service, for example) are
 * they kept in the settings of the application instead, and then in an obfuscated form. Users
 * of this class can't choose, but storeType tells them what was chosen, so that they can say so.
 * Values are obfuscated with SimpleCrypt in both places. That keeps a secret from being read
 * at a glance, but it is not strong encryption.
 *
 * Both load() and store() are asynchronous. Calls made before ready is true are held back, and
 * run once it is.
 */
class Secrets : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Secrets(QObject *parent = nullptr);
    ~Secrets() override;

    // The namespace of the secrets. Set it once, when the instance is declared.
    // clang-format off
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    // True once it is known where secrets are kept.
    // clang-format off
    Q_PROPERTY(bool ready
               READ isReady
               NOTIFY readyChanged)
    // clang-format on
    bool isReady() const { return m_ready; }
    Q_SIGNAL void readyChanged();

    enum StoreType { None, Keychain, LocalEncrypted };
    Q_ENUM(StoreType)

    // Where secrets are kept. None until ready is true.
    // clang-format off
    Q_PROPERTY(StoreType storeType
               READ storeType
               NOTIFY storeTypeChanged)
    // clang-format on
    StoreType storeType() const { return m_storeType; }
    Q_SIGNAL void storeTypeChanged();

    // Says what went wrong with the last call, or is empty if it went well.
    // clang-format off
    Q_PROPERTY(QString errorMessage
               READ errorMessage
               NOTIFY errorMessageChanged)
    // clang-format on
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

    // Emits loaded() with the value of the key, which is empty if there is no such key.
    Q_INVOKABLE void load(const QString &key);
    Q_SIGNAL void loaded(const QString &key, const QString &value);

    // Emits stored() with true if the value was kept. An empty value removes the key.
    Q_INVOKABLE void store(const QString &key, const QString &value);
    Q_SIGNAL void stored(const QString &key, bool ok);

private:
    struct PendingCall
    {
        bool isStore = false;
        QString key;
        QString value;
    };

    void probe();
    void setReady(bool val);
    void setStoreType(StoreType val);
    void setErrorMessage(const QString &val);
    void runPendingCalls();

    void run(const PendingCall &call);
    void loadFromKeychain(const QString &key);
    void storeInKeychain(const QString &key, const QString &value);
    void loadFromSettings(const QString &key);
    void storeInSettings(const QString &key, const QString &value);

    QString service() const;
    QString encrypt(const QString &plainText) const;
    QString decrypt(const QString &cipherText) const;
    QSettings *settings() const;

private:
    bool m_ready = false;
    StoreType m_storeType = None;
    QString m_name;
    QString m_errorMessage;
    QList<PendingCall> m_pendingCalls;
};

#endif // SECRETS_H
