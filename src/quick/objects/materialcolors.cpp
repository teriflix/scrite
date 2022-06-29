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

#include "materialcolors.h"

#include <QFile>
#include <QJSEngine>
#include <QJsonDocument>
#include <QVariant>

class MaterialColorsDb
{
public:
    MaterialColorsDb();
    ~MaterialColorsDb();

    QJsonObject data() const { return m_data; }
    QJsonObject data(const QString &name) const { return m_data.value(name.toLower()).toObject(); }

private:
    QJsonObject m_data;
};

MaterialColorsDb::MaterialColorsDb()
{
    QFile file(":/misc/material_colors_db.json");
    if (file.open(QFile::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        m_data = doc.object();
    }
}

MaterialColorsDb::~MaterialColorsDb() { }

MaterialColors::MaterialColors(QObject *parent)
    : QObject(parent), m_jsEngine(qobject_cast<QJSEngine *>(parent))
{
    this->setName("Blue Gray");
}

MaterialColors::MaterialColors(const QString &name, QObject *parent)
    : QObject(parent), m_jsEngine(qobject_cast<QJSEngine *>(parent))
{
    this->setName(name);
}

MaterialColors::~MaterialColors() { }

void MaterialColors::setName(const QString &val)
{
    if (m_name == val)
        return;

    static MaterialColorsDb db;

    m_name = val;
    m_palette = db.data(val);

    emit nameChanged();
}

QJSValue MaterialColors::color(const QString &key) const
{
    QJSEngine *engine = m_jsEngine == nullptr ? qjsEngine(this) : m_jsEngine;
    Q_ASSERT(engine != nullptr);

    QJSValue ret = engine->newObject();
    ret.setProperty("text", engine->toScriptValue<QColor>(m_defaultTextColor));
    ret.setProperty("background", engine->toScriptValue<QColor>(m_defaultBackgroundColor));
    if (key == "c10")
        return ret;

    const QJsonValue val = m_palette.value(key.toLower());
    if (val.isUndefined())
        return ret;

    const QJsonObject obj = val.toObject();

    const QColor textColor(obj.value("text").toString());
    if (textColor.isValid())
        ret.setProperty("text", engine->toScriptValue<QColor>(textColor));

    const QColor backgroundColor(obj.value("background").toString());
    if (backgroundColor.isValid())
        ret.setProperty("background", engine->toScriptValue<QColor>(backgroundColor));

    return ret;
}
