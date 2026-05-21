#include "ns3/core-module.h"
#include "ns3/lamport-clock.h"
#include "ns3/local-clock-helper.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LamportClockExample");

void
NodeLocalEvent(Ptr<Node> node, std::string nodeName)
{
    Ptr<LamportClock> clock = node->GetObject<LamportClock>();
    if (clock)
    {
        uint64_t t = clock->Tick();
        NS_LOG_INFO(Simulator::Now().GetSeconds()
                    << "s - " << nodeName << " local event. Lamport Time: " << t);
    }
}

void
ReceiveMessageEvent(Ptr<Node> receiver, std::string receiverName, uint64_t msgTime)
{
    Ptr<LamportClock> clock = receiver->GetObject<LamportClock>();
    if (clock)
    {
        uint64_t t = clock->Update(msgTime);
        NS_LOG_INFO(Simulator::Now().GetSeconds()
                    << "s - " << receiverName << " received message (time: " << msgTime
                    << "). Lamport Time updated to: " << t);
    }
}

void
SendMessageEvent(Ptr<Node> sender,
                 Ptr<Node> receiver,
                 std::string senderName,
                 std::string receiverName,
                 Time propagationDelay)
{
    Ptr<LamportClock> senderClock = sender->GetObject<LamportClock>();
    if (senderClock)
    {
        uint64_t sendTime = senderClock->Tick();

        NS_LOG_INFO(Simulator::Now().GetSeconds()
                    << "s - " << senderName << " sending message to " << receiverName
                    << ". Lamport Time: " << sendTime);

        Simulator::Schedule(propagationDelay,
                            &ReceiveMessageEvent,
                            receiver,
                            receiverName,
                            sendTime);
    }
}

int
main(int argc, char* argv[])
{
    LogComponentEnable("LamportClockExample", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);
    Ptr<Node> nodeA = nodes.Get(0);
    Ptr<Node> nodeB = nodes.Get(1);

    LocalClockHelper clockHelper;
    clockHelper.SetClockType("ns3::LamportClock");
    clockHelper.Install(nodes);

    Simulator::Schedule(Seconds(1.0), &NodeLocalEvent, nodeA, "Node A");
    Simulator::Schedule(Seconds(1.5), &NodeLocalEvent, nodeA, "Node A");
    Simulator::Schedule(Seconds(2.0), &NodeLocalEvent, nodeB, "Node B");

    Simulator::Schedule(Seconds(3.0),
                        &SendMessageEvent,
                        nodeA,
                        nodeB,
                        "Node A",
                        "Node B",
                        Seconds(0.1));
    Simulator::Schedule(Seconds(4.0),
                        &SendMessageEvent,
                        nodeB,
                        nodeA,
                        "Node B",
                        "Node A",
                        Seconds(0.1));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
