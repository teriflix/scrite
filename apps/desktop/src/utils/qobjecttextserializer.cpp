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

#include "qobjecttextserializer.h"
#include "utils.h"

#include "scene.h"
#include "fountain.h"
#include "screenplay.h"
#include "qobjectserializer.h"

static const QString preferredFormatKey = QStringLiteral("preferredFormat");
static const QString preferredFormatTextValue = QStringLiteral("text");

QObjectTextSerializer::Interface::~Interface() { }

QObjectTextSerializer::QObjectTextSerializer(QObject *parent) : QObject(parent)
{
    Utils::ObjectRegistry::add(
            this, QString::fromLatin1(QObjectTextSerializer::staticMetaObject.className()));
}

QObjectTextSerializer::~QObjectTextSerializer()
{
    Utils::ObjectRegistry::remove(this);
}

QObjectTextSerializer *QObjectTextSerializer::instance()
{
    static QObjectTextSerializer *theInstance = new QObjectTextSerializer(qApp);
    return theInstance;
}

QObjectTextSerializer *QObjectTextSerializer::create(QQmlEngine *, QJSEngine *)
{
    QObjectTextSerializer *ts = QObjectTextSerializer::instance();
    QJSEngine::setObjectOwnership(ts, QJSEngine::CppOwnership);
    return ts;
}

QVariantMap QObjectTextSerializer::queryDefaultOptions(const QObject *object)
{
    if (object == nullptr)
        return QVariantMap();

    const QObjectTextSerializer::Interface *iface =
            qobject_cast<const QObjectTextSerializer::Interface *>(object);
    if (iface != nullptr)
        return iface->defaultSerializeToTextOptions();

    return QVariantMap { { preferredFormatKey, preferredFormatTextValue } };
}

QString QObjectTextSerializer::toText(const QObject *object, const QVariantMap &options)
{
    if (object == nullptr)
        return QString();

    const QVariantMap defaultOpts = queryDefaultOptions(object);
    QVariantMap opts = options;
    auto it = defaultOpts.begin();
    auto end = defaultOpts.end();
    while(it != end) {
        if(!opts.contains(it.key()))
            opts.insert(it.key(), it.value());
        ++it;
    }

    const QObjectTextSerializer::Interface *iface =
            qobject_cast<const QObjectTextSerializer::Interface *>(object);
    if (iface != nullptr)
        return iface->serializeToText(opts);

    if (opts.value(preferredFormatKey).toString() == preferredFormatTextValue) {
        if (const Scene *scene = qobject_cast<const Scene *>(object))
            return Fountain::Writer(scene, nullptr, Fountain::Writer::NoOption).toString();

        if (const ScreenplayElement *screenplayElement =
                    qobject_cast<const ScreenplayElement *>(object))
            return Fountain::Writer(screenplayElement, Fountain::Writer::NoOption).toString();

        if (const Screenplay *screenplay = qobject_cast<const Screenplay *>(object))
            return Fountain::Writer(screenplay, Fountain::Writer::NoOption).toString();
    }

    return QObjectSerializer::toJsonString(object);
}
