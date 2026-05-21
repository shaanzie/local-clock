#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/local-clock-helper.h"
#include "ns3/unbounded-skew-clock.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UnboundedSkewClockExample");

void 
CheckAndShuffleNodeClocks(Ptr<Node> nodeA, Ptr<Node> nodeB)
{
    Ptr<UnboundedSkewClock> clockA = nodeA->GetObject<UnboundedSkewClock>();
    Ptr<UnboundedSkewClock> clockB = nodeB->GetObject<UnboundedSkewClock>();

    if (clockA && clockB)
    {
        Time timeA = clockA->Now();
        Time timeB = clockB->Now();
        
        NS_LOG_INFO("At simulation time " << Simulator::Now().GetSeconds()
                                          << "s: Node " << nodeA->GetId() << " clock = " << timeA.GetSeconds()
                                          << "s, Node " << nodeB->GetId() << " clock = " << timeB.GetSeconds()
                                          << "s");
        
        // Shuffle skew values
        clockA->ShuffleSkew();
        clockB->ShuffleSkew();
    }
}

int 
main(int argc, char* argv[])
{
    LogComponentEnable("UnboundedSkewClockExample", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);

    LocalClockHelper clockHelper;
    clockHelper.SetClockType("ns3::UnboundedSkewClock");
    clockHelper.Install(nodes);

    for (int i = 0; i < 20; ++i)
    {
        Simulator::Schedule(Seconds(i), &CheckAndShuffleNodeClocks, nodes.Get(0), nodes.Get(1));
    }

    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}