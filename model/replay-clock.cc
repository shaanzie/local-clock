/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "replay-clock.h"

#include "unbounded-skew-clock.h"

#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"

#include <bit>
#include <bitset>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReplayClock");

TypeId
ReplayClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ReplayClock")
                            .SetParent<LocalClock>()
                            .SetGroupName("Network")
                            .AddConstructor<ReplayClock>()
                            .AddAttribute("NodeId",
                                          "Node ID",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_nodeId),
                                          MakeUintegerChecker<uint32_t>())
                            .AddAttribute("LocalClock",
                                          "Pointer to LocalClock",
                                          PointerValue(),
                                          MakePointerAccessor(&ReplayClock::m_localClock),
                                          MakePointerChecker<LocalClock>())
                            .AddAttribute("HLC",
                                          "Pointer to HLC",
                                          PointerValue(),
                                          MakePointerAccessor(&ReplayClock::m_maxph),
                                          MakePointerChecker<LocalClock>())
                            .AddAttribute("Bitmap",
                                          "Bitmap",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_bitmap),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("Offsets",
                                          "Offsets",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_offsets),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("Counters",
                                          "Counters",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_counters),
                                          MakeUintegerChecker<uint64_t>());
    return tid;
}

ReplayClock::ReplayClock()
{
    m_maxph = CreateObject<UnboundedSkewClock>();
}

ReplayClock::ReplayClock(uint32_t nodeId,
                         Ptr<LocalClock> localClock,
                         Ptr<LocalClock> hlc,
                         uint64_t bitmap,
                         uint64_t offsets,
                         uint64_t counters)
    : m_nodeId(nodeId),
      m_localClock(localClock),
      m_maxph(hlc),
      m_bitmap(bitmap),
      m_offsets(offsets),
      m_counters(counters)
{
}

ReplayClock::~ReplayClock()
{
}

Time
ReplayClock::Now()
{
    return m_localClock->Now();
}

void
ReplayClock::SetLocalClock(Time ptime)
{
    m_maxph->SetLocalClock(ptime);
}

void
ReplayClock::Shift(int64_t epoch_diff, int64_t max_epochs)
{
    int64_t index = 0;
    uint64_t rawBitmap = m_bitmap;
    uint64_t bitmap = rawBitmap;

    while (bitmap > 0)
    {
        uint64_t currentBitMask = bitmap & -bitmap;
        int64_t newOffsetAtIndex = GetOffsetAtIndex(index, max_epochs) + epoch_diff;

        if (newOffsetAtIndex >= max_epochs)
        {
            RemoveOffsetAtIndex(index, max_epochs);
            RemoveCounterAtIndex(index, max_epochs);
            rawBitmap &= ~currentBitMask;
        }
        else
        {
            SetOffsetAtIndex(index, newOffsetAtIndex, max_epochs);
            index++;
        }
        bitmap &= (bitmap - 1);
    }
    m_bitmap = rawBitmap;
}

void
ReplayClock::MergeSameEpoch(const ReplayClock& o_replayClock, int64_t u_epsilon)
{
    ReplayClock temp = *this;

    uint64_t myBits = m_bitmap;
    uint64_t otherBits = o_replayClock.m_bitmap;
    uint64_t iterator = myBits | otherBits;
    uint64_t resultBits = iterator;

    temp.m_offsets = 0;
    temp.m_counters = 0;

    uint32_t left_index = 0;
    uint32_t right_index = 0;
    uint32_t new_index = 0;

    while (iterator > 0)
    {
        uint64_t mask = iterator & -iterator;
        bool hasLeft = (myBits & mask) != 0;
        bool hasRight = (otherBits & mask) != 0;

        int64_t new_offset;
        int64_t new_counter;

        if (hasLeft && hasRight)
        {
            int64_t leftOff = GetOffsetAtIndex(left_index, u_epsilon);
            int64_t rightOff = o_replayClock.GetOffsetAtIndex(right_index, u_epsilon);

            if (leftOff < rightOff)
            {
                new_offset = leftOff;
                new_counter = GetCounterAtIndex(left_index, u_epsilon);
            }
            else if (rightOff < leftOff)
            {
                new_offset = rightOff;
                new_counter = o_replayClock.GetCounterAtIndex(right_index, u_epsilon);
            }
            else
            {
                new_offset = leftOff;
                new_counter = std::max(GetCounterAtIndex(left_index, u_epsilon),
                                       o_replayClock.GetCounterAtIndex(right_index, u_epsilon));
            }
            left_index++;
            right_index++;
        }
        else if (hasLeft)
        {
            new_offset = GetOffsetAtIndex(left_index, u_epsilon);
            new_counter = GetCounterAtIndex(left_index, u_epsilon);
            left_index++;
        }
        else
        {
            new_offset = o_replayClock.GetOffsetAtIndex(right_index, u_epsilon);
            new_counter = o_replayClock.GetCounterAtIndex(right_index, u_epsilon);
            right_index++;
        }

        if (new_offset >= u_epsilon)
        {
            resultBits &= ~mask;
        }
        else
        {
            temp.SetOffsetAtIndex(new_index, new_offset, u_epsilon);
            temp.SetCounterAtIndex(new_index, new_counter, u_epsilon);
            new_index++;
        }
        iterator &= (iterator - 1);
    }

    temp.m_bitmap = resultBits;
    *this = temp;
}

bool
ReplayClock::EqualOffset(const ReplayClock& o_replayClock)
{
    return o_replayClock.m_maxph->Now() == m_maxph->Now() && o_replayClock.m_bitmap == m_bitmap &&
           o_replayClock.m_offsets == m_offsets;
}

bool
ReplayClock::CheckOverflow(uint64_t combinedBitmap, int64_t max_epochs) const
{
    if (max_epochs <= 0)
    {
        return false;
    }
    int64_t s = std::bit_width(static_cast<uint64_t>(max_epochs));
    return (std::popcount(combinedBitmap) * s) > 64;
}

int64_t
ReplayClock::ExtractKBitsFromPositionP(uint64_t number, int64_t k, int64_t p) const
{
    return (number >> p) & ((1ULL << k) - 1);
}

int64_t
ReplayClock::GetOffsetAtIndex(int64_t index, int64_t u_epsilon) const
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    return ExtractKBitsFromPositionP(m_offsets, s, s * index);
}

void
ReplayClock::InsertOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon)
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    uint64_t pos = s * index;
    uint64_t mask = (1ULL << pos) - 1;
    m_offsets =
        (m_offsets & mask) | (static_cast<uint64_t>(value) << pos) | ((m_offsets & ~mask) << s);
}

void
ReplayClock::SetOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon)
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    auto newOffsets = static_cast<uint64_t>(ExtractKBitsFromPositionP(m_offsets, s * index, 0));
    newOffsets |= static_cast<uint64_t>(value) << (index * s);
    newOffsets |= static_cast<uint64_t>(
                      ExtractKBitsFromPositionP(m_offsets, 64 - s * (index + 1), s * (index + 1)))
                  << ((index + 1) * s);
    m_offsets = newOffsets;
}

void
ReplayClock::RemoveOffsetAtIndex(int64_t index, int64_t u_epsilon)
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    auto newOffsets = static_cast<uint64_t>(ExtractKBitsFromPositionP(m_offsets, s * index, 0));
    newOffsets |= static_cast<uint64_t>(
                      ExtractKBitsFromPositionP(m_offsets, 64 - s * (index + 1), s * (index + 1)))
                  << (index * s);
    m_offsets = newOffsets;
}

int64_t
ReplayClock::GetCounterAtIndex(int64_t index, int64_t u_epsilon) const
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    return ExtractKBitsFromPositionP(m_counters, s, s * index);
}

void
ReplayClock::InsertCounterAtIndex(int64_t index, int64_t value, int64_t u_epsilon)
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    uint64_t pos = s * index;
    uint64_t mask = (1ULL << pos) - 1;
    m_counters =
        (m_counters & mask) | (static_cast<uint64_t>(value) << pos) | ((m_counters & ~mask) << s);
}

void
ReplayClock::SetCounterAtIndex(int64_t index, int64_t value, int64_t u_epsilon)
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    auto newCounters =
        static_cast<uint64_t>(ExtractKBitsFromPositionP(m_counters, s * index, 0));
    newCounters |= static_cast<uint64_t>(value) << (index * s);
    newCounters |= static_cast<uint64_t>(
                       ExtractKBitsFromPositionP(m_counters, 64 - s * (index + 1), s * (index + 1)))
                   << ((index + 1) * s);
    m_counters = newCounters;
}

void
ReplayClock::RemoveCounterAtIndex(int64_t index, int64_t u_epsilon)
{
    int64_t s = std::bit_width(static_cast<uint64_t>(u_epsilon));
    auto newCounters =
        static_cast<uint64_t>(ExtractKBitsFromPositionP(m_counters, s * index, 0));
    newCounters |= static_cast<uint64_t>(
                       ExtractKBitsFromPositionP(m_counters, 64 - s * (index + 1), s * (index + 1)))
                   << (index * s);
    m_counters = newCounters;
}

void
ReplayClock::IncrementCounterAtIndex(int64_t index, int64_t u_epsilon)
{
    SetCounterAtIndex(index, GetCounterAtIndex(index, u_epsilon) + 1, u_epsilon);
}

void
ReplayClock::IncrementSelfCounter(int64_t u_epsilon)
{
    uint64_t targetMask = 1ULL << m_nodeId;
    int64_t idx = 0;
    uint64_t iterator = m_bitmap;

    while (iterator > 0)
    {
        if ((iterator & -iterator) == targetMask)
        {
            IncrementCounterAtIndex(idx, u_epsilon);
            break;
        }
        iterator &= (iterator - 1);
        idx++;
    }
}

void
ReplayClock::PrintClock(std::ostream& os, uint64_t u_epsilon)
{
    os << m_maxph->Now().GetMicroSeconds() << " | ";
    os << std::bitset<64>(m_bitmap) << " | ";
    int64_t index = 0;
    uint64_t bitmap = m_bitmap;
    while (bitmap > 0)
    {
        uint32_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
        os << "[" << processId << ":" << GetOffsetAtIndex(index, u_epsilon) << ":"
           << GetCounterAtIndex(index, u_epsilon) << "] ";
        bitmap &= (bitmap - 1);
        index++;
    }
    os << std::endl;
}

void
ReplayClock::Send(int64_t u_epsilon, Time u_interval)
{
    int64_t pt_j = m_localClock->Now().GetMicroSeconds();
    int64_t ts_maxph = m_maxph->Now().GetMicroSeconds();
    int64_t I = u_interval.GetMicroSeconds();

    int64_t newmaxph = std::max(ts_maxph, pt_j);
    int64_t currmaxt = ts_maxph / I;
    int64_t newmaxt = newmaxph / I;
    int64_t my_epoch = pt_j / I;
    int64_t max_epochs = u_epsilon / I;

    if (CheckOverflow(m_bitmap | (1ULL << m_nodeId), max_epochs))
    {
        m_overflowed = true;
        return;
    }

    int64_t new_off = newmaxt - my_epoch;

    if (newmaxt > currmaxt)
    {
        m_counters = 0;
        Shift(newmaxt - currmaxt, max_epochs);
    }

    bool isNew = !(m_bitmap & (1ULL << m_nodeId));
    m_bitmap |= (1ULL << m_nodeId);
    int myIdx = std::popcount(m_bitmap & ((1ULL << m_nodeId) - 1));

    if (newmaxt == currmaxt)
    {
        if (isNew)
        {
            InsertOffsetAtIndex(myIdx, new_off, max_epochs);
            InsertCounterAtIndex(myIdx, 0, max_epochs);
        }
        else
        {
            int64_t curr_off = GetOffsetAtIndex(myIdx, max_epochs);
            new_off = std::min(new_off, curr_off);

            if (new_off == curr_off)
            {
                IncrementCounterAtIndex(myIdx, max_epochs);
            }
            else
            {
                SetCounterAtIndex(myIdx, 0, max_epochs);
            }

            SetOffsetAtIndex(myIdx, new_off, max_epochs);
        }
    }
    else
    {
        if (isNew)
        {
            InsertOffsetAtIndex(myIdx, new_off, max_epochs);
            InsertCounterAtIndex(myIdx, 0, max_epochs);
        }
        else
        {
            SetOffsetAtIndex(myIdx, new_off, max_epochs);
        }
    }

    m_maxph->SetLocalClock(MicroSeconds(newmaxph));
}

void
ReplayClock::Recv(Ptr<ReplayClock> o_replayClock, int64_t u_epsilon, Time u_interval)
{
    int64_t pt_j = m_localClock->Now().GetMicroSeconds();
    int64_t ts_local_maxph = m_maxph->Now().GetMicroSeconds();
    int64_t ts_remote_maxph = o_replayClock->m_maxph->Now().GetMicroSeconds();
    int64_t I = u_interval.GetMicroSeconds();

    int64_t newmaxph = std::max({ts_local_maxph, ts_remote_maxph, pt_j});
    int64_t newmaxt = newmaxph / I;
    int64_t my_epoch = pt_j / I;
    int64_t max_epochs = u_epsilon / I;

    if (CheckOverflow(m_bitmap | o_replayClock->m_bitmap | (1ULL << m_nodeId), max_epochs))
    {
        m_overflowed = true;
        return;
    }

    uint64_t orig_local_bitmap = m_bitmap;
    uint64_t orig_local_offsets = m_offsets;
    int64_t orig_local_maxt = ts_local_maxph / I;

    uint64_t orig_remote_bitmap = o_replayClock->m_bitmap;
    uint64_t orig_remote_offsets = o_replayClock->m_offsets;
    int64_t orig_remote_maxt = ts_remote_maxph / I;

    int64_t origLocalSelfCounter = 0;
    if (m_bitmap & (1ULL << m_nodeId))
    {
        origLocalSelfCounter =
            GetCounterAtIndex(std::popcount(m_bitmap & ((1ULL << m_nodeId) - 1)), max_epochs);
    }

    int64_t origRemoteSelfCounter = 0;
    if (o_replayClock->m_bitmap & (1ULL << m_nodeId))
    {
        origRemoteSelfCounter = o_replayClock->GetCounterAtIndex(
            std::popcount(o_replayClock->m_bitmap & ((1ULL << m_nodeId) - 1)),
            max_epochs);
    }

    Ptr<ReplayClock> shiftedRemote = CreateObject<ReplayClock>();
    shiftedRemote->m_bitmap = o_replayClock->m_bitmap;
    shiftedRemote->m_offsets = o_replayClock->m_offsets;
    shiftedRemote->m_counters = o_replayClock->m_counters;
    shiftedRemote->Shift(newmaxt - orig_remote_maxt, max_epochs);

    Shift(newmaxt - orig_local_maxt, max_epochs);
    MergeSameEpoch(*shiftedRemote, max_epochs);

    bool is_eq_local = orig_local_maxt == newmaxt && orig_local_bitmap == m_bitmap &&
                       orig_local_offsets == m_offsets;
    bool is_eq_remote = orig_remote_maxt == newmaxt && orig_remote_bitmap == m_bitmap &&
                        orig_remote_offsets == m_offsets;

    int64_t newSelfCounter;
    if (is_eq_local && is_eq_remote)
    {
        newSelfCounter = std::max(origLocalSelfCounter, origRemoteSelfCounter) + 1;
    }
    else if (is_eq_local)
    {
        newSelfCounter = origLocalSelfCounter + 1;
    }
    else if (is_eq_remote)
    {
        newSelfCounter = origRemoteSelfCounter + 1;
    }
    else
    {
        m_counters = 0;
        newSelfCounter = 0;
    }

    bool isNew = !(m_bitmap & (1ULL << m_nodeId));
    m_bitmap |= (1ULL << m_nodeId);
    int myIdx = std::popcount(m_bitmap & ((1ULL << m_nodeId) - 1));

    int64_t myOffset = newmaxt - my_epoch;

    if (isNew)
    {
        InsertOffsetAtIndex(myIdx, myOffset, max_epochs);
        InsertCounterAtIndex(myIdx, newSelfCounter, max_epochs);
    }
    else
    {
        myOffset = std::min(myOffset, GetOffsetAtIndex(myIdx, max_epochs));
        SetOffsetAtIndex(myIdx, myOffset, max_epochs);
        SetCounterAtIndex(myIdx, newSelfCounter, max_epochs);
    }

    m_maxph->SetLocalClock(MicroSeconds(newmaxph));
}

} // namespace ns3
