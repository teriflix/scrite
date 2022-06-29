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

#include "abstractdeviceio.h"
#include "garbagecollector.h"
#include "transliteration.h"

class AbstractExporter : public AbstractDeviceIO
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~AbstractExporter();
    Q_SIGNAL void aboutToDelete(AbstractExporter *ptr);

    Q_PROPERTY(QString format READ format CONSTANT)
    QString format() const;

    Q_PROPERTY(QString formatName READ formatName CONSTANT)
    QString formatName() const;

    Q_PROPERTY(QString nameFilters READ nameFilters CONSTANT)
    QString nameFilters() const;

    Q_PROPERTY(bool featureEnabled READ isFeatureEnabled NOTIFY featureEnabledChanged)
    bool isFeatureEnabled() const;
    Q_SIGNAL void featureEnabledChanged();

    Q_PROPERTY(bool canBundleFonts READ canBundleFonts CONSTANT)
    virtual bool canBundleFonts() const { return false; }

    Q_INVOKABLE void bundleFontForLanguage(int language, bool on = true)
    {
        m_languageBundleMap[TransliterationEngine::Language(language)] = on;
    }
    Q_INVOKABLE bool isFontForLanguageBundled(int language) const
    {
        return m_languageBundleMap.value(TransliterationEngine::Language(language), false);
    }

    Q_PROPERTY(bool requiresConfiguration READ requiresConfiguration CONSTANT)
    virtual bool requiresConfiguration() const { return false; }

    Q_INVOKABLE bool setConfigurationValue(const QString &name, const QVariant &value)
    {
        return this->setProperty(qPrintable(name), value);
    }
    Q_INVOKABLE QVariant getConfigurationValue(const QString &name) const
    {
        return this->property(qPrintable(name));
    }

    Q_INVOKABLE QJsonObject configurationFormInfo() const;

    Q_INVOKABLE bool write();

    Q_INVOKABLE void discard() { GarbageCollector::instance()->add(this); }

protected:
    AbstractExporter(QObject *parent = nullptr);
    virtual bool doExport(QIODevice *device) = 0;

    QMap<TransliterationEngine::Language, bool> languageBundleMap() const
    {
        return m_languageBundleMap;
    }

private:
    QMap<TransliterationEngine::Language, bool> m_languageBundleMap;
};

#endif // ABSTRACTEXPORTER_H
