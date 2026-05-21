#include "ns3/core-module.h"
#include "ns3/local-clock.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/test.h"
#include "ns3/unbounded-skew-clock.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ClockModelTestSuite");

class UnboundedSkewClockTestCase : public TestCase
{
  public:
    UnboundedSkewClockTestCase();
    void DoRun() override;

  private:
    void CheckFirst(Ptr<UnboundedSkewClock> skewClock);
    void CheckSecond(Ptr<UnboundedSkewClock> skewClock);
};

UnboundedSkewClockTestCase::UnboundedSkewClockTestCase()
    : TestCase("UnboundedSkewClockTestCase")
{
}

void 
UnboundedSkewClockTestCase::CheckFirst(Ptr<UnboundedSkewClock> skewClock)
{
    Time t1 = skewClock->Now();
    NS_TEST_ASSERT_MSG_EQ_TOL(t1.GetSeconds(), 2.0, 1e-9, "Local time should advance 2x faster than global time");
    NS_LOG_INFO("Passed 1-second skew check");
}

void 
UnboundedSkewClockTestCase::CheckSecond(Ptr<UnboundedSkewClock> skewClock)
{
    Time t2 = skewClock->Now();
    NS_TEST_ASSERT_MSG_EQ_TOL(t2.GetSeconds(), 3.0, 1e-9, "Local time should match expected skewed value");
    NS_LOG_INFO("Passed 1.5-second skew check");
}

void 
UnboundedSkewClockTestCase::DoRun()
{
    NS_LOG_INFO("Starting UnboundedSkewClockTestCase");

    Ptr<UnboundedSkewClock> skewClock = CreateObject<UnboundedSkewClock>();
    skewClock->SetSkewValues({2.0});

    Time t0 = skewClock->Now();
    NS_TEST_ASSERT_MSG_EQ(t0, Seconds(0), "Initial local time should be 0");

    Simulator::Schedule(Seconds(1.0), &UnboundedSkewClockTestCase::CheckFirst, this, skewClock);
    Simulator::Schedule(Seconds(1.5), &UnboundedSkewClockTestCase::CheckSecond, this, skewClock);

    Simulator::Run();
    Simulator::Destroy();

    NS_LOG_INFO("Completed UnboundedSkewClockTestCase");
}

class ClockModelTestSuite : public TestSuite
{
  public:
    ClockModelTestSuite();
};

ClockModelTestSuite::ClockModelTestSuite()
    : TestSuite("unbounded-skew-clock", TestSuite::Type::UNIT)
{
    AddTestCase(new UnboundedSkewClockTestCase(), TestCase::Duration::QUICK);
}

static ClockModelTestSuite g_clockModelTestSuite;