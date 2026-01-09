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

#ifndef DIACRITICHANDLER_H
#define DIACRITICHANDLER_H

#include <QObject>
#include <QQmlEngine>
#include <QMetaProperty>

class QQuickItem;

class DiacriticHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(DiacriticHandler)

public:
    explicit DiacriticHandler(QObject *parent = nullptr);
    ~DiacriticHandler();

    static DiacriticHandler *qmlAttachedProperties(QObject *parent);

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onEditorItemInputMethodComposingChanged();

private:
    Q_DISABLE_COPY(DiacriticHandler)
    bool m_enabled = false;
    QQuickItem *m_editorItem = nullptr;
    QMetaProperty m_editorInputMethodComposingProperty;
};

#endif // DIACRITICHANDLER_H
