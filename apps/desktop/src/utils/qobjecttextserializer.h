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

#ifndef QOBJECT_TEXT_SERIALIZER_H
#define QOBJECT_TEXT_SERIALIZER_H

#include <QObject>
#include <QQmlEngine>

class QObjectTextSerializer : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TextSerializer)
    QML_SINGLETON

public:
    class Interface
    {
    public:
        virtual ~Interface();

        virtual QVariantMap defaultSerializeToTextOptions() const { return QVariantMap(); }
        virtual QString serializeToText(const QVariantMap &options = QVariantMap()) const = 0;
    };

    ~QObjectTextSerializer() override;

    static QObjectTextSerializer *instance();
    static QObjectTextSerializer *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    Q_INVOKABLE static QVariantMap queryDefaultOptions(const QObject *object);
    Q_INVOKABLE static QString toText(const QObject *object,
                                      const QVariantMap &options = QVariantMap());

private:
    explicit QObjectTextSerializer(QObject *parent = nullptr);
};

Q_DECLARE_INTERFACE(QObjectTextSerializer::Interface,
                    "Scrite.App.QObjectTextSerializer.Interface/1.0")

#endif // QOBJECT_TEXT_SERIALIZER_H