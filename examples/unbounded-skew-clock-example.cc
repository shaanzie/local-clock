#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "unbounded-skew-clock.h"
#include "unbounded-skew-clock-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UnboundedSkewClockNodeExample");

// Standalone function to fetch the clocks from the nodes, print time, and shuffle
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
    LogComponentEnable("UnboundedSkewClockNodeExample", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);

    UnboundedSkewClockHelper clockHelper;
    clockHelper.Install(nodes);

    for (int i = 0; i < 20; ++i)
    {
        Simulator::Schedule(Seconds(i), &CheckAndShuffleNodeClocks, nodes.Get(0), nodes.Get(1));
    }

    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}