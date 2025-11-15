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

#ifndef ATTACHMENTS_H
#define ATTACHMENTS_H

#include "qobjectserializer.h"
#include "qobjectlistmodel.h"

#include <QFileInfo>
#include <QMimeType>
#include <QQuickItem>
#include <QQmlEngine>

class Character;

class Attachment : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit Attachment(QObject *parent = nullptr);
    ~Attachment();
    Q_SIGNAL void aboutToDelete(Attachment *ptr);

    enum Type { Photo, Video, Audio, Document };
    Q_ENUM(Type)
    // clang-format off
    Q_PROPERTY(Type type
               READ type
               NOTIFY typeChanged)
    // clang-format on
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    // clang-format off
    Q_PROPERTY(bool featured
               READ isFeatured
               WRITE setFeatured
               NOTIFY featuredChanged)
    // clang-format on
    void setFeatured(bool val);
    bool isFeatured() const { return m_featured; }
    Q_SIGNAL void featuredChanged();

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
    Q_PROPERTY(QString originalFileName
               READ originalFileName
               NOTIFY originalFileNameChanged)
    // clang-format on
    QString originalFileName() const { return m_originalFileName; }
    Q_SIGNAL void originalFileNameChanged();

    // clang-format off
    Q_PROPERTY(QString filePath
               READ filePath
               NOTIFY filePathChanged)
    // clang-format on
    QString filePath() const { return m_filePath; }
    Q_SIGNAL void filePathChanged();

    // For use with Image {}, PdfViewer etc..
    // clang-format off
    Q_PROPERTY(QUrl fileSource
               READ fileSource
               NOTIFY filePathChanged)
    // clang-format on
    QUrl fileSource() const { return m_fileSource; }

    // clang-format off
    Q_PROPERTY(QString mimeType
               READ mimeType
               NOTIFY mimeTypeChanged)
    // clang-format on
    QString mimeType() const { return m_mimeType; }
    Q_SIGNAL void mimeTypeChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject userData
               READ userData
               WRITE setUserData
               NOTIFY userDataChanged)
    // clang-format on
    void setUserData(const QJsonObject &val);
    QJsonObject userData() const { return m_userData; }
    Q_SIGNAL void userDataChanged();

    Q_INVOKABLE void openAttachmentAnonymously();
    Q_INVOKABLE void openAttachmentInPlace();

    Q_SIGNAL void attachmentModified();

    bool isValid() const;

    bool isRemoveFileOnDelete() const { return m_removeFileOnDelete; }

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

    static Type determineType(const QFileInfo &fi);

private:
    friend class Attachments;
    friend class AttachmentsDropArea;
    void setType(Type val);
    void setFilePath(const QString &val);
    void setMimeType(const QString &val);
    void setOriginalFileName(const QString &val);
    bool removeAttachedFile();
    void onDfsAuction(const QString &filePath, int *claims);
    void setRemoveFileOnDelete(bool val) { m_removeFileOnDelete = val; }

private:
    QString m_name;
    QString m_title;
    QUrl m_fileSource;
    QString m_filePath;
    QString m_mimeType;
    QJsonObject m_userData;
    bool m_featured = false;
    QString m_originalFileName;
    Type m_type = Document;
    QString m_anonFilePath;
    bool m_removeFileOnDelete = false;
};

class Attachments : public QObjectListModel<Attachment *>, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit Attachments(QObject *parent = nullptr);
    ~Attachments();

    enum AllowedType {
        PhotosOnly = 1,
        VideosOnly = 2,
        AudioOnly = 4,
        MediaOnly = 7,
        NoMedia = 8,
        DocumentsOfAnyType = 15
    };
    Q_ENUM(AllowedType)
    // clang-format off
    Q_PROPERTY(AllowedType allowedType
               READ allowedType
               NOTIFY allowedTypeChanged
               STORED false)
    // clang-format on
    void setAllowedType(AllowedType val); // Can only be called from C++ classes.
    AllowedType allowedType() const { return m_allowedType; }
    Q_SIGNAL void allowedTypeChanged();

    static bool canAllow(const QFileInfo &fi, AllowedType allowed);
    static QMimeType mimeTypeFor(const QFileInfo &fi);

    // clang-format off
    Q_PROPERTY(QStringList nameFilters
               READ nameFilters
               NOTIFY allowedTypeChanged
               STORED false)
    // clang-format on
    QStringList nameFilters() const { return m_nameFilters; }
    Q_SIGNAL void nameFiltersChanged();

    // clang-format off
    Q_PROPERTY(Attachment *featuredAttachment
               READ featuredAttachment
               NOTIFY featuredAttachmentChanged
               STORED false)
    // clang-format on
    Attachment *featuredAttachment() const { return m_featuredAttachment; }
    Q_SIGNAL void featuredAttachmentChanged();

    Q_INVOKABLE Attachment *includeAttachmentFromFileUrl(const QUrl &fileUrl)
    {
        if (fileUrl.isLocalFile())
            return this->includeAttachment(fileUrl.toLocalFile());
        return nullptr;
    }
    Q_INVOKABLE Attachment *includeAttachment(const QString &filePath);
    Q_INVOKABLE void removeAttachment(Attachment *ptr);
    Q_INVOKABLE Attachment *attachmentAt(int index) const;
    Q_INVOKABLE Attachment *firstAttachment() const { return this->attachmentAt(0); }
    Q_INVOKABLE Attachment *lastAttachment() const
    {
        return this->attachmentAt(this->attachmentCount() - 1);
    }
    Q_INVOKABLE void removeAllAttachments();

    // clang-format off
    Q_PROPERTY(int attachmentCount
               READ attachmentCount
               NOTIFY attachmentCountChanged)
    // clang-format on
    int attachmentCount() const { return this->size(); }
    Q_SIGNAL void attachmentCountChanged();

    Q_SIGNAL void attachmentsModified();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &json);

private:
    void includeAttachment(Attachment *ptr);
    void attachmentDestroyed(Attachment *ptr);
    void includeAttachments(const QList<Attachment *> &list);
    void evaluateFeaturedAttachment();
    void evaluateFeaturedAttachmentLater();

    void moveAttachments(Attachments *target);

private:
    friend class Character;
    AllowedType m_allowedType = DocumentsOfAnyType;
    QStringList m_nameFilters;
    Attachment *m_featuredAttachment = nullptr;
    QTimer *m_evalutateFeaturedAttachmentTimer = nullptr;
};

class AttachmentsDropArea : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(BasicAttachmentsDropArea)

public:
    explicit AttachmentsDropArea(QQuickItem *parent = nullptr);
    ~AttachmentsDropArea();

    // clang-format off
    Q_PROPERTY(Attachments *target
               READ target
               WRITE setTarget
               NOTIFY targetChanged)
    // clang-format on
    void setTarget(Attachments *val);
    Attachments *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    // clang-format off
    Q_PROPERTY(int allowedType
               READ allowedType
               WRITE setAllowedType
               NOTIFY allowedTypeChanged)
    // clang-format on
    void setAllowedType(int val);
    int allowedType() const { return m_allowedType; }
    Q_SIGNAL void allowedTypeChanged();

    // clang-format off
    Q_PROPERTY(QStringList allowedExtensions
               READ allowedExtensions
               WRITE setAllowedExtensions
               NOTIFY allowedExtensionsChanged)
    // clang-format on
    void setAllowedExtensions(const QStringList &val);
    QStringList allowedExtensions() const { return m_allowedExtensions; }
    Q_SIGNAL void allowedExtensionsChanged();

    // clang-format off
    Q_PROPERTY(bool allowMultiple
               READ isAllowMultiple
               WRITE setAllowMultiple
               NOTIFY allowMultipleChanged)
    // clang-format on
    void setAllowMultiple(bool val);
    bool isAllowMultiple() const { return m_allowMultiple; }
    Q_SIGNAL void allowMultipleChanged();

    // clang-format off
    Q_PROPERTY(bool active
               READ isActive
               NOTIFY attachmentChanged)
    // clang-format on
    bool isActive() const { return m_attachment != nullptr; }

    // clang-format off
    Q_PROPERTY(Attachment *attachment
               READ attachment
               NOTIFY attachmentChanged)
    // clang-format on
    Attachment *attachment() const { return m_attachment; }
    Q_SIGNAL void attachmentChanged();

    // clang-format off
    Q_PROPERTY(QPointF mouse
               READ mouse
               NOTIFY mouseChanged)
    // clang-format on
    QPointF mouse() const { return m_mouse; }
    Q_SIGNAL void mouseChanged();

    // clang-format off
    Q_PROPERTY(QList<QUrl> dropUrls
               READ dropUrls
               NOTIFY dropUrlsChanged)
    // clang-format on
    QList<QUrl> dropUrls() const { return m_dropUrls; }
    Q_SIGNAL void dropUrlsChanged();

    Q_INVOKABLE void allowDrop();
    Q_INVOKABLE void denyDrop();

signals:
    void dropped();

protected:
    // QQuickItem interface
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dropEvent(QDropEvent *);

private:
    void resetAttachments();
    void setAttachment(Attachment *attachment, const QList<QUrl> &dropUrls = QList<QUrl>());
    void setMouse(const QPointF &val);
    bool prepareAttachmentFromMimeData(const QMimeData *mimeData);

private:
    bool m_active = false;
    QPointF m_mouse;
    bool m_allowDrop = true;
    int m_allowedType = 0;
    QStringList m_allowedExtensions;
    Attachment *m_attachment = nullptr;
    Attachments *m_target = nullptr;
    QList<QUrl> m_dropUrls;
    bool m_allowMultiple = false;
};

#endif // ATTACHMENTS_H
