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
#include "ns3/vector-clock.h"

using namespace ns3;

/**
 * \ingroup clocks-tests
 * \brief Test case to verify basic initialization and Tick() functionality of the VectorClock.
 */
class VectorClockBasicTest : public TestCase
{
public:
    /**
     * \brief Constructor.
     */
    VectorClockBasicTest();

    /**
     * \brief Run the test case.
     */
    void DoRun() override;
};

VectorClockBasicTest::VectorClockBasicTest()
    : TestCase("VectorClockTest")
{
}

void
VectorClockBasicTest::DoRun()
{
    Ptr<VectorClock> clock = CreateObject<VectorClock>();
    clock->SetLocalId(1); 
    
    NS_TEST_ASSERT_MSG_EQ(clock->GetVector().size(), 0, "Vector should be empty initially.");
    NS_TEST_ASSERT_MSG_EQ(clock->Now().GetNanoSeconds(), 0, "Now() should map to 0 initially.");

    std::map<uint32_t, uint64_t> vec1 = clock->Tick();
    NS_TEST_ASSERT_MSG_EQ(vec1[1], 1, "Local ID 1 should be incremented to 1.");
    NS_TEST_ASSERT_MSG_EQ(clock->Now().GetNanoSeconds(), 1, "Now() should reflect local ID's tick.");

    std::map<uint32_t, uint64_t> vec2 = clock->Tick();
    NS_TEST_ASSERT_MSG_EQ(vec2[1], 2, "Local ID 1 should be incremented to 2.");
}

/**
 * \ingroup clocks-tests
 * \brief Test case to verify the causal Update logic of the VectorClock.
 */
class VectorClockUpdateTest : public TestCase
{
public:
    /**
     * \brief Constructor.
     */
    VectorClockUpdateTest();

    /**
     * \brief Run the test case.
     */
    void DoRun() override;
};

VectorClockUpdateTest::VectorClockUpdateTest()
    : TestCase("VectorClockTest2")
{
}

void
VectorClockUpdateTest::DoRun()
{
    Ptr<VectorClock> clock = CreateObject<VectorClock>();
    clock->SetLocalId(1);
    
    clock->Tick(); 
    clock->Tick(); 

    std::map<uint32_t, uint64_t> msgVector;
    msgVector[1] = 1; 
    msgVector[2] = 5; 
    msgVector[3] = 2; 

    std::map<uint32_t, uint64_t> updatedVec = clock->Update(msgVector);

    NS_TEST_ASSERT_MSG_EQ(updatedVec[1], 2, "Local ID should retain its higher value (max(2, 1)).");
    NS_TEST_ASSERT_MSG_EQ(updatedVec[2], 5, "Node 2's value should be updated from message.");
    NS_TEST_ASSERT_MSG_EQ(updatedVec[3], 2, "Node 3's value should be updated from message.");
}

/**
 * \ingroup clocks-tests
 * \brief The Test Suite that bundles all VectorClock tests together.
 */
class VectorClockTestSuite : public TestSuite
{
public:
    /**
     * \brief Constructor.
     */
    VectorClockTestSuite();
};

VectorClockTestSuite::VectorClockTestSuite()
    : TestSuite("vector-clock", Type::UNIT)
{
    AddTestCase(new VectorClockBasicTest, TestCase::Duration::QUICK);
    AddTestCase(new VectorClockUpdateTest, TestCase::Duration::QUICK);
}

static VectorClockTestSuite s_vectorClockTestSuite;