/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "ns3/local-clock.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/replay-clock.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

class TestLocalClock : public LocalClock
{
  public:
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::TestLocalClock")
                                .SetParent<LocalClock>()
                                .SetGroupName("Network")
                                .AddConstructor<TestLocalClock>()
                                .AddAttribute("PhysicalTime",
                                              "ptime to start with",
                                              TimeValue(NanoSeconds(0)),
                                              MakeTimeAccessor(&TestLocalClock::m_ptime),
                                              MakeTimeChecker());
        return tid;
    }

    TestLocalClock() : m_ptime(NanoSeconds(0)) {}
    TestLocalClock(Time time) : m_ptime(time) {}

    virtual Time Now() override { return m_ptime; }
    virtual void SetLocalClock(Time time) override { m_ptime = time; }

  private:
    Time m_ptime;
};

class ReplayClockTestCase : public TestCase
{
  public:
    ReplayClockTestCase(std::string name) : TestCase(name) {}

    void CheckParams(Ptr<ReplayClock> rc,
                     Ptr<TestLocalClock> hlc,
                     uint64_t bitmap,
                     uint64_t offsets,
                     uint64_t counters)
    {
        NS_TEST_ASSERT_MSG_EQ(rc->GetHLC()->Now(), hlc->Now(), "HLC mismatch!");
        NS_TEST_ASSERT_MSG_EQ(rc->GetBitmap(),   bitmap,   "Bitmap mismatch!");
        NS_TEST_ASSERT_MSG_EQ(rc->GetOffsets(),  offsets,  "Offsets mismatch!");
        NS_TEST_ASSERT_MSG_EQ(rc->GetCounters(), counters, "Counters mismatch!");
    }
};

class InitCase : public ReplayClockTestCase
{
  public:
    InitCase() : ReplayClockTestCase("Testing initialization parameters.") {}
    void DoRun() override
    {
        Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("HLC", PointerValue(hlc));
        CheckParams(rc, hlc, 0, 0, 0);
    }
};

class OffsetCase1 : public ReplayClockTestCase
{
  public:
    OffsetCase1() : ReplayClockTestCase("Testing offset init functionality.") {}
    void DoRun() override
    {
        Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>(MicroSeconds(200));
        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("HLC",      PointerValue(hlc));
        rc->SetAttribute("Bitmap",   UintegerValue(13));
        rc->SetAttribute("Offsets",  UintegerValue(49408));
        rc->SetAttribute("Counters", UintegerValue(2));
        CheckParams(rc, hlc, 13, 49408, 2);
    }
};

class OffsetCase2 : public ReplayClockTestCase
{
  public:
    OffsetCase2() : ReplayClockTestCase("Testing offset get functionality.") {}
    void DoRun() override
    {
        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("Offsets", UintegerValue(49408));
        NS_TEST_ASSERT_MSG_EQ(rc->GetOffsetAtIndex(1, 100), 2, "Offset retrieval failed!");
    }
};

class OffsetCase3 : public ReplayClockTestCase
{
  public:
    OffsetCase3() : ReplayClockTestCase("Testing offset set functionality.") {}
    void DoRun() override
    {
        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("Offsets", UintegerValue(49408));
        rc->SetOffsetAtIndex(1, 5, 100);
        NS_TEST_ASSERT_MSG_EQ(rc->GetOffsets(), 49792ULL, "Offset setting failed!");
    }
};

class OffsetCase4 : public ReplayClockTestCase
{
  public:
    OffsetCase4() : ReplayClockTestCase("Testing offset remove functionality.") {}
    void DoRun() override
    {
        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("Offsets", UintegerValue(49408));
        rc->RemoveOffsetAtIndex(1, 100);
        NS_TEST_ASSERT_MSG_EQ(rc->GetOffsets(), 384ULL, "Offset removal failed!");
    }
};

class ShiftCase : public ReplayClockTestCase
{
  public:
    ShiftCase() : ReplayClockTestCase("Testing Shift functionality.") {}
    void DoRun() override
    {
        Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>(MicroSeconds(200));
        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("HLC",    PointerValue(hlc));
        rc->SetAttribute("Bitmap", UintegerValue(13));
        rc->SetAttribute("Offsets", UintegerValue(49408));

        rc->Shift(50, 100);
        rc->GetHLC()->SetLocalClock(MicroSeconds(250));

        NS_TEST_EXPECT_MSG_EQ(rc->GetHLC()->Now(),  MicroSeconds(250), "HLC didn't update");
        NS_TEST_EXPECT_MSG_EQ(rc->GetBitmap(),      13ULL,             "Bitmap changed during shift");
        NS_TEST_EXPECT_MSG_EQ(rc->GetOffsets(),     875058ULL,         "Shifted offsets mismatch");
        NS_TEST_EXPECT_MSG_EQ(rc->GetCounters(),    0ULL,              "Counters should be zero after shift");
    }
};

class MergeSameEpochCase : public ReplayClockTestCase
{
  public:
    MergeSameEpochCase() : ReplayClockTestCase("Testing MergeSameEpoch functionality.") {}
    void DoRun() override
    {
        Ptr<ReplayClock> rcA = CreateObject<ReplayClock>();
        rcA->SetAttribute("Bitmap",  UintegerValue(13));
        rcA->SetAttribute("Offsets", UintegerValue(49408));

        Ptr<ReplayClock> rcB = CreateObject<ReplayClock>();
        rcB->SetAttribute("Bitmap",  UintegerValue(4));
        rcB->SetAttribute("Offsets", UintegerValue(0));

        rcA->MergeSameEpoch(*rcB, 100);
        NS_TEST_EXPECT_MSG_EQ(rcA->GetOffsets(),  49152ULL, "Merge offset mismatch");
        NS_TEST_EXPECT_MSG_EQ(rcA->GetCounters(), 0ULL,     "Merge counter mismatch");
    }
};

class SendCase : public ReplayClockTestCase
{
  public:
    SendCase() : ReplayClockTestCase("Testing Send functionality.") {}
    void DoRun() override
    {
        Ptr<TestLocalClock> pt = CreateObject<TestLocalClock>();
        pt->SetLocalClock(MicroSeconds(700));

        Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
        hlc->SetLocalClock(MicroSeconds(200));

        Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
        rc->SetAttribute("NodeId",    UintegerValue(2));
        rc->SetAttribute("LocalClock", PointerValue(pt));
        rc->SetAttribute("HLC",       PointerValue(hlc));
        rc->SetAttribute("Bitmap",    UintegerValue(13));
        rc->SetAttribute("Offsets",   UintegerValue(49408));
        rc->SetAttribute("Counters",  UintegerValue(2));

        rc->Send(1000, MicroSeconds(10));

        NS_TEST_EXPECT_MSG_EQ(rc->GetHLC()->Now(), MicroSeconds(700), "Send HLC mismatch");
        NS_TEST_EXPECT_MSG_EQ(rc->GetBitmap(),     13ULL,             "Send bitmap mismatch");
        NS_TEST_EXPECT_MSG_EQ(rc->GetOffsets(),    868402ULL,         "Send offset mismatch");
        NS_TEST_EXPECT_MSG_EQ(rc->GetCounters(),   0ULL,              "Send counters mismatch");
    }
};

class RecvCase : public ReplayClockTestCase
{
  public:
    RecvCase() : ReplayClockTestCase("Testing Recv functionality.") {}
    void DoRun() override
    {
        Ptr<TestLocalClock> ptA = CreateObject<TestLocalClock>();
        ptA->SetLocalClock(MicroSeconds(500));

        Ptr<TestLocalClock> hlcA = CreateObject<TestLocalClock>();
        hlcA->SetLocalClock(MicroSeconds(200));

        Ptr<ReplayClock> rcA = CreateObject<ReplayClock>();
        rcA->SetAttribute("NodeId",    UintegerValue(0));
        rcA->SetAttribute("LocalClock", PointerValue(ptA));
        rcA->SetAttribute("HLC",       PointerValue(hlcA));
        rcA->SetAttribute("Bitmap",    UintegerValue(13));
        rcA->SetAttribute("Offsets",   UintegerValue(49408));

        Ptr<TestLocalClock> hlcB = CreateObject<TestLocalClock>();
        hlcB->SetLocalClock(MicroSeconds(400));

        Ptr<ReplayClock> rcB = CreateObject<ReplayClock>();
        rcB->SetAttribute("NodeId",  UintegerValue(2));
        rcB->SetAttribute("HLC",     PointerValue(hlcB));
        rcB->SetAttribute("Bitmap",  UintegerValue(18));
        rcB->SetAttribute("Offsets", UintegerValue(512));

        rcA->Recv(rcB, 1000, MicroSeconds(10));

        NS_TEST_EXPECT_MSG_EQ(rcA->GetHLC()->Now(), MicroSeconds(500),    "Recv HLC mismatch");
        NS_TEST_EXPECT_MSG_EQ(rcA->GetBitmap(),     31ULL,                "Recv bitmap mismatch");
        NS_TEST_EXPECT_MSG_EQ(rcA->GetOffsets(),    3827827968ULL,        "Recv offset mismatch");
        NS_TEST_EXPECT_MSG_EQ(rcA->GetCounters(),   0ULL,                 "Recv counters mismatch");
    }
};

class ReplayClockTestSuite : public TestSuite
{
  public:
    ReplayClockTestSuite() : TestSuite("replay-clock", Type::UNIT)
    {
        AddTestCase(new InitCase(),           TestCase::Duration::QUICK);
        AddTestCase(new OffsetCase1(),        TestCase::Duration::QUICK);
        AddTestCase(new OffsetCase2(),        TestCase::Duration::QUICK);
        AddTestCase(new OffsetCase3(),        TestCase::Duration::QUICK);
        AddTestCase(new OffsetCase4(),        TestCase::Duration::QUICK);
        AddTestCase(new ShiftCase(),          TestCase::Duration::QUICK);
        AddTestCase(new MergeSameEpochCase(), TestCase::Duration::QUICK);
        AddTestCase(new SendCase(),           TestCase::Duration::QUICK);
        AddTestCase(new RecvCase(),           TestCase::Duration::QUICK);
    }
};

static ReplayClockTestSuite sReplayClockTestSuite;