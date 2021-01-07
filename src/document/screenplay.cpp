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

#include "undoredo.h"
#include "hourglass.h"
#include "screenplay.h"
#include "scritedocument.h"
#include "garbagecollector.h"

#include <QScopedValueRollback>

ScreenplayElement::ScreenplayElement(QObject *parent)
    : QObject(parent),
      m_scene(this, "scene"),
      m_screenplay(this, "screenplay")
{
    m_screenplay = qobject_cast<Screenplay*>(parent);

    connect(this, &ScreenplayElement::sceneChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::expandedChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::userSceneNumberChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::breakTitleChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::editorHintsChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::elementChanged, [=](){
        this->markAsModified();
    });

    connect(this, &ScreenplayElement::sceneChanged, [=]() {
        if(m_elementType == BreakElementType)
            emit breakTitleChanged();
    });

    connect(this, &ScreenplayElement::sceneNumberChanged, this, &ScreenplayElement::resolvedSceneNumberChanged);
    connect(this, &ScreenplayElement::userSceneNumberChanged, this, &ScreenplayElement::resolvedSceneNumberChanged);
    connect(this, &ScreenplayElement::userSceneNumberChanged, this, &ScreenplayElement::evaluateSceneNumberRequest);
}

ScreenplayElement::~ScreenplayElement()
{
    GarbageCollector::instance()->avoidChildrenOf(this);
    emit aboutToDelete(this);
}

void ScreenplayElement::setElementType(ScreenplayElement::ElementType val)
{
    if(m_elementType == val || m_elementTypeIsSet)
        return;

    m_elementType = val;
    emit elementTypeChanged();
}

void ScreenplayElement::setBreakType(int val)
{
    if(m_breakType == val || m_elementType != BreakElementType)
        return;

    m_breakType = val;

    if(m_sceneID.isEmpty())
    {
        QString id;
        int chapter = 1;
        for(int i=0; i<m_screenplay->elementCount(); i++)
        {
            ScreenplayElement *element = m_screenplay->elementAt(i);
            if(element->elementType() == BreakElementType)
            {
                if(element->breakType() == m_breakType)
                    ++chapter;
            }
        }

        switch(m_breakType)
        {
        case Screenplay::Act:
            id = "Act ";
            break;
        case Screenplay::Chapter:
            id = "Chapter ";
            break;
        case Screenplay::Interval:
            id = "Interval";
            break;
        default:
            id = "Break";
        }

        if(m_breakType != Screenplay::Interval)
            id += QString::number(chapter);
        this->setSceneFromID(id);
    }

    emit breakTypeChanged();
}

void ScreenplayElement::setBreakTitle(const QString &val)
{
    if(m_breakTitle == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "breakTitle");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_breakTitle = val;
    emit breakTitleChanged();
}

void ScreenplayElement::setScreenplay(Screenplay *val)
{
    if(m_screenplay != nullptr || m_screenplay == val)
        return;

    m_screenplay = val;
    emit screenplayChanged();
}

void ScreenplayElement::setSceneFromID(const QString &val)
{
    m_sceneID = val;
    if(m_elementType == BreakElementType)
        return;

    if(m_screenplay == nullptr)
        return;

    ScriteDocument *document = m_screenplay->scriteDocument();
    if(document == nullptr)
        return;

    Structure *structure = document->structure();
    if(structure == nullptr)
        return;

    StructureElement *element = structure->findElementBySceneID(val);
    if(element == nullptr)
        return;

    m_elementTypeIsSet = true;
    this->setScene(element->scene());
    if(m_scene != nullptr)
        m_sceneID.clear();
}

QString ScreenplayElement::sceneID() const
{
    if(m_elementType == BreakElementType)
    {
        if(m_sceneID.isEmpty())
        {
            switch(m_breakType)
            {
            case Screenplay::Act: return "Act";
            case Screenplay::Chapter: return "Chapter";
            case Screenplay::Interval: return "Interval";
            default: break;
            }
            return "Break";
        }

        return m_sceneID;
    }

    return m_scene ? m_scene->id() : m_sceneID;
}

void ScreenplayElement::setUserSceneNumber(const QString &val)
{
    if(m_userSceneNumber == val)
        return;

    m_userSceneNumber = val.toUpper();
    emit userSceneNumberChanged();
}

QString ScreenplayElement::resolvedSceneNumber() const
{
    return m_userSceneNumber.isEmpty() ? QString::number(this->sceneNumber()) : m_userSceneNumber;
}

void ScreenplayElement::setScene(Scene *val)
{
    if(m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    m_sceneID = m_scene->id();
    connect(m_scene, &Scene::aboutToDelete, this, &ScreenplayElement::resetScene);
    connect(m_scene, &Scene::sceneAboutToReset, this, &ScreenplayElement::sceneAboutToReset);
    connect(m_scene, &Scene::sceneReset, this, &ScreenplayElement::sceneReset);
    connect(m_scene, &Scene::typeChanged, this, &ScreenplayElement::sceneTypeChanged);

    if(m_screenplay)
        connect(m_scene->heading(), &SceneHeading::enabledChanged, this, &ScreenplayElement::evaluateSceneNumberRequest);

    emit sceneChanged();
}

void ScreenplayElement::setExpanded(bool val)
{
    if(m_expanded == val)
        return;

    m_expanded = val;
    emit expandedChanged();
}

void ScreenplayElement::setUserData(const QJsonValue &val)
{
    if(m_userData == val)
        return;

    m_userData = val;
    emit userDataChanged();
}

void ScreenplayElement::setEditorHints(const QJsonValue &val)
{
    if(m_editorHints == val)
        return;

    m_editorHints = val;
    emit editorHintsChanged();
}

void ScreenplayElement::setSelected(bool val)
{
    if(m_selected == val)
        return;

    m_selected = val;
    emit selectedChanged();
}

bool ScreenplayElement::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        this->setScreenplay( qobject_cast<Screenplay*>(this->parent()) );
        if(m_scene == nullptr && !m_sceneID.isEmpty())
            this->setSceneFromID(m_sceneID);
    }
    else if(event->type() == QEvent::DynamicPropertyChange)
    {
        QDynamicPropertyChangeEvent *propEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
        const QByteArray propName = propEvent->propertyName();
        if( propName == QByteArrayLiteral("#sceneNumber") )
        {
            m_customSceneNumber = property(propName).toInt();
            emit sceneNumberChanged();
        }
    }

    return QObject::event(event);
}

void ScreenplayElement::evaluateSceneNumber(int &number)
{
    int sn = -1;
    if(!m_userSceneNumber.isEmpty())
        return;

    if(m_scene != nullptr && m_scene->heading()->isEnabled())
    {
        if(number <= 0)
            number = 1;
        sn = number++;
    }

    if(m_sceneNumber != sn)
    {
        m_sceneNumber = sn;
        emit sceneNumberChanged();
    }
}


void ScreenplayElement::resetScene()
{
    if(m_screenplay != nullptr)
        m_screenplay->removeElement(this);
    else
    {
        if(m_sceneID.isEmpty())
            m_sceneID = m_scene->id();
        m_scene = nullptr;
        this->deleteLater();
    }
}

void ScreenplayElement::resetScreenplay()
{
    m_screenplay = nullptr;
    emit screenplayChanged();

    this->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////

static const QString coverPagePhotoPath("coverPage/photo.jpg");

Screenplay::Screenplay(QObject *parent)
    : QAbstractListModel(parent),
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent)),
      m_activeScene(this, "activeScene"),
      m_sceneNumberEvaluationTimer("Screenplay.m_sceneNumberEvaluationTimer")
{
    connect(this, &Screenplay::titleChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::emailChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::authorChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::loglineChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::websiteChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::basedOnChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::contactChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::versionChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::addressChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::subtitleChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::elementsChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::phoneNumberChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::coverPagePhotoChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::elementsChanged, this, &Screenplay::evaluateSceneNumbersLater);
    connect(this, &Screenplay::coverPagePhotoSizeChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::titlePageIsCenteredChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::screenplayChanged, [=](){ this->markAsModified(); });

    m_author = QSysInfo::machineHostName();
    m_version = "Initial Draft";
}

Screenplay::~Screenplay()
{
    GarbageCollector::instance()->avoidChildrenOf(this);
    emit aboutToDelete(this);
}

void Screenplay::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();

    this->evaluateHasTitlePageAttributes();
}

void Screenplay::setSubtitle(const QString &val)
{
    if(m_subtitle == val)
        return;

    m_subtitle = val;
    emit subtitleChanged();
}

void Screenplay::setLogline(const QString &val)
{
    if(m_logline == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "logline");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_logline = val;
    emit loglineChanged();
}

void Screenplay::setBasedOn(const QString &val)
{
    if(m_basedOn == val)
        return;

    m_basedOn = val;
    emit basedOnChanged();
}

void Screenplay::setAuthor(const QString &val)
{
    if(m_author == val)
        return;

    m_author = val;
    emit authorChanged();

    this->evaluateHasTitlePageAttributes();
}

void Screenplay::setContact(const QString &val)
{
    if(m_contact == val)
        return;

    m_contact = val;
    emit contactChanged();
}

void Screenplay::setAddress(QString val)
{
    if(m_address == val)
        return;

    m_address = val;
    emit addressChanged();
}

void Screenplay::setPhoneNumber(QString val)
{
    if(m_phoneNumber == val)
        return;

    m_phoneNumber = val;
    emit phoneNumberChanged();
}

void Screenplay::setEmail(QString val)
{
    if(m_email == val)
        return;

    m_email = val;
    emit emailChanged();
}

void Screenplay::setWebsite(QString val)
{
    if(m_website == val)
        return;

    m_website = val;
    emit websiteChanged();
}

void Screenplay::setVersion(const QString &val)
{
    if(m_version == val)
        return;

    m_version = val;
    emit versionChanged();

    this->evaluateHasTitlePageAttributes();
}

void Screenplay::setCoverPagePhoto(const QString &val)
{
    HourGlass hourGlass;

#if 0
    QImage image = val.isEmpty() ? QImage() : QImage(val);
    if( image.isNull() )
    {
        m_scriteDocument->fileSystem()->remove(coverPagePhotoPath);
        m_coverPagePhoto.clear();
    }
    else
    {
        if(m_scriteDocument->fileSystem()->contains(val))
            m_coverPagePhoto = val;
        else
        {
            const QSize imageSize = image.size().scaled(1920, 1080, Qt::KeepAspectRatio);
            if(image.width() > imageSize.width() || image.height() > imageSize.height())
                image = image.scaled(imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            const QString photoPath = m_scriteDocument->fileSystem()->absolutePath(coverPagePhotoPath, true);
            if(image.save(photoPath, "JPG"))
                m_coverPagePhoto = photoPath;
            else
                m_coverPagePhoto.clear();
        }
    }
#else
    const QSize fullHdSize(1920, 1080);
    const QString val2 = m_scriteDocument->fileSystem()->addImage(val, coverPagePhotoPath, fullHdSize);
    m_coverPagePhoto = val2.isEmpty() ? val2 : m_scriteDocument->fileSystem()->absolutePath(val2);
#endif

    emit coverPagePhotoChanged();
}

void Screenplay::clearCoverPagePhoto()
{
    this->setCoverPagePhoto(QString());
}

void Screenplay::setCoverPagePhotoSize(Screenplay::CoverPagePhotoSize val)
{
    if(m_coverPagePhotoSize == val)
        return;

    m_coverPagePhotoSize = val;
    emit coverPagePhotoSizeChanged();
}

void Screenplay::setTitlePageIsCentered(bool val)
{
    if(m_titlePageIsCentered == val)
        return;

    m_titlePageIsCentered = val;
    emit titlePageIsCenteredChanged();
}

QQmlListProperty<ScreenplayElement> Screenplay::elements()
{
    return QQmlListProperty<ScreenplayElement>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Screenplay::staticAppendElement,
                &Screenplay::staticElementCount,
                &Screenplay::staticElementAt,
                &Screenplay::staticClearElements);
}

void Screenplay::addElement(ScreenplayElement *ptr)
{
    this->insertElementAt(ptr, -1);
}

void Screenplay::addScene(Scene *scene)
{
    if(scene == nullptr)
        return;

    ScreenplayElement *element = new ScreenplayElement(this);
    element->setScene(scene);
    this->addElement(element);
}

static void screenplayAppendElement(Screenplay *screenplay, ScreenplayElement *ptr) { screenplay->addElement(ptr); }
static void screenplayRemoveElement(Screenplay *screenplay, ScreenplayElement *ptr) { screenplay->removeElement(ptr); }
static void screenplayInsertElement(Screenplay *screenplay, ScreenplayElement *ptr, int index) { screenplay->insertElementAt(ptr, index); }
static ScreenplayElement *screenplayElementAt(Screenplay *screenplay, int index) { return screenplay->elementAt(index); }
static int screenplayIndexOfElement(Screenplay *screenplay, ScreenplayElement *ptr) { return screenplay->indexOfElement(ptr); }

void Screenplay::insertElementAt(ScreenplayElement *ptr, int index)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    index = (index < 0 || index >= m_elements.size()) ? m_elements.size() : index;

    QScopedPointer< PushObjectListCommand<Screenplay,ScreenplayElement> > cmd;
    ObjectPropertyInfo *info = m_scriteDocument == nullptr ? nullptr : ObjectPropertyInfo::get(this, "elements");
    if(info != nullptr && !info->isLocked())
    {
        ObjectListPropertyMethods<Screenplay,ScreenplayElement> methods(&screenplayAppendElement, &screenplayRemoveElement, &screenplayInsertElement, &screenplayElementAt, screenplayIndexOfElement);
        cmd.reset( new PushObjectListCommand<Screenplay,ScreenplayElement> (ptr, this, info->property, ObjectList::InsertOperation, methods) );
    }

    this->beginInsertRows(QModelIndex(), index, index);
    if(index == m_elements.size())
        m_elements.append(ptr);
    else
        m_elements.insert(index, ptr);

    ptr->setParent(this);
    connect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    connect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);
    connect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset);
    connect(ptr, &ScreenplayElement::evaluateSceneNumberRequest, this, &Screenplay::evaluateSceneNumbersLater);
    connect(ptr, &ScreenplayElement::sceneTypeChanged, this, &Screenplay::evaluateSceneNumbersLater);
    if(ptr->elementType() == ScreenplayElement::BreakElementType)
        connect(ptr, &ScreenplayElement::breakTitleChanged, this, &Screenplay::breakTitleChanged);

    this->endInsertRows();

    emit elementInserted(ptr, index);
    emit elementCountChanged();
    emit elementsChanged();

    if(ptr->elementType() == ScreenplayElement::SceneElementType && (this->scriteDocument() && !this->scriteDocument()->isLoading()))
        this->setCurrentElementIndex(index);
}

void Screenplay::removeElement(ScreenplayElement *ptr)
{
    HourGlass hourGlass;

    if(ptr == nullptr)
        return;

    const int row = m_elements.indexOf(ptr);
    if(row < 0)
        return;

    QScopedPointer< PushObjectListCommand<Screenplay,ScreenplayElement> > cmd;
    ObjectPropertyInfo *info = m_scriteDocument == nullptr ? nullptr : ObjectPropertyInfo::get(this, "elements");
    if(info != nullptr && !info->isLocked())
    {
        ObjectListPropertyMethods<Screenplay,ScreenplayElement> methods(&screenplayAppendElement, &screenplayRemoveElement, &screenplayInsertElement, &screenplayElementAt, screenplayIndexOfElement);
        cmd.reset( new PushObjectListCommand<Screenplay,ScreenplayElement> (ptr, this, info->property, ObjectList::RemoveOperation, methods) );
    }

    this->beginRemoveRows(QModelIndex(), row, row);
    m_elements.removeAt(row);

    disconnect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    disconnect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);
    disconnect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset);
    disconnect(ptr, &ScreenplayElement::evaluateSceneNumberRequest, this, &Screenplay::evaluateSceneNumbersLater);
    disconnect(ptr, &ScreenplayElement::sceneTypeChanged, this, &Screenplay::evaluateSceneNumbersLater);

    this->endRemoveRows();

    emit elementRemoved(ptr, row);
    emit elementCountChanged();
    emit elementsChanged();

    this->validateCurrentElementIndex();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

class ScreenplayElementMoveCommand : public QUndoCommand
{
public:
    ScreenplayElementMoveCommand(Screenplay *screenplay, ScreenplayElement *element, int fromRow, int toRow);
    ~ScreenplayElementMoveCommand();

    static bool lock;

    // QUndoCommand interface
    void undo();
    void redo();

    void markAsObselete();

private:
    int m_toRow = -1;
    int m_fromRow = -1;
    QPointer<Screenplay> m_screenplay;
    QPointer<ScreenplayElement> m_element;
};

bool ScreenplayElementMoveCommand::lock = false;

ScreenplayElementMoveCommand::ScreenplayElementMoveCommand(Screenplay *screenplay, ScreenplayElement *element, int fromRow, int toRow)
    : QUndoCommand(),
      m_toRow(toRow),
      m_fromRow(fromRow),
      m_screenplay(screenplay),
      m_element(element)
{

}

ScreenplayElementMoveCommand::~ScreenplayElementMoveCommand()
{

}

void ScreenplayElementMoveCommand::undo()
{
    if(m_screenplay.isNull() || m_element.isNull() || m_fromRow < 0 || m_toRow < 0)
    {
        this->setObsolete(true);
        return;
    }

    lock = true;
    m_screenplay->moveElement(m_element, m_fromRow);
    lock = false;
}

void ScreenplayElementMoveCommand::redo()
{
    if(m_screenplay.isNull() || m_element.isNull() || m_fromRow < 0 || m_toRow < 0)
    {
        this->setObsolete(true);
        return;
    }

    lock = true;
    m_screenplay->moveElement(m_element, m_toRow);
    lock = false;
}

void Screenplay::moveElement(ScreenplayElement *ptr, int toRow)
{
    if(ptr == nullptr || toRow >= m_elements.size())
        return;

    if(toRow < 0)
        toRow = m_elements.size()-1;

    const int fromRow = m_elements.indexOf(ptr);
    if(fromRow < 0)
        return;

    if(fromRow == toRow)
        return;

    this->beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), toRow < fromRow ? toRow : toRow+1);
    m_elements.move(fromRow, toRow);
    this->endMoveRows();

    if(fromRow == m_currentElementIndex)
        this->setCurrentElementIndex(toRow);

    emit elementMoved(ptr, fromRow, toRow);
    emit elementsChanged();

    if(UndoStack::active() != nullptr && !ScreenplayElementMoveCommand::lock)
        UndoStack::active()->push(new ScreenplayElementMoveCommand(this, ptr, fromRow, toRow));
}

// TODO implement undo-command to revert group move operation
// This could be a simple pre-and-post scene id list thing.
class ScreenplayElementsMoveCommand : public QUndoCommand
{
public:
    ScreenplayElementsMoveCommand(Screenplay *screenplay);
    ~ScreenplayElementsMoveCommand();

    void undo();
    void redo();

private:
    QVariantList save() const;
    bool restore(const QVariantList &array) const;

private:
    bool m_initialized = false;
    QVariantList m_after;
    QVariantList m_before;
    QPointer<Screenplay> m_screenplay;
    QMetaObject::Connection m_connection;
};

ScreenplayElementsMoveCommand::ScreenplayElementsMoveCommand(Screenplay *screenplay)
    : QUndoCommand(QStringLiteral("Element Selection Move")),
      m_screenplay(screenplay)
{
    m_before = this->save();

    m_connection = QObject::connect(screenplay, &Screenplay::aboutToDelete, [=]() {
        this->setObsolete(true);
    });
}

ScreenplayElementsMoveCommand::~ScreenplayElementsMoveCommand()
{
    QObject::disconnect(m_connection);
}

void ScreenplayElementsMoveCommand::undo()
{
    if(m_screenplay.isNull() || !this->restore(m_before))
        this->setObsolete(true);
}

void ScreenplayElementsMoveCommand::redo()
{
    if(!m_initialized)
    {
        m_initialized = true;
        if(m_screenplay.isNull())
            this->setObsolete(true);
        else
            m_after = this->save();
        return;
    }

    if(m_screenplay.isNull() || !this->restore(m_after))
        this->setObsolete(true);
}

QVariantList ScreenplayElementsMoveCommand::save() const
{
    HourGlass hourGlass;

    QVariantList ret;
    if(m_screenplay.isNull())
        return ret;

    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        if(element->elementType() == ScreenplayElement::BreakElementType)
            ret << element->breakType();
        else
            ret << element->sceneID();
    }

    return ret;
}

bool ScreenplayElementsMoveCommand::restore(const QVariantList &array) const
{
    HourGlass hourGlass;

    QList<ScreenplayElement*> elements = m_screenplay->getElements();
    if(array.size() != elements.size())
        return false;

    auto findSceneElement = [&elements](const QString &id) {
        for(int i=0; i<elements.size(); i++) {
            ScreenplayElement *element = elements.at(i);
            if(element->sceneID() == id) {
                elements.takeAt(i);
                return element;
            }
        }

        return (ScreenplayElement*)nullptr;
    };

    auto findBreakElement = [&elements](int type) {
        for(int i=0; i<elements.size(); i++) {
            ScreenplayElement *element = elements.at(i);
            if(element->elementType() == ScreenplayElement::BreakElementType && element->breakType() == type) {
                elements.takeAt(i);
                return element;
            }
        }
        return (ScreenplayElement*)nullptr;
    };

    QList<ScreenplayElement*> newElements;
    for(QVariant item : array)
    {
        ScreenplayElement *element = nullptr;

        if(item.userType() == QMetaType::QString)
            element = findSceneElement(item.toString());
        else
            element = findBreakElement(item.toInt());

        if(element == nullptr)
            return false;

        newElements.append(element);
    }

    if(newElements.size() != array.size() || !elements.isEmpty())
        return false;

    return m_screenplay->setElements(newElements);
}

void Screenplay::moveSelectedElements(int toRow)
{
    HourGlass hourGlass;

    toRow = qBound(0, toRow, m_elements.size()-1);

    /**
     * Why are we resetting the models while moving multiple elements, instead of removing and inserting them?
     * Or better yet, moving them?
     *
     * The ScreenplayTextDocument class was built with the assumption that elements will be added, removed or moved
     * one at a time. So, if we removed all selected elements and reinserted them elsewhere; the text document
     * wont get updated properly.
     *
     * But when we reset the model, it will simply update the whole screenplay at once. This could be a bit slow,
     * but is still far better than moving one scene at a time.
     */
    ScreenplayElementsMoveCommand *cmd = nullptr;

    ScreenplayElement *toRowElement = this->elementAt(toRow);
    if(toRowElement == nullptr)
        return;

    int fromRow = -1;
    QList<ScreenplayElement*> selectedElements;
    for(int i=m_elements.size()-1; i>=0; i--)
    {
        ScreenplayElement *element = m_elements.at(i);
        if(!element->isSelected())
            continue;

        if(cmd == nullptr)
        {
            cmd = new ScreenplayElementsMoveCommand(this);
            this->beginResetModel();
        }

        selectedElements.prepend(element);
        m_elements.removeAt(i);
        fromRow = i;
    }

    if(cmd == nullptr)
        return;

    toRow = m_elements.indexOf(toRowElement);
    if(toRow < 0)
    {
        delete cmd;
        return;
    }

    if(toRow > fromRow)
        ++toRow;

    while(!selectedElements.isEmpty())
    {
        ScreenplayElement *element = selectedElements.takeLast();
        m_elements.insert(toRow, element);
    }

    this->endResetModel();

    emit elementsChanged();

    if(UndoStack::active() != nullptr)
        UndoStack::active()->push(cmd);
}

// TODO implement undo-command to revert group remove operation
// This could be a simple pre-and-post scene id list thing.

void Screenplay::removeSelectedElements()
{

}

void Screenplay::clearSelection()
{
    for(ScreenplayElement *element : m_elements)
        element->setSelected(false);
}

ScreenplayElement *Screenplay::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

int Screenplay::elementCount() const
{
    return m_elements.size();
}

class UndoClearScreenplayCommand : public QUndoCommand
{
public:
    UndoClearScreenplayCommand(Screenplay *screenplay, const QStringList &sceneIds);
    ~UndoClearScreenplayCommand();

    // QUndoCommand interface
    void undo();
    void redo();

private:
    bool m_firstRedoDone = false;
    char m_padding[7];
    QStringList m_sceneIds;
    Screenplay *m_screenplay = nullptr;
    QMetaObject::Connection m_connection;
};

UndoClearScreenplayCommand::UndoClearScreenplayCommand(Screenplay *screenplay, const QStringList &sceneIds)
    : QUndoCommand(), m_sceneIds(sceneIds), m_screenplay(screenplay)
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    m_connection = QObject::connect(m_screenplay, &Screenplay::destroyed, [this]() {
        this->setObsolete(true);
    });
}

UndoClearScreenplayCommand::~UndoClearScreenplayCommand()
{
    QObject::disconnect(m_connection);
}

void UndoClearScreenplayCommand::undo()
{
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(m_screenplay, "elements");
    if(info) info->lock();

    ScriteDocument *document = m_screenplay->scriteDocument();
    Structure *structure = document->structure();
    Q_FOREACH(QString sceneId, m_sceneIds)
    {
        StructureElement *element = structure->findElementBySceneID(sceneId);
        if(element == nullptr)
            continue;

        Scene *scene = element->scene();
        ScreenplayElement *screenplayElement = new ScreenplayElement(m_screenplay);
        screenplayElement->setScene(scene);
        m_screenplay->addElement(screenplayElement);
    }

    if(info) info->unlock();
}

void UndoClearScreenplayCommand::redo()
{
    if(!m_firstRedoDone)
    {
        m_firstRedoDone = true;
        return;
    }

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(m_screenplay, "elements");
    if(info) info->lock();

    while(m_screenplay->elementCount())
        m_screenplay->removeElement( m_screenplay->elementAt(0) );

    if(info) info->unlock();
}

void Screenplay::clearElements()
{
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(info) info->lock();

    this->beginResetModel();

    QStringList sceneIds;
    while(m_elements.size())
    {
        sceneIds << m_elements.first()->sceneID();
        // this->removeElement(m_elements.first());

        ScreenplayElement *ptr = m_elements.takeLast();
        emit elementRemoved(ptr, m_elements.size());
        disconnect(ptr, nullptr, this, nullptr);
        GarbageCollector::instance()->add(ptr);
    }

    this->endResetModel();

    emit elementCountChanged();
    emit elementsChanged();
    this->validateCurrentElementIndex();

    if(UndoStack::active())
        UndoStack::active()->push(new UndoClearScreenplayCommand(this, sceneIds));

    if(info) info->unlock();
}

class SplitElementUndoCommand : public QUndoCommand
{
public:
    SplitElementUndoCommand(ScreenplayElement *ptr);
    ~SplitElementUndoCommand();

    void prepare();
    void commit(Scene *splitScene);

    // QUndoCommand interface
    void undo();
    void redo();

private:
    QByteArray captureScreenplayElements() const;
    void applyScreenplayElements(const QByteArray &bytes);

    QPair<StructureElement*,StructureElement*> findStructureElements() const;

private:
    QString m_splitSceneID;
    QString m_originalSceneID;
    QByteArray m_splitScenesData[2];
    QByteArray m_originalSceneData;
    QList<int> m_splitElementIndexes;
    Screenplay *m_screenplay = nullptr;
    ScreenplayElement *m_screenplayElement = nullptr;
};

SplitElementUndoCommand::SplitElementUndoCommand(ScreenplayElement *ptr)
    : QUndoCommand(), m_screenplayElement(ptr)
{

}

SplitElementUndoCommand::~SplitElementUndoCommand()
{

}

void SplitElementUndoCommand::prepare()
{
    if(m_screenplayElement == nullptr)
    {
        this->setObsolete(true);
        return;
    }

    m_screenplay = m_screenplayElement->screenplay();

    Scene *originalScene = m_screenplayElement->scene();
    m_originalSceneID = originalScene->id();
    m_originalSceneData = originalScene->toByteArray();
}

void SplitElementUndoCommand::commit(Scene *splitScene)
{
    if(m_screenplayElement == nullptr || splitScene == nullptr || m_screenplay == nullptr)
    {
        this->setObsolete(true);
        return;
    }

    m_splitSceneID = splitScene->id();
    m_splitScenesData[0] = m_screenplayElement->scene()->toByteArray();
    m_splitScenesData[1] = splitScene->toByteArray();
    UndoStack::active()->push(this);
}

void SplitElementUndoCommand::undo()
{
    QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);

    QPair<StructureElement*,StructureElement*> pair = this->findStructureElements();
    if(pair.first == nullptr || pair.second == nullptr)
    {
        this->setObsolete(true);
        return;
    }

    m_screenplay->scriteDocument()->setBusyMessage("Performing undo of split-scene operation...");

    Structure *structure = m_screenplay->scriteDocument()->structure();
    Scene *splitScene = pair.second->scene();
    Scene *originalScene = pair.first->scene();

    // Reset our screenplay first, one of the scenes that it refers to is about to be destroyed.
    Q_FOREACH(int index, m_splitElementIndexes)
        m_screenplay->removeElement( m_screenplay->elementAt(index) );

    // Destroy the split scene
    GarbageCollector::instance()->add(splitScene);
    structure->removeElement(pair.second);

    // Restore Original Scene to its original state
    originalScene->resetFromByteArray(m_originalSceneData);

    m_screenplay->scriteDocument()->clearBusyMessage();
}

void SplitElementUndoCommand::redo()
{
    if(m_splitElementIndexes.isEmpty())
    {
        if(m_screenplayElement == nullptr)
        {
            this->setObsolete(true);
            return;
        }

        for(int i=m_screenplay->elementCount()-1; i>=0; i--)
        {
            ScreenplayElement *element = m_screenplay->elementAt(i);
            if(element->sceneID() == m_splitSceneID)
                m_splitElementIndexes.append(i);
        }

        m_screenplayElement = nullptr;

        return;
    }

    QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);

    QPair<StructureElement*,StructureElement*> pair = this->findStructureElements();
    if(pair.first == nullptr || pair.second != nullptr)
    {
        this->setObsolete(true);
        return;
    }

    m_screenplay->scriteDocument()->setBusyMessage("Performing redo of split-scene operation...");

    Structure *structure = m_screenplay->scriteDocument()->structure();

    // Create the split scene first
    StructureElement *splitStructureElement = new StructureElement(structure);
    splitStructureElement->setX(pair.first->x() + 300);
    splitStructureElement->setY(pair.first->y() + 80);
    Scene *splitScene = new Scene(splitStructureElement);
    splitScene->setId(m_splitSceneID);
    splitScene->resetFromByteArray(m_splitScenesData[1]);
    splitStructureElement->setScene(splitScene);
    structure->insertElement(splitStructureElement, structure->indexOfElement(pair.first)+1);

    // Update original scene with its split data
    Scene *originalScene = pair.first->scene();
    originalScene->resetFromByteArray(m_splitScenesData[0]);

    // Reset our screenplay now
    Q_FOREACH(int index, m_splitElementIndexes)
    {
        ScreenplayElement *element = new ScreenplayElement(m_screenplay);
        element->setElementType(ScreenplayElement::SceneElementType);
        element->setScene(splitScene);
        m_screenplay->insertElementAt(element, index);
    }

    m_screenplay->scriteDocument()->clearBusyMessage();
}

QByteArray SplitElementUndoCommand::captureScreenplayElements() const
{
    QByteArray bytes;

    QDataStream ds(&bytes, QIODevice::WriteOnly);
    ds << m_screenplay->elementCount();
    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        ds << int(element->elementType());
        ds << element->sceneID();
    }

    ds << m_screenplay->currentElementIndex();
    return bytes;
}

void SplitElementUndoCommand::applyScreenplayElements(const QByteArray &bytes)
{
    if(bytes.isEmpty())
    {
        this->setObsolete(true);
        return;
    }

    QDataStream ds(bytes);

    int nrElements = 0;
    ds >> nrElements;

    m_screenplay->clearElements();

    for(int i=0; i<nrElements; i++)
    {
        ScreenplayElement *element = new ScreenplayElement(m_screenplay);

        int type = -1;
        ds >> type;
        element->setElementType( ScreenplayElement::ElementType(type) );

        QString sceneID;
        ds >> sceneID;
        element->setSceneFromID(sceneID);

        m_screenplay->addElement(element);
    }

    int currentIndex = -1;
    ds >> currentIndex;
    m_screenplay->setCurrentElementIndex(currentIndex);
}

QPair<StructureElement *, StructureElement *> SplitElementUndoCommand::findStructureElements() const
{
    Structure *structure = m_screenplay->scriteDocument()->structure();

    StructureElement *splitSceneStructureElement = structure->findElementBySceneID(m_splitSceneID);
    if(splitSceneStructureElement == nullptr || splitSceneStructureElement->scene() == nullptr)
        splitSceneStructureElement = nullptr;

    StructureElement *originalSceneStructureElement = structure->findElementBySceneID(m_originalSceneID);
    if(originalSceneStructureElement == nullptr || originalSceneStructureElement->scene() == nullptr)
        originalSceneStructureElement = nullptr;

    return qMakePair(originalSceneStructureElement, splitSceneStructureElement);
}

ScreenplayElement *Screenplay::splitElement(ScreenplayElement *ptr, SceneElement *element, int textPosition)
{
    ScreenplayElement *ret = nullptr;
    QScopedPointer<SplitElementUndoCommand> undoCommand( new SplitElementUndoCommand(ptr) );

    {
        QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);

        if(ptr == nullptr)
            return ret;

        const int index = this->indexOfElement(ptr);
        if(index < 0)
            return ret;

        if(ptr->elementType() == ScreenplayElement::BreakElementType)
            return ret;

        Scene *scene = ptr->scene();
        if(scene == nullptr)
            return ret;

        undoCommand->prepare();

        Structure *structure = this->scriteDocument()->structure();
        StructureElement *structureElement = structure->findElementBySceneID(scene->id());
        if(structureElement == nullptr)
            return ret;

        StructureElement *newStructureElement = structure->splitElement(structureElement, element, textPosition);
        if(newStructureElement == nullptr)
            return ret;

        for(int i=this->elementCount()-1; i>=0; i--)
        {
            ScreenplayElement *screenplayElement = this->elementAt(i);
            if(screenplayElement->scene() == scene)
            {
                ScreenplayElement *newScreenplayElement = new ScreenplayElement(this);
                newScreenplayElement->setScene(newStructureElement->scene());
                this->insertElementAt(newScreenplayElement, i+1);

                if(screenplayElement == ptr)
                    ret = newScreenplayElement;
            }
        }

        this->setCurrentElementIndex(this->indexOfElement(ret));
    }

    if(ret != nullptr && UndoStack::active() != nullptr)
    {
        undoCommand->commit(ret->scene());
        undoCommand.take();
    }

    return ret;
}

ScreenplayElement *Screenplay::mergeElementWithPrevious(ScreenplayElement *element)
{
    /* We dont capture undo for this action, because user can always split scene once again */
    QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);
    Screenplay *screenplay = this;

    if(element == nullptr || element->scene() == nullptr)
        return nullptr;

    Scene *currentScene = element->scene();
    int previousElementIndex = screenplay->indexOfElement(element)-1;
    while(previousElementIndex >= 0)
    {
        ScreenplayElement *element = screenplay->elementAt(previousElementIndex);
        if(element == nullptr || element->scene() == nullptr)
        {
            --previousElementIndex;
            continue;
        }

        break;
    }

    if(previousElementIndex < 0)
        return nullptr;

    ScreenplayElement *previousSceneElement = screenplay->elementAt(previousElementIndex);
    Scene *previousScene = previousSceneElement->scene();

    SceneElement *newElement = new SceneElement(previousScene);
    newElement->setType(SceneElement::Action);
    newElement->setText(QStringLiteral("--"));
    previousScene->addElement(newElement);

    int length = 0;
    for(int i=0; i<previousScene->elementCount(); i++)
        length += previousScene->elementAt(i)->text().length();
    length += previousScene->elementCount();

    while(currentScene->elementCount())
    {
        SceneElement *element = currentScene->elementAt(0);

        newElement = new SceneElement(previousScene);
        newElement->setType(element->type());
        newElement->setText(element->text());
        previousScene->addElement(newElement);

        currentScene->removeElement(element);
    }

    previousScene->setCursorPosition(length);
    screenplay->setCurrentElementIndex(previousElementIndex);
    GarbageCollector::instance()->add(element);
    return previousSceneElement;
}

void Screenplay::removeSceneElements(Scene *scene)
{
    if(scene == nullptr)
        return;

    for(int i=m_elements.size()-1; i>=0; i--)
    {
        ScreenplayElement *ptr = m_elements.at(i);
        if(ptr->scene() == scene)
            this->removeElement(ptr);
    }
}

int Screenplay::firstIndexOfScene(Scene *scene) const
{
    const QList<int> indexes = this->sceneElementIndexes(scene, 1);
    return indexes.isEmpty() ? -1 : indexes.first();
}

int Screenplay::indexOfElement(ScreenplayElement *element) const
{
    return m_elements.indexOf(element);
}

QList<int> Screenplay::sceneElementIndexes(Scene *scene, int max) const
{
    QList<int> ret;
    if(scene == nullptr || max == 0)
        return ret;

    for(int i=0; i<m_elements.size(); i++)
    {
        ScreenplayElement *element = m_elements.at(i);
        if(element->scene() == scene)
        {
            ret << i;
            if(max > 0 && ret.size() == max)
                break;
        }
    }

    return ret;
}

QList<ScreenplayElement *> Screenplay::sceneElements(Scene *scene, int max) const
{
    const QList<int> indexes = this->sceneElementIndexes(scene, max);
    QList<ScreenplayElement*> elements;
    Q_FOREACH(int idx, indexes)
        elements << m_elements.at(idx);
    return elements;
}

int Screenplay::firstSceneIndex() const
{
    int index = 0;
    while(index < m_elements.size())
    {
        ScreenplayElement *element = m_elements.at(index);
        if(element->scene() != nullptr)
            return index;

        ++index;
    }

    return -1;
}

int Screenplay::lastSceneIndex() const
{
    int index = m_elements.size()-1;
    while(index >= 0)
    {
        ScreenplayElement *element = m_elements.at(index);
        if(element->scene() != nullptr)
            return index;

        --index;
    }

    return -1;
}

bool Screenplay::setElements(const QList<ScreenplayElement *> &list)
{
    // Works only if the elements in the list supplied as parameters already
    // is just a reordered list of elements already in the screenplay.
    if(list == m_elements)
        return true;

    QList<ScreenplayElement*> copy = m_elements;
    for(ScreenplayElement *element : list)
    {
        const int index = copy.isEmpty() ? -1 : copy.indexOf(element);
        if(index < 0)
            return false;
        copy.takeAt(index);
    }

    if(!copy.isEmpty())
        return false;

    this->beginResetModel();
    m_elements = list;
    this->endResetModel();

    emit elementsChanged();

    return true;
}

void Screenplay::addBreakElement(Screenplay::BreakType type)
{
    this->insertBreakElement(type, -1);
}

void Screenplay::insertBreakElement(Screenplay::BreakType type, int index)
{
    ScreenplayElement *element = new ScreenplayElement(this);
    element->setElementType(ScreenplayElement::BreakElementType);
    element->setBreakType(type);
    this->insertElementAt(element, index);
}

void Screenplay::setCurrentElementIndex(int val)
{
    val = qBound(-1, val, m_elements.size()-1);
    if(m_currentElementIndex == val)
        return;

    m_currentElementIndex = val;
    emit currentElementIndexChanged(m_currentElementIndex);

    if(m_currentElementIndex >= 0)
    {
        ScreenplayElement *element = m_elements.at(m_currentElementIndex);
        this->setActiveScene(element->scene());
    }
    else
        this->setActiveScene(nullptr);
}

int Screenplay::nextSceneElementIndex()
{
    int index = m_currentElementIndex+1;
    while(index < m_elements.size()-1)
    {
        ScreenplayElement *element = m_elements.at(index);
        if(element->elementType() == ScreenplayElement::BreakElementType)
        {
            ++index;
            continue;
        }

        break;
    }

    if(index < m_elements.size())
        return index;

    return m_elements.size()-1;
}

int Screenplay::previousSceneElementIndex()
{
    int index = m_currentElementIndex-1;
    while(index >= 0)
    {
        ScreenplayElement *element = m_elements.at(index);
        if(element->elementType() == ScreenplayElement::BreakElementType)
        {
            --index;
            continue;
        }

        break;
    }

    if(index >= 0)
        return index;

    return 0;
}

void Screenplay::setActiveScene(Scene *val)
{
    if(m_activeScene == val)
        return;

    // Ensure that the scene belongs to this screenplay.
    if(m_currentElementIndex >= 0)
    {
        ScreenplayElement *element = this->elementAt(m_currentElementIndex);
        if(element && element->scene() == val)
        {
            m_activeScene = val;
            emit activeSceneChanged();
            return;
        }
    }

    const int index = this->firstIndexOfScene(val);
    if(index < 0)
    {
        if(m_activeScene != nullptr)
        {
            m_activeScene = nullptr;
            emit activeSceneChanged();
        }
    }
    else
    {
        m_activeScene = val;
        emit activeSceneChanged();
    }

    this->setCurrentElementIndex(index);
}

QJsonArray Screenplay::search(const QString &text, int flags) const
{
    HourGlass hourGlass;

    QJsonArray ret;

    const int nrScenes = m_elements.size();
    for(int i=0; i<nrScenes; i++)
    {
        Scene *scene = m_elements.at(i)->scene();
        if(scene == nullptr)
            continue;

        int sceneResultIndex = 0;

        const int nrElements = scene->elementCount();
        for(int j=0; j<nrElements; j++)
        {
            SceneElement *element = scene->elementAt(j);

            const QJsonArray results = element->find(text, flags);
            if(!results.isEmpty())
            {
                for(int r=0; r<results.size(); r++)
                {
                    const QJsonObject result = results.at(r).toObject();

                    QJsonObject item;
                    item.insert("sceneIndex", i);
                    item.insert("elementIndex", j);
                    item.insert("sceneResultIndex", sceneResultIndex++);
                    item.insert("from", result.value("from"));
                    item.insert("to", result.value("to"));
                    ret.append(item);
                }
            }
        }
    }

    return ret;
}

int Screenplay::replace(const QString &text, const QString &replacementText, int flags)
{
    HourGlass hourGlass;

    int counter = 0;

    const int nrScenes = m_elements.size();
    for(int i=0; i<nrScenes; i++)
    {
        Scene *scene = m_elements.at(i)->scene();
        if(scene == nullptr)
            continue;

        bool begunUndoCapture = false;

        const int nrElements = scene->elementCount();
        for(int j=0; j<nrElements; j++)
        {
            SceneElement *element = scene->elementAt(j);
            const QJsonArray results = element->find(text, flags);
            counter += results.size();

            if(results.isEmpty())
                continue;

            if(!begunUndoCapture)
            {
                scene->beginUndoCapture();
                begunUndoCapture = true;
            }

            QString elementText = element->text();
            for(int r=results.size()-1; r>=0; r--)
            {
                const QJsonObject result = results.at(r).toObject();
                const int from = result.value("from").toInt();
                const int to = result.value("to").toInt();
                elementText = elementText.replace(from, to-from+1, replacementText);
            }

            element->setText(elementText);
        }

        if(begunUndoCapture)
            scene->endUndoCapture();
    }

    return counter;
}

void Screenplay::serializeToJson(QJsonObject &json) const
{
    json.insert("hasCoverPagePhoto", !m_coverPagePhoto.isEmpty());
}

void Screenplay::deserializeFromJson(const QJsonObject &)
{
    const QString cpPhotoPath = m_scriteDocument->fileSystem()->absolutePath(coverPagePhotoPath);
    if( QFile::exists(cpPhotoPath) )
    {
        m_coverPagePhoto = cpPhotoPath;
        emit coverPagePhotoChanged();
    }

    this->evaluateSceneNumbers();

    if(!m_scriteDocument->isCreatedOnThisComputer())
    {
        for(ScreenplayElement *element : m_elements)
            element->setEditorHints(QJsonValue());
    }
}

bool Screenplay::canSetPropertyFromObjectList(const QString &propName) const
{
    if(propName == QStringLiteral("elements"))
        return m_elements.isEmpty();

    return false;
}

void Screenplay::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if(propName == QStringLiteral("elements"))
    {
        const QList<ScreenplayElement*> list = qobject_list_cast<ScreenplayElement*>(objects);
        if(!m_elements.isEmpty() || list.isEmpty())
            return;

        this->beginResetModel();

        for(ScreenplayElement *ptr : list)
        {
            ptr->setParent(this);
            connect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
            connect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);
            connect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset);
            connect(ptr, &ScreenplayElement::evaluateSceneNumberRequest, this, &Screenplay::evaluateSceneNumbersLater);
            connect(ptr, &ScreenplayElement::sceneTypeChanged, this, &Screenplay::evaluateSceneNumbersLater);
            if(ptr->elementType() == ScreenplayElement::BreakElementType)
                connect(ptr, &ScreenplayElement::breakTitleChanged, this, &Screenplay::breakTitleChanged);

            m_elements.append(ptr);
        }

        this->endResetModel();

        emit elementCountChanged();
        emit elementsChanged();

        this->setCurrentElementIndex(0);

        return;
    }
}

int Screenplay::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elements.size();
}

QVariant Screenplay::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    ScreenplayElement *element = this->elementAt(index.row());
    switch(role)
    {
    case IdRole:
        return element->sceneID();
    case ScreenplayElementRole:
        return QVariant::fromValue<ScreenplayElement*>(element);
    case ScreenplayElementTypeRole:
        return element->elementType();
    case BreakTypeRole:
        return element->breakType();
    case SceneRole:
        return QVariant::fromValue<Scene*>(element->scene());
    case RowNumberRole:
        return index.row();
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> Screenplay::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if(roles.isEmpty())
    {
        roles[IdRole] = "id";
        roles[ScreenplayElementRole] = "screenplayElement";
        roles[ScreenplayElementTypeRole] = "screenplayElementType";
        roles[BreakTypeRole] = "breakType";
        roles[SceneRole] = "scene";
        roles[RowNumberRole] = "rowNumber";
    }

    return roles;
}

bool Screenplay::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_scriteDocument = qobject_cast<ScriteDocument*>(this->parent());

    return QObject::event(event);
}

void Screenplay::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_sceneNumberEvaluationTimer.timerId())
    {
        m_sceneNumberEvaluationTimer.stop();
        this->evaluateSceneNumbers();
    }
}

void Screenplay::resetActiveScene()
{
    m_activeScene = nullptr;
    emit activeSceneChanged();
    this->setCurrentElementIndex(-1);
}

void Screenplay::onSceneReset(int elementIndex)
{
    ScreenplayElement *element = qobject_cast<ScreenplayElement*>(this->sender());
    if(element == nullptr)
        return;

    int sceneIndex = this->indexOfElement(element);
    if(sceneIndex < 0)
        return;

    emit sceneReset(sceneIndex, elementIndex);
}

void Screenplay::evaluateSceneNumbers()
{
    int number = 1;
    bool containsNonStandardScenes = false;
    Q_FOREACH(ScreenplayElement *element, m_elements)
    {
        element->evaluateSceneNumber(number);
        if(!containsNonStandardScenes && element->scene() && element->scene()->type() != Scene::Standard)
            containsNonStandardScenes = true;
    }

    this->setHasNonStandardScenes(containsNonStandardScenes);
}

void Screenplay::evaluateSceneNumbersLater()
{
    m_sceneNumberEvaluationTimer.start(0, this);
}

void Screenplay::validateCurrentElementIndex()
{
    int val = m_currentElementIndex;
    if(m_elements.isEmpty())
        val = -1;
    else
        val = qBound(0, val, m_elements.size()-1);

    if(val >= 0)
    {
        Scene *currentScene = m_elements.at(val)->scene();
        if(m_activeScene != currentScene)
            m_currentElementIndex = -2;
    }

    this->setCurrentElementIndex(val);
}

void Screenplay::setHasNonStandardScenes(bool val)
{
    if(m_hasNonStandardScenes == val)
        return;

    m_hasNonStandardScenes = val;
    emit hasNonStandardScenesChanged();
}

void Screenplay::setHasTitlePageAttributes(bool val)
{
    if(m_hasTitlePageAttributes == val)
        return;

    m_hasTitlePageAttributes = val;
    emit hasTitlePageAttributesChanged();
}

void Screenplay::evaluateHasTitlePageAttributes()
{
    this->setHasTitlePageAttributes(
            !m_title.isEmpty() &&
            !m_author.isEmpty() &&
            !m_version.isEmpty()
                );
}

void Screenplay::staticAppendElement(QQmlListProperty<ScreenplayElement> *list, ScreenplayElement *ptr)
{
    reinterpret_cast< Screenplay* >(list->data)->addElement(ptr);
}

void Screenplay::staticClearElements(QQmlListProperty<ScreenplayElement> *list)
{
    reinterpret_cast< Screenplay* >(list->data)->clearElements();
}

ScreenplayElement *Screenplay::staticElementAt(QQmlListProperty<ScreenplayElement> *list, int index)
{
    return reinterpret_cast< Screenplay* >(list->data)->elementAt(index);
}

int Screenplay::staticElementCount(QQmlListProperty<ScreenplayElement> *list)
{
    return reinterpret_cast< Screenplay* >(list->data)->elementCount();
}

