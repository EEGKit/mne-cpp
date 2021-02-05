
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "event.h"
#include "eventgroup.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace EVENTSINTERNAL;

//=============================================================================================================
// INIT STATIC MEMBERS
//=============================================================================================================

int Event::eventIdCounter(0);

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

Event::Event(int iSample, const EventGroup& group)
: m_iSample(iSample)
, m_iGroup(group.getId())
{
    m_iId = eventIdCounter++;
}

//=============================================================================================================

int Event::getSample() const
{
    return m_iSample;
}

//=============================================================================================================

void Event::setSample(int iSample)
{
    m_iSample = iSample;
}

//=============================================================================================================

int Event::getType() const
{
    return m_iType;
}

//=============================================================================================================

void Event::setType(int iType)
{
    m_iType = iType;
}

//=============================================================================================================

int Event::getGroup() const
{
    return m_iGroup;
}

//=============================================================================================================

void Event::setGroup(int iGroup)
{
    m_iGroup = iGroup;
}

//=============================================================================================================

int Event::getId() const
{
    return m_iId;
}

//=============================================================================================================
bool Event::operator<(const Event& rhs) const
{
    bool isLessThan;
    if (m_iSample == rhs.getSample()){
        isLessThan = m_iId < rhs.getId();
    } else {
        isLessThan = m_iSample < rhs.getSample();
    }
    return isLessThan;
}
