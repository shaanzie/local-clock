/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef LAMPORT_CLOCK_H
#define LAMPORT_CLOCK_H

#include "local-clock.h"

namespace ns3
{

/**
 * @ingroup clocks
 * @brief Implementation of a Lamport logical clock.
 */
class LamportClock : public LocalClock
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    LamportClock();
    ~LamportClock() override;

    /**
     * @brief Get the current logical time cast as an ns-3 Time object.
     *
     * @return Current time
     */
    Time Now() override;

    /**
     * @brief Increment the clock for a local event or message transmission.
     * @return The new logical time counter.
     */
    uint64_t Tick();

    /**
     * @brief Update the clock upon receiving a message from another node.
     *
     * @param messageTime The Lamport timestamp piggybacked on the incoming message.
     * @return The updated logical time counter.
     */
    uint64_t Update(uint64_t messageTime);

    /**
     * @brief Retrieve the raw logical integer value of the clock.
     * @return The current Lamport counter.
     */
    uint64_t GetValue() const;

  private:
    uint64_t m_counter; ///< The logical timestamp counter
};

} // namespace ns3

#endif /* LAMPORT_CLOCK_H */
