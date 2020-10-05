#include "usagecomputer.h"

void UsageComputer::setTicks(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle)
{
    unsigned long long totalTicks = system + user + wait + idle;
    unsigned long long totalDiff = totalTicks - m_totalTicks;

    auto percentage =  [totalDiff] (unsigned long long tickDiff) {
        // according to the documentation some counters can go backwards in some circumstances
        return tickDiff > 0 ? 100.0 *  tickDiff / totalDiff : 0;
    };

    systemUsage = percentage(system - m_systemTicks);
    userUsage = percentage(user - m_userTicks);
    waitUsage = percentage(wait - m_waitTicks);
    totalUsage = percentage((system + user + wait) - (m_systemTicks + m_userTicks + m_waitTicks));

    m_totalTicks = totalTicks;
    m_systemTicks = system;
    m_userTicks = user;
    m_waitTicks = wait;
}
