/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef VECTOR_CLOCK_H
#define VECTOR_CLOCK_H

#include "local-clock.h"

#include <map>

namespace ns3
{

/**
 * @ingroup clocks
 * @brief Implementation of a Vector logical clock.
 */
class VectorClock : public LocalClock
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * @brief Default constructor.
     */
    VectorClock();

    /**
     * @brief Destructor.
     */
    ~VectorClock() override;

    /**
     * @brief Get the current logical time of the local node cast as an ns-3 Time object.
     *
     * @return Current local time.
     */
    Time Now() override;

    /**
     * @brief Set the unique identifier for the local node.
     *
     * @param localId The unique node identifier.
     */
    void SetLocalId(uint32_t localId);

    /**
     * @brief Increment the clock for a local event or message transmission.
     *
     * @return The updated vector clock state.
     */
    std::map<uint32_t, uint64_t> Tick();

    /**
     * @brief Update the local vector upon receiving a message from another node.
     *
     * @param msgVector The vector clock on the incoming message.
     * @return The merged vector clock state.
     */
    std::map<uint32_t, uint64_t> Update(const std::map<uint32_t, uint64_t>& msgVector);

    /**
     * @brief Retrieve the current vector clock state.
     * @return A map representing the vector clock.
     */
    std::map<uint32_t, uint64_t> GetVector() const;

  private:
    uint32_t m_localId;                    ///< The identifier for the local node.
    std::map<uint32_t, uint64_t> m_vector; ///< The vector of logical timestamps.
};

} // namespace ns3

#endif /* VECTOR_CLOCK_H */
