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
#include "undoredo.h"
#include "fountain.h"
#include "screenplay.h"
#include "qobjectserializer.h"

#include <QScopeGuard>

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
    while (it != end) {
        if (!opts.contains(it.key()))
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

        if (const SceneElement *sceneElement = qobject_cast<const SceneElement *>(object)) {
            QString ret;
            if (Fountain::encodeEmphasis(sceneElement->text(), sceneElement->textFormats(), ret))
                return ret;
            return sceneElement->text();
        }

        if (const ScreenplayElement *screenplayElement =
                    qobject_cast<const ScreenplayElement *>(object))
            return Fountain::Writer(screenplayElement, Fountain::Writer::NoOption).toString();

        if (const Screenplay *screenplay = qobject_cast<const Screenplay *>(object))
            return Fountain::Writer(screenplay, Fountain::Writer::NoOption).toString();
    }

    return QObjectSerializer::toJsonString(object);
}

bool QObjectTextSerializer::fromText(QObject *object, const QString &text,
                                     const QVariantMap &options)
{
    if (object == nullptr)
        return false;

    const QVariantMap defaultOpts = queryDefaultOptions(object);
    QVariantMap opts = options;
    auto it = defaultOpts.begin();
    auto end = defaultOpts.end();
    while (it != end) {
        if (!opts.contains(it.key()))
            opts.insert(it.key(), it.value());
        ++it;
    }

    QObjectTextSerializer::Interface *iface =
            qobject_cast<QObjectTextSerializer::Interface *>(object);
    if (iface != nullptr)
        return iface->serializeFromText(text, opts);

    if (opts.value(preferredFormatKey).toString() == preferredFormatTextValue) {
        if (qobject_cast<Screenplay *>(object))
            return false;

        const QString macro = QStringLiteral("Serializing from text");
        if (SceneElement *sceneElement = qobject_cast<SceneElement *>(object)) {
            UndoHub::beginMacro(macro);
            auto guard = qScopeGuard([]() { UndoHub::endMacro(); });

            QString plainText;
            QVector<QTextLayout::FormatRange> formats;
            if (!Fountain::resolveEmphasis(text, plainText, formats)) {
                plainText = text;
                formats.clear();
            }

            sceneElement->setText(plainText);
            sceneElement->setTextFormats(formats);
            return true;
        }

        const Fountain::Body body = Fountain::Parser(text).body();

        if (Scene *scene = qobject_cast<Scene *>(object)) {
            UndoHub::beginMacro(macro);
            auto guard = qScopeGuard([]() { UndoHub::endMacro(); });

            scene->clearElements();
            scene->setSynopsis(QString());
            Fountain::loadIntoScene(body, scene);
            return true;
        }

        if (ScreenplayElement *screenplayElement = qobject_cast<ScreenplayElement *>(object)) {
            Scene *scene = screenplayElement->scene();
            if (scene == nullptr)
                return false;

            UndoHub::beginMacro(macro);
            auto guard = qScopeGuard([]() { UndoHub::endMacro(); });

            scene->clearElements();
            scene->setSynopsis(QString());
            Fountain::loadIntoScene(body, scene, screenplayElement);
            return true;
        }
    }

    return QObjectSerializer::fromJsonString(text, object);
}
