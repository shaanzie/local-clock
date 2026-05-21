#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/local-clock-helper.h"
#include "ns3/vector-clock.h"
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VectorClockExample");

std::string 
FormatVector(const std::map<uint32_t, uint64_t>& vec)
{
    std::stringstream ss;
    ss << "{ ";
    for (auto const& [nodeId, time] : vec)
    {
        ss << "Node" << nodeId << ":" << time << " ";
    }
    ss << "}";
    return ss.str();
}

void 
SetupNodeId(Ptr<Node> node, uint32_t id)
{
    Ptr<VectorClock> clock = node->GetObject<VectorClock>();
    if (clock)
    {
        clock->SetLocalId(id);
    }
}

void 
ReceiveMessageEvent(Ptr<Node> receiver, std::string receiverName, std::map<uint32_t, uint64_t> msgVector)
{
    Ptr<VectorClock> clock = receiver->GetObject<VectorClock>();
    if (clock)
    {
        clock->Update(msgVector);
        NS_LOG_INFO(Simulator::Now().GetSeconds() << "s - " 
                    << receiverName << " received message. Vector merged to: " 
                    << FormatVector(clock->GetVector()));
    }
}

void 
SendMessageEvent(Ptr<Node> sender, Ptr<Node> receiver, std::string senderName, std::string receiverName, Time delay)
{
    Ptr<VectorClock> senderClock = sender->GetObject<VectorClock>();
    if (senderClock)
    {
        std::map<uint32_t, uint64_t> sendVector = senderClock->Tick();
        
        NS_LOG_INFO(Simulator::Now().GetSeconds() << "s - " 
                    << senderName << " sending message to " << receiverName 
                    << ". Vector: " << FormatVector(sendVector));

        Simulator::Schedule(delay, &ReceiveMessageEvent, receiver, receiverName, sendVector);
    }
}

int 
main(int argc, char* argv[])
{
    LogComponentEnable("VectorClockExample", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    LocalClockHelper clockHelper;
    clockHelper.SetClockType("ns3::VectorClock");
    clockHelper.Install(nodes);

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        SetupNodeId(nodes.Get(i), i);
    }

    Simulator::Schedule(Seconds(1.0), &SendMessageEvent, nodes.Get(0), nodes.Get(1), "Node 0", "Node 1", Seconds(0.1));
    Simulator::Schedule(Seconds(2.0), &SendMessageEvent, nodes.Get(1), nodes.Get(2), "Node 1", "Node 2", Seconds(0.1));
    Simulator::Schedule(Seconds(3.0), &SendMessageEvent, nodes.Get(2), nodes.Get(0), "Node 2", "Node 0", Seconds(0.1));

    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}