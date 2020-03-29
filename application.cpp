/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "application.h"
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonObject>
#include <QColorDialog>

Application *Application::instance()
{
    return qobject_cast<Application*>(qApp);
}

Application::Application(int &argc, char **argv, const QVersionNumber &version)
    : QtApplicationClass(argc, argv),
      m_versionNumber(version)
{
    this->setBaseWindowTitle("scrite - build your screenplay");
}

Application::~Application()
{

}

void Application::setBaseWindowTitle(const QString &val)
{
    if(m_baseWindowTitle == val)
        return;

    m_baseWindowTitle = val;
    emit baseWindowTitleChanged();
}

QString Application::typeName(QObject *object) const
{
    if(object == nullptr)
        return QString();

    return QString::fromLatin1(object->metaObject()->className());
}

QJsonObject Application::systemFontInfo() const
{
    QFontDatabase fontdb;

    QJsonObject ret;
    ret.insert("families", QJsonArray::fromStringList(fontdb.families()));

    QJsonArray sizes;
    QList<int> stdSizes = fontdb.standardSizes();
    Q_FOREACH(int stdSize, stdSizes)
        sizes.append( QJsonValue(stdSize) );
    ret.insert("standardSizes", sizes);

    return ret;
}

QColor Application::pickColor(const QColor &initial) const
{
    QColorDialog::ColorDialogOptions options =
            QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog;
    return QColorDialog::getColor(initial, nullptr, "Select Color", options);
}

bool Application::notify(QObject *object, QEvent *event)
{
    const bool ret = QtApplicationClass::notify(object, event);

    if(event->type() == QEvent::ChildAdded)
    {
        QChildEvent *childEvent = reinterpret_cast<QChildEvent*>(event);
        QObject *childObject = childEvent->child();

        if(!childObject->isWidgetType() && !childObject->isWindowType())
        {
            /**
             * For whatever reason, ParentChange event is only sent
             * if the child is a widget or window or declarative-item.
             * I was not aware of this up until now. Classes like
             * StructureElement, SceneElement etc assume that ParentChange
             * event will be sent when they are inserted into the document
             * object tree, so that they can evaluate a pointer to the
             * parent object in the tree. Since these classes are subclassed
             * from QObject, we will need the following lines to explicitly
             * despatch ParentChange events.
             */
            QEvent parentChangeEvent(QEvent::ParentChange);
            QtApplicationClass::notify(childObject, &parentChangeEvent);
        }
    }

    return ret;
}
