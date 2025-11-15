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

    // clang-format off
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    // clang-format on
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    // clang-format off
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
    // clang-format on
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    // clang-format off
    Q_PROPERTY(QColor color
               READ color
               WRITE setColor
               NOTIFY colorChanged)
    // clang-format on
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    // clang-format off
    Q_PROPERTY(QColor textColor
               READ textColor
               WRITE setTextColor
               NOTIFY textColorChanged)
    // clang-format on
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    // clang-format off
    Q_PROPERTY(QUrl image
               READ image
               WRITE setImage
               NOTIFY imageChanged)
    // clang-format on
    void setImage(const QUrl &val);
    QUrl image() const { return m_image; }
    Q_SIGNAL void imageChanged();

    // clang-format off
    Q_PROPERTY(bool hasImage
               READ hasImage
               NOTIFY imageChanged)
    // clang-format on
    bool hasImage() const { return !m_image.isEmpty(); }

    // clang-format off
    Q_PROPERTY(bool active
               READ active
               WRITE setActive
               NOTIFY activeChanged)
    // clang-format on
    void setActive(bool val);
    bool active() const { return m_active; }
    Q_SIGNAL void activeChanged();

    // clang-format off
    Q_PROPERTY(bool autoClose
               READ autoClose
               WRITE setAutoClose
               NOTIFY autoCloseChanged)
    // clang-format on
    void setAutoClose(bool val);
    bool autoClose() const { return m_autoClose; }
    Q_SIGNAL void autoCloseChanged();

    // clang-format off
    Q_PROPERTY(int autoCloseDelay
               READ autoCloseDelay
               WRITE setAutoCloseDelay
               NOTIFY autoCloseDelayChanged)
    // clang-format on
    void setAutoCloseDelay(int val);
    int autoCloseDelay() const { return m_autoCloseDelay; }
    Q_SIGNAL void autoCloseDelayChanged();

    // clang-format off
    Q_PROPERTY(bool closeOnButtonClick
               READ isCloseOnButtonClick
               WRITE setCloseOnButtonClick
               NOTIFY closeOnButtonClickChanged)
    // clang-format on
    void setCloseOnButtonClick(bool val);
    bool isCloseOnButtonClick() const { return m_closeOnButtonClick; }
    Q_SIGNAL void closeOnButtonClickChanged();

    // clang-format off
    Q_PROPERTY(QStringList buttons
               READ buttons
               WRITE setButtons
               NOTIFY buttonsChanged)
    // clang-format on
    void setButtons(const QStringList &val);
    QStringList buttons() const { return m_buttons; }
    Q_SIGNAL void buttonsChanged();

    // clang-format off
    Q_PROPERTY(bool hasButtons
               READ hasButtons
               NOTIFY buttonsChanged)
    // clang-format on
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
    bool m_closeOnButtonClick = true;
    ExecLaterTimer m_autoCloseTimer;
};

#endif // NOTIFICATION_H
