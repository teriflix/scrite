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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QColor>
#include <QQmlEngine>

#include "execlatertimer.h"

class Notification : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(Notification)

public:
    explicit Notification(QObject *parent = nullptr);
    ~Notification();

    static Notification *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(QUrl image READ image WRITE setImage NOTIFY imageChanged)
    void setImage(const QUrl &val);
    QUrl image() const { return m_image; }
    Q_SIGNAL void imageChanged();

    Q_PROPERTY(bool hasImage READ hasImage NOTIFY imageChanged)
    bool hasImage() const { return !m_image.isEmpty(); }

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool active() const { return m_active; }
    Q_SIGNAL void activeChanged();

    Q_PROPERTY(bool autoClose READ autoClose WRITE setAutoClose NOTIFY autoCloseChanged)
    void setAutoClose(bool val);
    bool autoClose() const { return m_autoClose; }
    Q_SIGNAL void autoCloseChanged();

    Q_PROPERTY(int autoCloseDelay READ autoCloseDelay WRITE setAutoCloseDelay NOTIFY autoCloseDelayChanged)
    void setAutoCloseDelay(int val);
    int autoCloseDelay() const { return m_autoCloseDelay; }
    Q_SIGNAL void autoCloseDelayChanged();

    Q_PROPERTY(QStringList buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
    void setButtons(const QStringList &val);
    QStringList buttons() const { return m_buttons; }
    Q_SIGNAL void buttonsChanged();

    Q_PROPERTY(bool hasButtons READ hasButtons NOTIFY buttonsChanged)
    bool hasButtons() const { return !m_buttons.isEmpty(); }

    Q_INVOKABLE void notifyButtonClick(int index);
    Q_INVOKABLE void notifyImageClick();

signals:
    void dismissed();
    void buttonClicked(int index);
    void imageClicked();

private:
    void doAutoClose();
    void timerEvent(QTimerEvent *te);

private:
    bool m_active = false;
    QUrl m_image;
    QColor m_color = QColor(Qt::white);
    QString m_text;
    QString m_title;
    bool m_autoClose = true;
    QColor m_textColor = QColor(Qt::black);
    int m_autoCloseDelay = 2000;
    QStringList m_buttons;
    ExecLaterTimer m_autoCloseTimer;
};

#endif // NOTIFICATION_H
