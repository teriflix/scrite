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

#include "attachments.h"

#include "application.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "documentfilesystem.h"

#include <QFileInfo>
#include <QMimeType>
#include <QMimeData>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QMimeDatabase>
#include <QDesktopServices>

static QStringList supportedExtensions(Attachment::Type type)
{
    const QChar comma(',');
    static const QStringList photoExtensions =
            QStringLiteral("jpg,bmp,png,jpeg,svg").split(comma, Qt::SkipEmptyParts);
    static const QStringList videoExtensions =
            QStringLiteral("mp4,mov,avi,wmv,m4v,mpg,mpeg").split(comma, Qt::SkipEmptyParts);
    static const QStringList audioExtensions =
            QStringLiteral("mp3,wav,m4a,ogg,flac,aiff,au").split(QChar(','), Qt::SkipEmptyParts);
    static const QStringList documentExtensions =
            QStringLiteral(
                    "pdf,txt,zip,docx,xlsx,doc,xls,ppt,pptx,odt,odp,ods,ics,ical,scrite,fdx,xml")
                    .split(QChar(','), Qt::SkipEmptyParts);
    switch (type) {
    case Attachment::Audio:
        return audioExtensions;
    case Attachment::Video:
        return videoExtensions;
    case Attachment::Photo:
        return photoExtensions;
    case Attachment::Document:
        return documentExtensions;
    }

    return audioExtensions + videoExtensions + photoExtensions + documentExtensions;
}

Attachment::Attachment(QObject *parent) : QObject(parent)
{
    connect(this, &Attachment::titleChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::filePathChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::mimeTypeChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::featuredChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::originalFileNameChanged, this, &Attachment::attachmentModified);

    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    connect(dfs, &DocumentFileSystem::auction, this, &Attachment::onDfsAuction);
}

Attachment::~Attachment()
{
    if (!m_anonFilePath.isEmpty() && QFile::exists(m_anonFilePath))
        QFile::remove(m_anonFilePath);
    m_anonFilePath = QString();

    if (m_removeFileOnDelete)
        this->removeAttachedFile();

    emit aboutToDelete(this);
}

void Attachment::setFeatured(bool val)
{
    if (m_featured == val)
        return;

    m_featured = val;
    emit featuredChanged();
}

void Attachment::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Attachment::setUserData(const QJsonObject &val)
{
    if (m_userData == val)
        return;

    m_userData = val;
    emit userDataChanged();
}

void Attachment::openAttachmentAnonymously()
{
    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString path = dfs->absolutePath(m_filePath);

    if (!m_anonFilePath.isEmpty()) {
        if (QDesktopServices::openUrl(QUrl::fromLocalFile(m_anonFilePath)))
            return;

        QFile::remove(m_anonFilePath);
        m_anonFilePath = QString();
    }

    if (m_anonFilePath.isEmpty()) {
        static QTemporaryDir tempDir;
        QString anonPath = QDir::cleanPath(tempDir.filePath(m_originalFileName));

        int index = 0;
        while (1) {
            QFileInfo fi(anonPath);
            if (fi.exists())
                anonPath = fi.absolutePath() + QStringLiteral("/") + fi.completeBaseName()
                        + QString::number(index++) + QStringLiteral(".") + fi.suffix();
            else
                break;
        }

        m_anonFilePath = anonPath;
        if (QFile::copy(path, m_anonFilePath)
            && QDesktopServices::openUrl(QUrl::fromLocalFile(m_anonFilePath)))
            return;

        m_anonFilePath = QString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void Attachment::openAttachmentInPlace()
{
    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString path = dfs->absolutePath(m_filePath);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool Attachment::isValid() const
{
    return !(m_filePath.isEmpty() || m_title.isEmpty() || m_originalFileName.isEmpty()
             || m_mimeType.isEmpty());
}

void Attachment::setOriginalFileName(const QString &val)
{
    if (m_originalFileName == val || !m_originalFileName.isEmpty())
        return;

    m_originalFileName = val;
    emit originalFileNameChanged();
}

void Attachment::setFilePath(const QString &val)
{
    if (m_filePath == val || !m_filePath.isEmpty())
        return;

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString path = QFile::exists(val) ? val : dfs->absolutePath(val);
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable())
        return;

    m_filePath = val;
    m_fileSource = QUrl::fromLocalFile(path);
    emit filePathChanged();
}

void Attachment::setMimeType(const QString &val)
{
    if (m_mimeType == val || !m_mimeType.isEmpty())
        return;

    m_mimeType = val;
    emit mimeTypeChanged();
}

bool Attachment::removeAttachedFile()
{
    if (m_filePath.isEmpty())
        return false;

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString path = dfs->absolutePath(m_filePath);

    if (QFile::remove(path)) {
        this->setFilePath(QString());
        return true;
    }

    return false;
}

void Attachment::onDfsAuction(const QString &filePath, int *claims)
{
    if (m_filePath == filePath)
        *claims = *claims + 1;
}

void Attachment::serializeToJson(QJsonObject &json) const
{
    json.insert(QStringLiteral("#filePath"), m_filePath);
    json.insert(QStringLiteral("#mimeType"), m_mimeType);
    json.insert(QStringLiteral("#originalFileName"), m_originalFileName);
}

void Attachment::deserializeFromJson(const QJsonObject &json)
{
    this->setFilePath(json.value(QStringLiteral("#filePath")).toString());
    this->setMimeType(json.value(QStringLiteral("#mimeType")).toString());
    this->setOriginalFileName(json.value(QStringLiteral("#originalFileName")).toString());

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString path = dfs->absolutePath(m_filePath);
    this->setType(Attachment::determineType(QFileInfo(path)));
}

Attachment::Type Attachment::determineType(const QFileInfo &fi)
{
    const QStringList photoExtensions = ::supportedExtensions(Attachment::Photo);
    const QStringList videoExtensions = ::supportedExtensions(Attachment::Video);
    const QStringList audioExtensions = ::supportedExtensions(Attachment::Audio);

    const QString suffix = fi.suffix().toLower();

    if (photoExtensions.contains(suffix))
        return Photo;

    if (videoExtensions.contains(suffix))
        return Video;

    if (audioExtensions.contains(suffix))
        return Audio;

    return Document;
}

void Attachment::setType(Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

///////////////////////////////////////////////////////////////////////////////

Attachments::Attachments(QObject *parent) : QObjectListModel<Attachment *>(parent)
{
    connect(this, &Attachments::rowsMoved, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::dataChanged, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::objectCountChanged, this, &Attachments::attachmentsModified);
    connect(this, &Attachments::objectCountChanged, this, &Attachments::attachmentCountChanged);
    connect(this, &Attachments::attachmentsModified, this,
            &Attachments::evaluateFeaturedAttachmentLater);
}

Attachments::~Attachments() { }

void Attachments::setAllowedType(AllowedType val)
{
    if (m_allowedType == val)
        return;

    auto toFilter = [](const QString &type, const QStringList &extensions) {
        QString filter;
        for (const QString &ext : extensions)
            filter += QStringLiteral(" *.") + ext;
        return type + QStringLiteral(" (") + filter + QStringLiteral(" )");
    };

    m_nameFilters.clear();
    if (val & PhotosOnly)
        m_nameFilters << toFilter(QStringLiteral("Photos"),
                                  ::supportedExtensions(Attachment::Photo));
    if (val & VideosOnly)
        m_nameFilters << toFilter(QStringLiteral("Videos"),
                                  ::supportedExtensions(Attachment::Video));
    if (val & AudioOnly)
        m_nameFilters << toFilter(QStringLiteral("Audios"),
                                  ::supportedExtensions(Attachment::Video));
    if (val & VideosOnly)
        m_nameFilters << toFilter(QStringLiteral("Videos"),
                                  ::supportedExtensions(Attachment::Video));
    if (val & NoMedia)
        m_nameFilters << toFilter(QStringLiteral("Documents"),
                                  ::supportedExtensions(Attachment::Document));

    m_allowedType = val;
    emit allowedTypeChanged();
}

bool Attachments::canAllow(const QFileInfo &fi, AllowedType allowed)
{
    if (!fi.exists() || !fi.isFile() || !fi.isReadable())
        return false;

    if (allowed == DocumentsOfAnyType)
        return true;

    Attachment::Type type = Attachment::determineType(fi);

    bool ret = false;
    if (allowed == NoMedia)
        ret = !(type == Attachment::Photo || type == Attachment::Video
                || type == Attachment::Audio);
    else if (allowed & PhotosOnly)
        ret = type == Attachment::Photo;
    else if (allowed & AudioOnly)
        ret = type == Attachment::Audio;
    else if (allowed & VideosOnly)
        ret = type == Attachment::Video;

    return ret;
}

QMimeType Attachments::mimeTypeFor(const QFileInfo &fi)
{
    static QMimeDatabase mimeDb;
    const QMimeType mimeType = mimeDb.mimeTypeForFile(fi);
    return mimeType;
}

Attachment *Attachments::includeAttachment(const QString &filePath)
{
    if (filePath.isEmpty())
        return nullptr;

    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isReadable())
        return nullptr;

    if (!Attachments::canAllow(fi, m_allowedType))
        return nullptr;

    const QMimeType mimeType = Attachments::mimeTypeFor(fi);
    if (!mimeType.isValid())
        return nullptr;

    const QString ns = QStringLiteral("attachments");

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString attachedFilePath = dfs->add(filePath, ns);

    Attachment *ptr = new Attachment(this);
    ptr->setTitle(fi.completeBaseName());
    ptr->setMimeType(mimeType.name());
    ptr->setFilePath(attachedFilePath);
    ptr->setOriginalFileName(fi.fileName());
    ptr->setType(Attachment::determineType(fi));
    this->includeAttachment(ptr);

    return ptr;
}

void Attachments::removeAttachment(Attachment *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if (index < 0)
        return;

    if (ptr->removeAttachedFile())
        delete ptr;
}

Attachment *Attachments::attachmentAt(int index) const
{
    return this->at(index);
}

void Attachments::removeAllAttachments()
{
    QList<Attachment *> &list = this->list();
    while (!list.isEmpty())
        this->removeAttachment(list.last());
}

void Attachments::serializeToJson(QJsonObject &json) const
{
    if (this->isEmpty())
        return;

    QJsonArray jsAttachments;

    const QList<Attachment *> &list = this->list();
    for (Attachment *attachment : list) {
        QJsonObject jsAttachment = QObjectSerializer::toJson(attachment);
        jsAttachments.append(jsAttachment);
    }

    json.insert(QStringLiteral("#data"), jsAttachments);
}

void Attachments::deserializeFromJson(const QJsonObject &json)
{
    const QJsonArray jsAttachments = json.value(QStringLiteral("#data")).toArray();
    if (jsAttachments.isEmpty())
        return;

    // Remove duplicate notes.
    QList<Attachment *> list;

    list.reserve(jsAttachments.size());
    for (const QJsonValue &jsItem : jsAttachments) {
        const QJsonObject jsAttachment = jsItem.toObject();
        Attachment *attachment = new Attachment(this);
        if (QObjectSerializer::fromJson(jsAttachment, attachment))
            list.append(attachment);
        else
            delete attachment;
    }

    this->includeAttachments(list);
}

void Attachments::includeAttachment(Attachment *ptr)
{
    if (ptr == nullptr || this->indexOf(ptr) >= 0)
        return;

    if (!ptr->isValid()) {
        ptr->deleteLater();
        return;
    }

    ptr->setParent(this);

    connect(ptr, &Attachment::aboutToDelete, this, &Attachments::attachmentDestroyed);
    connect(ptr, &Attachment::attachmentModified, this, &Attachments::attachmentsModified);
    this->evaluateFeaturedAttachmentLater();

    this->append(ptr);
}

void Attachments::attachmentDestroyed(Attachment *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if (index < 0)
        return;

    disconnect(ptr, &Attachment::aboutToDelete, this, &Attachments::attachmentDestroyed);
    disconnect(ptr, &Attachment::attachmentModified, this, &Attachments::attachmentsModified);
    this->removeAt(index);

    if (m_featuredAttachment == ptr) {
        m_featuredAttachment = nullptr;
        emit featuredAttachmentChanged();
    }

    ptr->deleteLater();

    this->evaluateFeaturedAttachmentLater();
}

void Attachments::includeAttachments(const QList<Attachment *> &list)
{
    if (!this->isEmpty())
        return;

    // Here we dont have to check whether attachments match a specific type, because
    // they would have already been matched during their original creation.
    for (Attachment *ptr : list) {
        ptr->setParent(this);
        connect(ptr, &Attachment::aboutToDelete, this, &Attachments::attachmentDestroyed);
        connect(ptr, &Attachment::attachmentModified, this, &Attachments::attachmentsModified);
    }

    this->assign(list);
    this->evaluateFeaturedAttachmentLater();
}

void Attachments::evaluateFeaturedAttachment()
{
    const QList<Attachment *> &_list = this->list();

    Attachment *fattachment = [_list]() -> Attachment * {
        for (int i = _list.size() - 1; i >= 0; i--) {
            Attachment *ptr = _list.at(i);
            if (ptr->isFeatured())
                return ptr;
        }
        return nullptr;
    }();

    if (fattachment != m_featuredAttachment) {
        m_featuredAttachment = fattachment;
        emit featuredAttachmentChanged();
    }
}

void Attachments::evaluateFeaturedAttachmentLater()
{
    if (m_evalutateFeaturedAttachmentTimer == nullptr) {
        m_evalutateFeaturedAttachmentTimer = new QTimer(this);
        m_evalutateFeaturedAttachmentTimer->setSingleShot(true);
        m_evalutateFeaturedAttachmentTimer->setInterval(0);
        connect(m_evalutateFeaturedAttachmentTimer, &QTimer::timeout, this,
                &Attachments::evaluateFeaturedAttachment);
    }

    m_evalutateFeaturedAttachmentTimer->start();
}

void Attachments::moveAttachments(Attachments *target)
{
    if (this->isEmpty() || target == nullptr)
        return;

    QList<Attachment *> list = this->list();
    while (!list.isEmpty())
        target->includeAttachment(list.takeFirst());
}

///////////////////////////////////////////////////////////////////////////////

AttachmentsDropArea::AttachmentsDropArea(QQuickItem *parent) : QQuickItem(parent)
{
    this->setFlag(QQuickItem::ItemHasContents, true);
    this->setAcceptHoverEvents(false);
    this->setAcceptedMouseButtons(Qt::NoButton);
    this->setFlag(QQuickItem::ItemAcceptsDrops, true);
}

AttachmentsDropArea::~AttachmentsDropArea() { }

void AttachmentsDropArea::setTarget(Attachments *val)
{
    if (m_target == val)
        return;

    m_target = val;
    emit targetChanged();

    if (m_target != nullptr)
        this->setAllowedType(m_target->allowedType());
    else
        this->setAllowedType(Attachments::DocumentsOfAnyType);
}

void AttachmentsDropArea::setAllowedType(int val)
{
    if (m_target != nullptr)
        val = m_target->allowedType();

    if (m_allowedType == val)
        return;

    m_allowedType = val;
    emit allowedTypeChanged();
}

void AttachmentsDropArea::setAllowedExtensions(const QStringList &val)
{
    if (m_allowedExtensions == val)
        return;

    m_allowedExtensions = val;
    emit allowedExtensionsChanged();
}

void AttachmentsDropArea::setAllowMultiple(bool val)
{
    if (m_allowMultiple == val)
        return;

    m_allowMultiple = val;
    emit allowMultipleChanged();
}

void AttachmentsDropArea::allowDrop()
{
    m_allowDrop = true;
}

void AttachmentsDropArea::denyDrop()
{
    m_allowDrop = false;
}

void AttachmentsDropArea::dragEnterEvent(QDragEnterEvent *de)
{
    this->setMouse(de->posF());

    const QMimeData *mimeData = de->mimeData();
    if (de->proposedAction() == Qt::CopyAction && this->prepareAttachmentFromMimeData(mimeData)) {
        de->setDropAction(Qt::CopyAction);
        de->acceptProposedAction();
    } else
        de->ignore();
}

void AttachmentsDropArea::dragMoveEvent(QDragMoveEvent *de)
{
    this->setMouse(de->posF());

    if (de->proposedAction() == Qt::CopyAction && m_attachment != nullptr) {
        de->setDropAction(Qt::CopyAction);
        de->acceptProposedAction();
    } else
        de->ignore();
}

void AttachmentsDropArea::dragLeaveEvent(QDragLeaveEvent *)
{
    this->setMouse(QPointF());

    if (m_attachment != nullptr) {
        Attachment *ptr = m_attachment;
        this->setAttachment(nullptr);
        if (ptr != nullptr)
            ptr->deleteLater();
    }
}

void AttachmentsDropArea::dropEvent(QDropEvent *de)
{
    if (m_attachment != nullptr) {
        QQuickWindow *qmlWindow = this->window();
        if (qmlWindow) {
            qmlWindow->requestActivate();
            qmlWindow->raise();
        }

        this->setMouse(de->posF());
        m_allowDrop = true;

        emit dropped();

        if (m_allowDrop) {
            de->setDropAction(Qt::CopyAction);
            de->acceptProposedAction();

            if (m_target != nullptr) {
                for (int i = 0; i < m_dropUrls.size(); i++) {
                    const QUrl dropUrl = m_dropUrls.at(i);
                    const QFileInfo fi(dropUrl.toLocalFile());
                    Attachment *attachment = m_target->includeAttachment(fi.absoluteFilePath());
                    attachment->setFeatured(i == 0 || m_attachment->isFeatured());
                }
            }
        }

        m_attachment->deleteLater();
        this->setAttachment(nullptr);
    }
}

void AttachmentsDropArea::setAttachment(Attachment *attachment, const QList<QUrl> &dropUrls)
{
    if (m_attachment != attachment) {
        if (m_attachment != nullptr && m_attachment->parent() == this)
            m_attachment->deleteLater();

        m_attachment = attachment;
        emit attachmentChanged();
    }

    if (m_dropUrls != dropUrls) {
        m_dropUrls = dropUrls;
        emit dropUrlsChanged();
    }
}

void AttachmentsDropArea::setMouse(const QPointF &val)
{
    if (m_mouse == val)
        return;

    m_mouse = val;
    emit mouseChanged();
}

bool AttachmentsDropArea::prepareAttachmentFromMimeData(const QMimeData *mimeData)
{
    const QList<QUrl> urls = mimeData->urls();
    if (urls.isEmpty()) {
        this->setAttachment(nullptr);
        return false;
    }

    QList<QUrl> dropUrls;
    std::copy_if(urls.begin(), urls.end(), std::back_inserter(dropUrls), [=](const QUrl &url) {
        if (url.isValid() && url.scheme() == QStringLiteral("file")) {
            const QFileInfo fi(url.toLocalFile());
            const QMimeType mimeType = Attachments::mimeTypeFor(fi);
            return fi.exists() && fi.isReadable() && !fi.isDir()
                    && Attachments::canAllow(fi, Attachments::AllowedType(m_allowedType))
                    && mimeType.isValid()
                    && (m_allowedExtensions.isEmpty()
                                ? true
                                : m_allowedExtensions.contains(fi.suffix(), Qt::CaseInsensitive));
        }
        return false;
    });

    if (!m_allowMultiple) {
        if (!dropUrls.isEmpty())
            dropUrls = QList<QUrl>({ dropUrls.first() });
    }

    if (dropUrls.isEmpty()) {
        this->setAttachment(nullptr);
        return false;
    }

    const QUrl firstUrl = dropUrls.first();
    const QString firstFilePath = firstUrl.toLocalFile();
    const QFileInfo firstFileInfo(firstFilePath);
    const QMimeType firstFileMimeType = Attachments::mimeTypeFor(firstFileInfo);

    Attachment *ptr = new Attachment(this);
    if (dropUrls.size() > 1)
        ptr->setTitle(firstFileInfo.fileName()
                      + QStringLiteral(" and %1 more files").arg(dropUrls.size() - 1));
    else
        ptr->setTitle(firstFileInfo.fileName());
    ptr->setFilePath(firstFileInfo.absoluteFilePath());
    ptr->setMimeType(firstFileMimeType.name());
    ptr->setOriginalFileName(firstFileInfo.fileName());
    ptr->setType(Attachment::determineType(firstFileInfo));
    ptr->setRemoveFileOnDelete(false);

    this->setAttachment(ptr, dropUrls);

    return true;
}
