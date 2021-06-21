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

#include "attachments.h"

#include "timeprofiler.h"
#include "scritedocument.h"
#include "documentfilesystem.h"

#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QMimeData>

Attachment::Attachment(QObject *parent)
    : QObject(parent)
{
    connect(this, &Attachment::nameChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::titleChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::filePathChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::mimeTypeChanged, this, &Attachment::attachmentModified);
}

Attachment::~Attachment()
{
    emit aboutToDelete(this);
}

void Attachment::setName(const QString &val)
{
    if(m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void Attachment::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Attachment::setOriginalFileName(const QString &val)
{
    if(m_originalFileName == val || !m_originalFileName.isEmpty())
        return;

    m_originalFileName = val;
    emit originalFileNameChanged();
}

void Attachment::setFilePath(const QString &val)
{
    if(m_filePath == val || !m_filePath.isEmpty())
        return;

    QFileInfo fi(val);
    if(!fi.exists() || !fi.isReadable())
        return;

    m_filePath = val;
    emit filePathChanged();
}

void Attachment::setMimeType(const QString &val)
{
    if(m_mimeType == val || !m_mimeType.isEmpty())
        return;

    m_mimeType = val;
    // m_type = ...
    // m_typeIcon = ...
    emit mimeTypeChanged();
}

bool Attachment::removeAttachedFile()
{
    if(m_filePath.isEmpty())
        return false;

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString path = dfs->absolutePath(m_filePath);

    if( QFile::remove(path) )
    {
        this->setFilePath( QString() );
        return true;
    }

    return false;
}

void Attachment::serializeToJson(QJsonObject &json) const
{
    json.insert( QStringLiteral("#filePath"), m_filePath );
    json.insert( QStringLiteral("#mimeType"), m_mimeType );
    json.insert( QStringLiteral("#originalFileName"), m_originalFileName );
}

void Attachment::deserializeFromJson(const QJsonObject &json)
{
    this->setFilePath( json.value( QStringLiteral("#filePath") ).toString() );
    this->setMimeType( json.value( QStringLiteral("#mimeType") ).toString() );
    this->setOriginalFileName( json.value( QStringLiteral("#originalFileName") ).toString() );
}

Attachment::Type Attachment::determineType(const QFileInfo &fi)
{
    const QChar comma(',');
    static const QStringList photoExtensions = QStringLiteral("jpg,bmp,png,jpeg,svg").split(comma, QString::SkipEmptyParts);
    static const QStringList videoExtensions = QStringLiteral("mp4,mov,avi,wmv").split(comma, QString::SkipEmptyParts);
    static const QStringList audioExtensions = QStringLiteral("mp3,wav,m4a,ogg,flac,aiff,au").split(QChar(','), QString::SkipEmptyParts);

    const QString suffix = fi.suffix().toLower();

    if(photoExtensions.contains(suffix))
        return Photo;

    if(videoExtensions.contains(suffix))
        return Video;

    if(audioExtensions.contains(suffix))
        return Audio;

    return Document;
}

void Attachment::setType(Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

///////////////////////////////////////////////////////////////////////////////

Attachments::Attachments(QObject *parent)
    : ObjectListPropertyModel<Attachment *>(parent)
{
    connect(this, &Attachments::rowsInserted, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::rowsRemoved, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::rowsMoved, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::dataChanged, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::objectCountChanged, this, &Attachments::attachmentCountChanged);
}

Attachments::~Attachments()
{

}

void Attachments::setAllowedType(AllowedType val)
{
    if(m_allowedType == val)
        return;

    m_allowedType = val;
    emit allowedTypeChanged();
}

bool Attachments::canAllow(const QFileInfo &fi, AllowedType allowed)
{
    if(!fi.exists() || !fi.isFile() || !fi.isReadable())
        return false;

    if(allowed == DocumentsOfAnyType)
        return true;

    Attachment::Type type = Attachment::determineType(fi);

    bool ret = false;
    if(allowed == NoMedia)
        ret = (type == Attachment::Photo || type == Attachment::Video || type == Attachment::Photo);
    else if(allowed & PhotosOnly)
        ret = type == Attachment::Photo;
    else if(allowed & AudioOnly)
        ret = type == Attachment::Audio;
    else if(allowed & VideosOnly)
        ret = type == Attachment::Video;

    return ret;
}

Attachment *Attachments::includeAttachment(const QString &filePath)
{
    QFileInfo fi(filePath);
    if( !fi.exists() || !fi.isReadable() )
        return nullptr;

    if(!Attachments::canAllow(fi, m_allowedType))
        return nullptr;

    static QMimeDatabase mimeDb;
    const QMimeType mimeType = mimeDb.mimeTypeForFile(fi);
    if(!mimeType.isValid())
        return nullptr;

    const QString ns = QStringLiteral("attachments");

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString attachedFilePath = dfs->add(filePath, ns);

    Attachment *ptr = new Attachment(this);
    ptr->setTitle(fi.baseName());
    ptr->setMimeType(mimeType.name());
    ptr->setFilePath(attachedFilePath);
    ptr->setOriginalFileName(fi.fileName());
    this->includeAttachment(ptr);

    return ptr;
}

void Attachments::removeAttachment(Attachment *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if(index < 0)
        return;

    if(ptr->removeAttachedFile())
        delete ptr;
}

Attachment *Attachments::attachmentAt(int index) const
{
    return this->at(index);
}

void Attachments::removeAllAttachments()
{
    QList<Attachment*> &list = this->list();
    while(!list.isEmpty())
        this->removeAttachment(list.last());
}

void Attachments::serializeToJson(QJsonObject &json) const
{
    if(this->isEmpty())
        return;

    QJsonArray jsAttachments;

    const QList<Attachment*> &list = this->list();
    for(Attachment *attachment : list)
    {
        QJsonObject jsAttachment = QObjectSerializer::toJson(attachment);
        jsAttachments.append(jsAttachment);
    }

    json.insert( QStringLiteral("#data"), jsAttachments );
}

void Attachments::deserializeFromJson(const QJsonObject &json)
{
    const QJsonArray jsAttachments = json.value( QStringLiteral("#data") ).toArray();
    if(jsAttachments.isEmpty())
        return;

    // Remove duplicate notes.
    QList<Attachment*> list;

    list.reserve(jsAttachments.size());
    for(const QJsonValue &jsItem : jsAttachments)
    {
        const QJsonObject jsAttachment = jsItem.toObject();
        Attachment *attachment = new Attachment(this);
        if( QObjectSerializer::fromJson(jsAttachment, attachment) )
            list.append(attachment);
        else
            delete attachment;
    }

    this->includeAttachments(list);
}

void Attachments::includeAttachment(Attachment *ptr)
{
    if(ptr == nullptr || this->indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Attachment::aboutToDelete, this, &Attachments::attachmentDestroyed);
    connect(ptr, &Attachment::attachmentModified, this, &Attachments::attachmentsModified);

    this->append(ptr);
}

void Attachments::attachmentDestroyed(Attachment *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if(index < 0)
        return;

    disconnect(ptr, &Attachment::aboutToDelete, this, &Attachments::attachmentDestroyed);
    disconnect(ptr, &Attachment::attachmentModified, this, &Attachments::attachmentsModified);

    this->removeAt(index);

    ptr->deleteLater();
}

void Attachments::includeAttachments(const QList<Attachment *> &list)
{
    if(!this->isEmpty())
        return;

    // Here we dont have to check whether attachments match a specific type, because
    // they would have already been matched during their original creation.

    for(Attachment *ptr : list)
    {
        ptr->setParent(this);
        connect(ptr, &Attachment::aboutToDelete, this, &Attachments::attachmentDestroyed);
        connect(ptr, &Attachment::attachmentModified, this, &Attachments::attachmentsModified);
    }

    this->assign(list);
}

///////////////////////////////////////////////////////////////////////////////

AttachmentsDropArea::AttachmentsDropArea(QQuickItem *parent)
                    :QQuickItem(parent)
{
    this->setFlag(QQuickItem::ItemHasContents, true);
    this->setAcceptHoverEvents(false);
    this->setAcceptedMouseButtons(Qt::NoButton);
    this->setFlag(QQuickItem::ItemAcceptsDrops, true);
}

AttachmentsDropArea::~AttachmentsDropArea()
{

}

void AttachmentsDropArea::setTarget(Attachments *val)
{
    if(m_target == val)
        return;

    m_target = val;
    emit targetChanged();

    if(m_target != nullptr)
        this->setAllowedType(m_target->allowedType());
    else
        this->setAllowedType(Attachments::DocumentsOfAnyType);
}

void AttachmentsDropArea::setAllowedType(int val)
{
    if(m_target != nullptr)
        val = m_target->allowedType();

    if(m_allowedType == val)
        return;

    m_allowedType = val;
    emit allowedTypeChanged();
}

void AttachmentsDropArea::allowDrop()
{
    m_allowDrop = false;
}

void AttachmentsDropArea::denyDrop()
{
    m_allowDrop = false;
}

void AttachmentsDropArea::dragEnterEvent(QDragEnterEvent *de)
{
    this->setMouse(de->posF());
    if(de->proposedAction() != Qt::CopyAction)
        return;

    const QMimeData *mimeData = de->mimeData();
    if( this->prepareAttachmentFromMimeData(mimeData) )
        de->acceptProposedAction();
}

void AttachmentsDropArea::dragMoveEvent(QDragMoveEvent *de)
{
    this->setMouse(de->posF());
    if(de->proposedAction() != Qt::CopyAction)
        return;

    if(m_attachment != nullptr)
        de->acceptProposedAction();
}

void AttachmentsDropArea::dragLeaveEvent(QDragLeaveEvent *)
{
    this->setMouse(QPointF());

    if(m_attachment != nullptr)
    {
        Attachment *ptr = m_attachment;
        this->setAttachment(nullptr);
        if(ptr != nullptr)
            ptr->deleteLater();
    }
}

void AttachmentsDropArea::dropEvent(QDropEvent *de)
{
    if(m_attachment != nullptr)
    {
        m_allowDrop = true;

        emit dropped();

        if(m_allowDrop)
        {
            de->acceptProposedAction();

            if(m_target != nullptr)
            {
                m_target->includeAttachment( m_attachment->filePath() );
                m_attachment->deleteLater();
                this->setAttachment(nullptr);
            }
        }
    }
}

void AttachmentsDropArea::setAttachment(Attachment *val)
{
    if(m_attachment == val)
        return;

    m_attachment = val;
    emit attachmentChanged();
}

void AttachmentsDropArea::setMouse(const QPointF &val)
{
    if(m_mouse == val)
        return;

    m_mouse = val;
    emit mouseChanged();
}

bool AttachmentsDropArea::prepareAttachmentFromMimeData(const QMimeData *mimeData)
{

}
