/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "vector-clock.h"

#include "ns3/log.h"

#include <algorithm>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("VectorClock");

NS_OBJECT_ENSURE_REGISTERED(VectorClock);

TypeId
VectorClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::VectorClock")
                            .SetParent<LocalClock>()
                            .SetGroupName("Network")
                            .AddConstructor<VectorClock>();
    return tid;
}

VectorClock::VectorClock()
    : m_localId(0)
{
}

VectorClock::~VectorClock()
{
}

Time
VectorClock::Now()
{
    return NanoSeconds(m_vector[m_localId]);
}

void
VectorClock::SetLocalId(uint32_t localId)
{
    m_localId = localId;
}

std::map<uint32_t, uint64_t>
VectorClock::Tick()
{
    m_vector[m_localId]++;
    return m_vector;
}

std::map<uint32_t, uint64_t>
VectorClock::Update(const std::map<uint32_t, uint64_t>& msgVector)
{
    for (const auto& [key, value] : msgVector)
    {
        m_vector[key] = std::max(m_vector[key], value);
    }
    return m_vector;
}

std::map<uint32_t, uint64_t>
VectorClock::GetVector() const
{
    return m_vector;
}

} // namespace ns3
