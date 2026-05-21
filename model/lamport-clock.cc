/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "lamport-clock.h"

#include "ns3/log.h"

#include <algorithm>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LamportClock");

NS_OBJECT_ENSURE_REGISTERED(LamportClock);

TypeId
LamportClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LamportClock")
                            .SetParent<LocalClock>()
                            .SetGroupName("Network")
                            .AddConstructor<LamportClock>();
    return tid;
}

LamportClock::LamportClock()
    : m_counter(0)
{
    NS_LOG_FUNCTION(this);
}

LamportClock::~LamportClock()
{
    NS_LOG_FUNCTION(this);
}

Time
LamportClock::Now()
{
    NS_LOG_FUNCTION(this);
    return NanoSeconds(m_counter);
}

uint64_t
LamportClock::Tick()
{
    NS_LOG_FUNCTION(this);
    m_counter++;
    NS_LOG_INFO("Lamport clock ticked. New value: " << m_counter);
    return m_counter;
}

uint64_t
LamportClock::Update(uint64_t messageTime)
{
    NS_LOG_FUNCTION(this << messageTime);
    m_counter = std::max(m_counter, messageTime) + 1;
    NS_LOG_INFO("Lamport clock updated with message time " << messageTime
                                                           << ". New value: " << m_counter);
    return m_counter;
}

uint64_t
LamportClock::GetValue() const
{
    NS_LOG_FUNCTION(this);
    return m_counter;
}

} // namespace ns3
