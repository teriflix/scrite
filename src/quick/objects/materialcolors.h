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

#ifndef MATERIALCOLORS_H
#define MATERIALCOLORS_H

#include <QColor>
#include <QObject>
#include <QJSValue>
#include <QQmlEngine>
#include <QJsonObject>

class MaterialColors : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MaterialColors(QObject *parent = nullptr);
    explicit MaterialColors(const QString &name, QObject *parent = nullptr);
    ~MaterialColors();

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_PROPERTY(QJSValue regular READ regular NOTIFY nameChanged)
    QJSValue regular() const { return this->color("regular"); }

    Q_PROPERTY(QJSValue c10 READ c10 NOTIFY nameChanged)
    QJSValue c10() const { return this->color("c10"); }

    Q_PROPERTY(QJSValue c50 READ c50 NOTIFY nameChanged)
    QJSValue c50() const { return this->color("c50"); }

    Q_PROPERTY(QJSValue c100 READ c100 NOTIFY nameChanged)
    QJSValue c100() const { return this->color("c100"); }

    Q_PROPERTY(QJSValue c200 READ c200 NOTIFY nameChanged)
    QJSValue c200() const { return this->color("c200"); }

    Q_PROPERTY(QJSValue c300 READ c300 NOTIFY nameChanged)
    QJSValue c300() const { return this->color("c300"); }

    Q_PROPERTY(QJSValue c400 READ c400 NOTIFY nameChanged)
    QJSValue c400() const { return this->color("c400"); }

    Q_PROPERTY(QJSValue c500 READ c500 NOTIFY nameChanged)
    QJSValue c500() const { return this->color("c500"); }

    Q_PROPERTY(QJSValue c600 READ c600 NOTIFY nameChanged)
    QJSValue c600() const { return this->color("c600"); }

    Q_PROPERTY(QJSValue c700 READ c700 NOTIFY nameChanged)
    QJSValue c700() const { return this->color("c700"); }

    Q_PROPERTY(QJSValue c800 READ c800 NOTIFY nameChanged)
    QJSValue c800() const { return this->color("c800"); }

    Q_PROPERTY(QJSValue c900 READ c900 NOTIFY nameChanged)
    QJSValue c900() const { return this->color("c900"); }

    Q_PROPERTY(QJSValue a100 READ a100 NOTIFY nameChanged)
    QJSValue a100() const { return this->color("a100"); }

    Q_PROPERTY(QJSValue a200 READ a200 NOTIFY nameChanged)
    QJSValue a200() const { return this->color("a200"); }

    Q_PROPERTY(QJSValue a400 READ a400 NOTIFY nameChanged)
    QJSValue a400() const { return this->color("a400"); }

    Q_PROPERTY(QJSValue a700 READ a700 NOTIFY nameChanged)
    QJSValue a700() const { return this->color("a700"); }

    Q_PROPERTY(QJsonObject palette READ palette NOTIFY nameChanged)
    QJsonObject palette() const { return m_palette; }

    Q_INVOKABLE QJSValue color(const QString &key) const;

private:
    QString m_name;
    QJSEngine *m_jsEngine = nullptr;
    QJsonObject m_palette;
    QColor m_defaultTextColor = Qt::black;
    QColor m_defaultBackgroundColor = QColor::fromRgbF(1, 1, 1, 0);
};

#endif // MATERIALCOLORS_H
