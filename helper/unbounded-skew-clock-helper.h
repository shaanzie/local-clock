/*
 * Copyright (c) 2024 Ishaan Lagwankar
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef UNBOUNDED_SKEW_CLOCK_HELPER_H
#define UNBOUNDED_SKEW_CLOCK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/unbounded-skew-clock.h"

namespace ns3
{

/**
 * \brief Helper class to aggregate UnboundedSkewClock to nodes.
 */
class UnboundedSkewClockHelper
{
public:
    /**
     * \brief Create an UnboundedSkewClockHelper to make it easier to add 
     *        UnboundedSkewClocks to Nodes.
     */
    UnboundedSkewClockHelper();

    /**
     * \brief Set an attribute of the underlying UnboundedSkewClock object.
     *
     * \param name The name of the attribute to set.
     * \param value The value of the attribute to set.
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * \brief Install an UnboundedSkewClock on the provided node.
     *
     * \param node The node to install the clock on.
     */
    void Install(Ptr<Node> node) const;

    /**
     * \brief Install an UnboundedSkewClock on all nodes in the container.
     *
     * \param c The container of nodes.
     */
    void Install(NodeContainer c) const;

private:
    ObjectFactory m_factory; //!< Object factory to create the clock instances.
};

} // namespace ns3

#endif /* UNBOUNDED_SKEW_CLOCK_HELPER_H */