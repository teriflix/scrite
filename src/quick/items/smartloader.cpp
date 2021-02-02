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

#include "smartloader.h"

#include <QQmlContext>
#include <QQmlEngine>

SmartLoader::SmartLoader(QQuickItem *parent)
    : QQuickItem(parent)
{

}

SmartLoader::~SmartLoader()
{

}

void SmartLoader::setWidthPolicy(SmartLoader::SizePolicy val)
{
    if(m_widthPolicy == val)
        return;

    m_widthPolicy = val;
    this->sizeItem();
    emit widthPolicyChanged();
}

void SmartLoader::setHeightPolicy(SmartLoader::SizePolicy val)
{
    if(m_heightPolicy == val)
        return;

    m_heightPolicy = val;
    this->sizeItem();
    emit heightPolicyChanged();
}

void SmartLoader::setActive(bool val)
{
    if(m_active == val)
        return;

    m_active = val;

    if(val)
        this->loadItem();
    else
        this->unloadItem();

    emit activeChanged();
}

void SmartLoader::setSourceComponent(QQmlComponent *val)
{
    if(m_sourceComponent == val)
        return;

    if(m_sourceComponent != nullptr)
        this->unloadItem();

    m_sourceComponent = val;

    if(m_sourceComponent != nullptr && m_active)
        this->loadItem();

    emit sourceComponentChanged();
}

void SmartLoader::loadItem()
{
    if(m_sourceComponent == nullptr)
        return;

    QQmlContext *myContext = QQmlEngine::contextForObject(this);
    QQmlContext *itemContext = new QQmlContext(myContext);
    QObject *itemObject = m_sourceComponent->create(itemContext);
    if(itemObject == nullptr)
    {
        delete itemContext;
        return;
    }

    itemContext->setParent(itemObject);

    m_item = qobject_cast<QQuickItem*>(itemObject);
    if(m_item == nullptr)
    {
        delete m_item;
        return;
    }

    m_item->setParentItem(this);
    m_item->setX(0);
    m_item->setY(0);

    this->sizeItem();

    connect(m_item, &SmartLoader::widthChanged, this, &SmartLoader::updateWidth);
    connect(m_item, &SmartLoader::heightChanged, this, &SmartLoader::updateHeight);

    emit itemChanged();
}

void SmartLoader::unloadItem()
{
    if(m_item != nullptr)
    {
        if(m_widthPolicy == PreferItemValue)
            this->setWidth(m_item->width());
        if(m_heightPolicy == PreferItemValue)
            this->setHeight(m_item->height());

        delete m_item;
        m_item = nullptr;
        emit itemChanged();
    }
}

void SmartLoader::sizeItem()
{
    if(m_item == nullptr)
        return;

    switch(m_widthPolicy)
    {
    case PreferItemValue:
        if(m_item->width() > 0) {
            if( !qFuzzyCompare(m_item->width(), this->width()) ) {
                this->setWidth(m_item->width());
                emit widthUpdated();
            }
        } else
            m_item->setWidth(this->width());
        break;
    case PreferLoaderValue:
        if(this->width() > 0)
            m_item->setWidth(this->width());
        else if( !qFuzzyCompare(m_item->width(), this->width()) ) {
            this->setWidth(m_item->width());
            emit widthUpdated();
        }
        break;
    case PreferHigherValue: {
        const qreal max = qMax(m_item->width(), this->width());
        m_item->setWidth(max);
        if( !qFuzzyCompare(max, m_item->width()) ) {
            this->setWidth(max);
            emit widthUpdated();
        }
        } break;
    }

    switch(m_heightPolicy)
    {
    case PreferItemValue:
        if(m_item->height() > 0) {
            if( !qFuzzyCompare(this->height(), m_item->height()) ) {
                this->setHeight(m_item->height());
                emit heightUpdated();
            }
        } else
            m_item->setHeight(this->height());
        break;
    case PreferLoaderValue:
        if(this->height() > 0)
            m_item->setHeight(this->height());
        else if( !qFuzzyCompare(this->height(), m_item->height()) ) {
            this->setHeight(m_item->height());
            emit heightUpdated();
        }
        break;
    case PreferHigherValue: {
        const qreal max = qMax(m_item->height(), this->height());
        m_item->setHeight(max);
        if( !qFuzzyCompare(max, this->height()) ) {
            this->setHeight(max);
            emit heightUpdated();
        }
        } break;
    }
}

void SmartLoader::updateWidth()
{
    if(m_item != nullptr && m_widthPolicy == PreferItemValue)
    {
        if( !qFuzzyCompare(m_item->width(), this->width()) )
        {
            this->setWidth(m_item->width());
            emit widthUpdated();
        }
    }
}

void SmartLoader::updateHeight()
{
    if(m_item != nullptr && m_heightPolicy == PreferItemValue)
    {
        if( !qFuzzyCompare(this->height(), m_item->height()) )
        {
            this->setHeight(m_item->height());
            emit heightUpdated();
        }
    }
}

