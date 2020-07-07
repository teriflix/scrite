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

#include "screenplaysubsetreport.h"

ScreenplaySubsetReport::ScreenplaySubsetReport(QObject *parent)
    : AbstractScreenplaySubsetReport(parent)
{

}

ScreenplaySubsetReport::~ScreenplaySubsetReport()
{

}

void ScreenplaySubsetReport::setSceneNumbers(const QList<int> &val)
{
    if(m_sceneNumbers == val)
        return;

    m_sceneNumbers = val;
    std::sort(m_sceneNumbers.begin(), m_sceneNumbers.end());
    emit sceneNumbersChanged();
}

bool ScreenplaySubsetReport::includeScreenplayElement(const ScreenplayElement *element) const
{
    if(element->scene() == nullptr)
        return false;

    return m_sceneNumbers.contains(element->sceneNumber());
}

QString ScreenplaySubsetReport::screenplaySubtitle() const
{
    if(m_sceneNumbers.isEmpty())
        return QStringLiteral("All scenes of the screenplay.");

    return QStringLiteral("Snapshot of ") + QString::number(m_sceneNumbers.size()) + QStringLiteral(" scene(s).");
}

void ScreenplaySubsetReport::configureScreenplayTextDocument(ScreenplayTextDocument &stDoc)
{
    Q_UNUSED(stDoc);
}
