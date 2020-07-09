/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ABSTRACTIMPORTER_H
#define ABSTRACTIMPORTER_H

#include "abstractdeviceio.h"

class QIODevice;

class AbstractImporter : public AbstractDeviceIO
{
    Q_OBJECT

public:
    ~AbstractImporter();

    Q_INVOKABLE bool read();

protected:
    AbstractImporter(QObject *parent=nullptr);
    virtual bool doImport(QIODevice *device) = 0;

    void configureCanvas(int nrBlocks);
    Scene *createScene(const QString &heading);
    SceneElement *addSceneElement(Scene *scene, SceneElement::Type type, const QString &text);
};

#ifdef QDOM_H

class TraverseDomElement
{
public:
    TraverseDomElement(QDomElement &element, ProgressReport *progress)
        : m_element(&element), m_progress(progress) { }
    ~TraverseDomElement() {
        *m_element = m_element->nextSiblingElement(m_element->tagName());
        m_progress->tick();
    }

private:
    QDomElement *m_element = nullptr;
    ProgressReport *m_progress = nullptr;
};

#endif

#endif // ABSTRACTIMPORTER_H
