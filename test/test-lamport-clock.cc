/*
 * Copyright (c) 2026 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "ns3/lamport-clock.h"

using namespace ns3;

/**
 * \brief Test case to verify basic initialization and Tick() functionality.
 */
class LamportClockBasicTest : public TestCase
{
public:
    LamportClockBasicTest();
    virtual ~LamportClockBasicTest();

private:
    void DoRun() override;
};

LamportClockBasicTest::LamportClockBasicTest()
    : TestCase("Verify initialization and Tick operations of LamportClock")
{
}

LamportClockBasicTest::~LamportClockBasicTest()
{
}

void
LamportClockBasicTest::DoRun()
{
    Ptr<LamportClock> clock = CreateObject<LamportClock>();

    NS_TEST_ASSERT_MSG_EQ(clock->GetValue(), 0, "Lamport clock must initialize to 0.");
    NS_TEST_ASSERT_MSG_EQ(clock->Now().GetNanoSeconds(), 0, "Now() must do initial tick to 0 nanoseconds.");

    uint64_t t1 = clock->Tick();
    NS_TEST_ASSERT_MSG_EQ(t1, 1, "First Tick() must return 1.");
    NS_TEST_ASSERT_MSG_EQ(clock->GetValue(), 1, "Internal state must be 1 after first tick.");

    uint64_t t2 = clock->Tick();
    NS_TEST_ASSERT_MSG_EQ(t2, 2, "Second Tick() must return 2.");
    
    NS_TEST_ASSERT_MSG_EQ(clock->Now().GetNanoSeconds(), 2, "Now() must do 2 ticks to 2 nanoseconds.");
}

/**
 * \brief Test case to verify the Update() logic.
 */
class LamportClockUpdateTest : public TestCase
{
public:
    LamportClockUpdateTest();
    virtual ~LamportClockUpdateTest();

private:
    void DoRun() override;
};

LamportClockUpdateTest::LamportClockUpdateTest()
    : TestCase("UpdateTest")
{
}

LamportClockUpdateTest::~LamportClockUpdateTest()
{
}

void
LamportClockUpdateTest::DoRun()
{
    Ptr<LamportClock> clock = CreateObject<LamportClock>();
    
    clock->Tick(); 
    clock->Tick(); 

    uint64_t t1 = clock->Update(1);
    NS_TEST_ASSERT_MSG_EQ(t1, 3, "Update with smaller timestamp failed max() rule.");

    uint64_t t2 = clock->Update(3);
    NS_TEST_ASSERT_MSG_EQ(t2, 4, "Update with equal timestamp failed max() rule.");

    uint64_t t3 = clock->Update(10);
    NS_TEST_ASSERT_MSG_EQ(t3, 11, "Update with larger timestamp failed max() rule.");
}

/**
 * \brief The Test Suite that bundles the individual tests together.
 */
class LamportClockTestSuite : public TestSuite
{
public:
    LamportClockTestSuite();
};

LamportClockTestSuite::LamportClockTestSuite()
    : TestSuite("lamport-clock", Type::UNIT)
{
    AddTestCase(new LamportClockBasicTest, TestCase::Duration::QUICK);
    AddTestCase(new LamportClockUpdateTest, TestCase::Duration::QUICK);
}

static LamportClockTestSuite slamportClockTestSuite;