/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */


#ifndef REPLAY_CLOCK_H
#define REPLAY_CLOCK_H

#include "local-clock.h"

#include "ns3/log.h"
#include "ns3/nstime.h"

#include <bit>

namespace ns3
{

/**
 * \ingroup network
 * \brief A hybrid logical clock that supports causal replay of distributed events.
 *
 * ReplayClock extends LocalClock by compactly tracking maximum physical time (HLC), 
 * participating nodes, and their logical counters to maintain causal ordering.
 */
class ReplayClock : public LocalClock
{
  public:
    /**
     * \brief Get the TypeId of this class.
     * \return The TypeId.
     */
    static TypeId GetTypeId();

    /**
     * \brief Default constructor.
     */
    ReplayClock();

    /**
     * \brief Constructor.
     * \param nodeId     The ID of the local node.
     * \param localClock Pointer to the node's physical clock.
     * \param hlc        Pointer to the HLC used to track maximum physical time.
     * \param bitmap     Initial bitmap of participating nodes.
     * \param offsets    Initial packed offset array.
     * \param counters   Initial packed counter array.
     */
    ReplayClock(uint32_t nodeId,
                Ptr<LocalClock> localClock,
                Ptr<LocalClock> hlc,
                uint64_t bitmap,
                uint64_t offsets,
                uint64_t counters);

    /**
     * \brief Destructor.
     */
    virtual ~ReplayClock();

    /**
     * \brief Get the current physical time.
     * \return Current physical time.
     */
    virtual Time Now() override;

    /**
     * \brief Update the internal physical time.
     * \param ptime The new physical time to set.
     */
    virtual void SetLocalClock(Time ptime) override;

    /** \return The local node ID. */
    uint32_t GetNodeId() const { return m_nodeId; }

    /** \return Pointer to the HLC tracking maximum physical time. */
    Ptr<LocalClock> GetHLC() const { return m_maxph; }

    /** \return The bitmap of participating nodes. */
    uint64_t GetBitmap() const { return m_bitmap; }

    /** \return The packed offset array. */
    uint64_t GetOffsets() const { return m_offsets; }

    /** \return The packed counter array. */
    uint64_t GetCounters() const { return m_counters; }

    /** \return The number of nodes currently tracked. */
    uint32_t GetNumStoredOffsets() const { return std::popcount(m_bitmap); }

    /** \return True if a 64-bit overflow was detected during Send or Recv. */
    bool HasOverflowed() const { return m_overflowed; }

    /** \brief Clear the overflow flag. */
    void ResetOverflow() { m_overflowed = false; }

    /**
     * \brief Set the HLC pointer.
     * \param hlc Pointer to the new HLC.
     */
    void SetHLC(Ptr<LocalClock> hlc) { m_maxph = hlc; }

    /**
     * \brief Set the bitmap of participating nodes.
     * \param bitmap The new bitmap value.
     */
    void SetBitmap(uint64_t bitmap) { m_bitmap = bitmap; }

    /**
     * \brief Set the packed offset array.
     * \param offsets The new offsets value.
     */
    void SetOffsets(uint64_t offsets) { m_offsets = offsets; }

    /**
     * \brief Set the packed counter array.
     * \param counters The new counters value.
     */
    void SetCounters(uint64_t counters) { m_counters = counters; }

    /**
     * \brief Equality operator. 
     * \param other The clock to compare against.
     * \return True if HLC, bitmap, offsets, and counters all match.
     */
    bool operator==(const ReplayClock& other) const
    {
        return m_maxph == other.m_maxph && m_bitmap == other.m_bitmap &&
               m_offsets == other.m_offsets && m_counters == other.m_counters;
    }

    /**
     * \brief Inequality operator.
     * \param other The clock to compare against.
     * \return True if not equal.
     */
    bool operator!=(const ReplayClock& other) const { return !(*this == other); }

    /**
     * \brief Copy constructor.
     * \param other The clock to copy from.
     */
    ReplayClock(const ReplayClock& other)
        : m_nodeId(other.m_nodeId),
          m_localClock(other.m_localClock),
          m_maxph(other.m_maxph),
          m_bitmap(other.m_bitmap),
          m_offsets(other.m_offsets),
          m_counters(other.m_counters)
    {
    }

    /**
     * \brief Assignment operator.
     * \param other The clock to assign from.
     * \return Reference to this clock.
     */
    ReplayClock& operator=(const ReplayClock& other)
    {
        if (this != &other)
        {
            m_nodeId     = other.m_nodeId;
            m_localClock = other.m_localClock;
            m_maxph      = other.m_maxph;
            m_bitmap     = other.m_bitmap;
            m_offsets    = other.m_offsets;
            m_counters   = other.m_counters;
        }
        return *this;
    }

    /**
     * \brief Advances tracked epochs and evicts stale nodes.
     * \param epoch_diff Number of epochs to shift forward.
     * \param max_epochs Eviction threshold.
     */
    void Shift(int64_t epoch_diff, int64_t max_epochs);

    /**
     * \brief Merges the state of a remote clock into the local clock.
     * \param o_replayClock The remote clock to merge with.
     * \param u_epsilon     The bounded skew parameter.
     */
    void MergeSameEpoch(const ReplayClock& o_replayClock, int64_t u_epsilon);

    /**
     * \brief Checks if two clocks share the same physical time, bitmap, and offsets.
     * \param o_replayClock The clock to compare against.
     * \return True if equal.
     */
    bool EqualOffset(const ReplayClock& o_replayClock);

    /**
     * \brief Checks if merging nodes would cause the internal arrays to overflow.
     * \param combinedBitmap Union of all bitmaps that would be present.
     * \param max_epochs     Slot width parameter.
     * \return True if the arrays would overflow 64 bits.
     */
    bool CheckOverflow(uint64_t combinedBitmap, int64_t max_epochs) const;

    /**
     * \brief Extracts a subset of bits from a given value.
     * \param number The value to extract from.
     * \param k      Number of bits to extract.
     * \param p      Starting bit position.
     * \return The extracted k-bit value.
     */
    int64_t ExtractKBitsFromPositionP(uint64_t number, int64_t k, int64_t p) const;

    /**
     * \brief Reads the offset at the given index.
     * \param index     Compressed array index.
     * \param u_epsilon Slot width parameter.
     * \return The offset value.
     */
    int64_t GetOffsetAtIndex(int64_t index, int64_t u_epsilon) const;

    /**
     * \brief Inserts a new offset at the given index.
     * \param index     Insertion point.
     * \param value     Offset value to insert.
     * \param u_epsilon Slot width parameter.
     */
    void InsertOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon);

    /**
     * \brief Updates the offset at the given index.
     * \param index     Compressed array index.
     * \param value     New offset value.
     * \param u_epsilon Slot width parameter.
     */
    void SetOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon);

    /**
     * \brief Removes the offset at the given index.
     * \param index     Compressed array index.
     * \param u_epsilon Slot width parameter.
     */
    void RemoveOffsetAtIndex(int64_t index, int64_t u_epsilon);

    /**
     * \brief Reads the counter at the given index.
     * \param index     Compressed array index.
     * \param u_epsilon Slot width parameter.
     * \return The counter value.
     */
    int64_t GetCounterAtIndex(int64_t index, int64_t u_epsilon) const;

    /**
     * \brief Inserts a new counter at the given index.
     * \param index     Insertion point.
     * \param value     Counter value to insert.
     * \param u_epsilon Slot width parameter.
     */
    void InsertCounterAtIndex(int64_t index, int64_t value, int64_t u_epsilon);

    /**
     * \brief Updates the counter at the given index.
     * \param index     Compressed array index.
     * \param value     New counter value.
     * \param u_epsilon Slot width parameter.
     */
    void SetCounterAtIndex(int64_t index, int64_t value, int64_t u_epsilon);

    /**
     * \brief Removes the counter at the given index.
     * \param index     Compressed array index.
     * \param u_epsilon Slot width parameter.
     */
    void RemoveCounterAtIndex(int64_t index, int64_t u_epsilon);

    /**
     * \brief Increments the counter at the given index.
     * \param index     Compressed array index.
     * \param u_epsilon Slot width parameter.
     */
    void IncrementCounterAtIndex(int64_t index, int64_t u_epsilon);

    /**
     * \brief Increments the local node's logical counter.
     * \param u_epsilon Slot width parameter.
     */
    void IncrementSelfCounter(int64_t u_epsilon);

    /**
     * \brief Prints the current clock state.
     * \param os        Output stream to write to.
     * \param u_epsilon Slot width parameter.
     */
    void PrintClock(std::ostream& os, uint64_t u_epsilon);

    /**
     * \brief Updates the clock state for a local event or message transmission.
     * \param u_epsilon  The bounded skew parameter.
     * \param u_interval The epoch interval length.
     */
    void Send(int64_t u_epsilon, Time u_interval);

    /**
     * \brief Updates the clock state upon receiving a message from a remote node.
     * \param o_replayClock The clock received from the remote node.
     * \param u_epsilon     The bounded skew parameter.
     * \param u_interval    The epoch interval length.
     */
    void Recv(Ptr<ReplayClock> o_replayClock, int64_t u_epsilon, Time u_interval);

  private:
    uint32_t        m_nodeId;             //!< ID of the local node.
    Ptr<LocalClock> m_localClock;         //!< Physical clock of the local node.
    Ptr<LocalClock> m_maxph;              //!< HLC tracking the maximum physical time seen.
    uint64_t        m_bitmap   = 0;       //!< Bitmap of nodes with active offset/counter slots.
    uint64_t        m_offsets  = 0;       //!< Packed per-node epoch offset array.
    uint64_t        m_counters = 0;       //!< Packed per-node logical counter array.
    bool            m_overflowed = false; //!< True if a 64-bit capacity breach was detected.
};

} // namespace ns3

#endif /* REPLAY_CLOCK_H */