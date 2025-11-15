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

#ifndef ABSTRACTEXPORTER_H
#define ABSTRACTEXPORTER_H

#include "utils.h"
#include "abstractdeviceio.h"
#include "garbagecollector.h"

class AbstractExporter : public AbstractDeviceIO
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~AbstractExporter();
    Q_SIGNAL void aboutToDelete(AbstractExporter *ptr);

    // clang-format off
    Q_PROPERTY(QString format
               READ format
               CONSTANT )
    // clang-format on
    QString format() const;

    // clang-format off
    Q_PROPERTY(QString formatName
               READ formatName
               CONSTANT )
    // clang-format on
    QString formatName() const;

    // clang-format off
    Q_PROPERTY(QString nameFilters
               READ nameFilters
               CONSTANT )
    // clang-format on
    QString nameFilters() const;

    // clang-format off
    Q_PROPERTY(bool featureEnabled
               READ isFeatureEnabled
               NOTIFY featureEnabledChanged)
    // clang-format on
    bool isFeatureEnabled() const;
    Q_SIGNAL void featureEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool canCopyToClipboard
               READ canCopyToClipboard
               CONSTANT )
    // clang-format on
    virtual bool canCopyToClipboard() const { return false; }

    // clang-format off
    Q_PROPERTY(bool requiresConfiguration
               READ requiresConfiguration
               CONSTANT )
    // clang-format on
    virtual bool requiresConfiguration() const { return false; }

    Q_INVOKABLE bool setConfigurationValue(const QString &name, const QVariant &value);
    Q_INVOKABLE QVariant getConfigurationValue(const QString &name) const;

    Q_INVOKABLE Utils::ObjectConfig configuration() const;

    enum Target { FileTarget, ClipboardTarget };
    Q_ENUM(Target)

    Q_INVOKABLE bool write(Target target = FileTarget);

    Q_INVOKABLE void discard() { GarbageCollector::instance()->add(this); }

protected:
    AbstractExporter(QObject *parent = nullptr);
    virtual bool doExport(QIODevice *device) = 0;
};

#endif // ABSTRACTEXPORTER_H
