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

#include "diacritichandler.h"

#include <QKeyEvent>
#include <QQuickItem>

static const char *inputMethodComposing = "inputMethodComposing";
static const char *inputMethodComposingChanged = SIGNAL(inputMethodComposingChanged());

DiacriticHandler::DiacriticHandler(QObject *parent) : QObject { parent }
{
    m_editorItem = qobject_cast<QQuickItem *>(parent);

#ifdef Q_OS_MACOS
    if (m_editorItem) {
        int propIndex = m_editorItem->metaObject()->indexOfProperty(inputMethodComposing);
        if (propIndex >= 0) {
            m_editorInputMethodComposingProperty = m_editorItem->metaObject()->property(propIndex);
            this->setEnabled(m_editorInputMethodComposingProperty.read(m_editorItem).toBool());
            connect(m_editorItem, inputMethodComposingChanged, this,
                    SLOT(onEditorItemInputMethodComposingChanged()));
        } else
            this->setEnabled(false);

        m_editorItem->installEventFilter(this);
    }
#endif
}

DiacriticHandler::~DiacriticHandler()
{
#ifdef Q_OS_MACOS
    if (m_editorItem)
        m_editorItem->removeEventFilter(this);
#endif
}

DiacriticHandler *DiacriticHandler::qmlAttachedProperties(QObject *parent)
{
    return new DiacriticHandler(parent);
}

void DiacriticHandler::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

bool DiacriticHandler::eventFilter(QObject *object, QEvent *event)
{
#ifdef Q_OS_MACOS
    if (m_enabled && object == m_editorItem && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->isAutoRepeat()) {
            keyEvent->accept();
            return true;
        }
    }
#endif

    return QObject::eventFilter(object, event);
}

void DiacriticHandler::onEditorItemInputMethodComposingChanged()
{
#ifdef Q_OS_MACOS
    if (m_editorItem && m_editorInputMethodComposingProperty.isValid()) {
        this->setEnabled(m_editorInputMethodComposingProperty.read(m_editorItem).toBool());
    }
#endif
}
