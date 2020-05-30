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

#ifndef MODIFIABLE_H
#define MODIFIABLE_H

class Modifiable
{
public:
    void markAsModified() { ++m_modificationTime; }
    int modificationTime() const { return m_modificationTime; }
    bool isModified(int *time) const {
        const bool ret = m_modificationTime > *time;
        *time = m_modificationTime;
        return ret;
    }
    bool isModified(const int time) const { return m_modificationTime > time; }

private:
    int m_modificationTime = 0;
};

class ModificationTracker
{
public:
    ModificationTracker(Modifiable *target=nullptr) :
        m_target(target) {
        if(m_target)
            m_modificationTime = m_target->modificationTime();
    }
    ~ModificationTracker() { }

    ModificationTracker& operator = (const ModificationTracker &other) {
        m_target = other.m_target;
        m_modificationTime = other.m_modificationTime;
        return *this;
    }

    bool isTracking(Modifiable *target) const { return m_target == target; }

    void track(Modifiable *target) {
        if(m_target != target) {
            m_target = target;
            if(m_target)
                m_modificationTime = m_target->modificationTime();
        }
    }

    bool isModified() { return m_target ? m_target->isModified(&m_modificationTime) : false; }

    bool isModified(Modifiable *target) {
        if(this->isTracking(target))
            return this->isModified();
        track(target);
        return true;
    }

private:
    Modifiable *m_target;
    int m_modificationTime = 0;
};

#endif // MODIFIABLE_H
