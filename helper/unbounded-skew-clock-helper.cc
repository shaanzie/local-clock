/*
 * Copyright (c) 2024 Ishaan Lagwankar
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "unbounded-skew-clock-helper.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UnboundedSkewClockHelper");

UnboundedSkewClockHelper::UnboundedSkewClockHelper()
{
    m_factory.SetTypeId("ns3::UnboundedSkewClock");
}

void
UnboundedSkewClockHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

void
UnboundedSkewClockHelper::Install(Ptr<Node> node) const
{
    if (node->GetObject<UnboundedSkewClock>())
    {
        NS_FATAL_ERROR("UnboundedSkewClock is already installed on node " << node->GetId());
    }

    Ptr<UnboundedSkewClock> clock = m_factory.Create<UnboundedSkewClock>();
    node->AggregateObject(clock);
}

void
UnboundedSkewClockHelper::Install(NodeContainer c) const
{
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Install(*i);
    }
}

} // namespace ns3