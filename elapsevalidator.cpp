#include "elapsevalidator.h"

ElapseValidator::ElapseValidator()
    : m_maxCounter(10000),
      m_cancelled(false),
      m_limitReached(false),
      m_counter(0)
{
    m_incrementor = [=](int incr)
    {
        long curCounter = m_counter.fetch_add(incr) + incr;
        if(curCounter >= m_maxCounter)
        {
            m_limitReached = true;
        }
        return curCounter + incr;
    };
}
ElapseValidator::~ElapseValidator()
{
//    qDebug() << "in ElapseValidator-DEstructor";
    m_incrementor = nullptr;
}

void ElapseValidator::addElements(long elmnts)
{
    m_incrementor(elmnts);
}

bool ElapseValidator::aborted() const
{
    return m_limitReached.load() || m_cancelled.load();
}

bool ElapseValidator::limitReached() const
{
    return m_limitReached.load();
}
bool ElapseValidator::cancelled() const
{
    return m_cancelled.load();
}
void ElapseValidator::setCancelled()
{
    m_cancelled = true;
}

void ElapseValidator::setMaximumCounter(int maxCoutr)
{
    m_maxCounter = maxCoutr;
}
