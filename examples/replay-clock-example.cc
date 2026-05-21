#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/local-clock.h"
#include "ns3/replay-clock.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ReplayClockExample");

class SimpleLocalClock : public LocalClock
{
  public:
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::SimpleLocalClock")
                                .SetParent<LocalClock>()
                                .SetGroupName("Network")
                                .AddConstructor<SimpleLocalClock>();
        return tid;
    }

    SimpleLocalClock() : m_time(NanoSeconds(0)) {}
    
    virtual Time Now() override { return m_time; }
    virtual void SetLocalClock(Time time) override { m_time = time; }

  private:
    Time m_time;
};

NS_OBJECT_ENSURE_REGISTERED(SimpleLocalClock);

void NodeRecvEvent(Ptr<ReplayClock> receiver, Ptr<LocalClock> receiverPt, Ptr<ReplayClock> piggybackedClock, int64_t epsilon, Time interval);

void
NodeSendEvent(Ptr<ReplayClock> sender, Ptr<ReplayClock> receiver, Ptr<LocalClock> senderPt, Ptr<LocalClock> receiverPt, int64_t epsilon, Time interval, Time delay)
{
    senderPt->SetLocalClock(Simulator::Now());
    sender->Send(epsilon, interval);

    NS_LOG_INFO(Simulator::Now().GetSeconds() << "s - Node " << sender->GetNodeId() 
                << " sending message. Bitmap: " << sender->GetBitmap());

    Ptr<ReplayClock> piggybackedClock = CreateObject<ReplayClock>();
    *piggybackedClock = *sender;

    Simulator::Schedule(delay, &NodeRecvEvent, receiver, receiverPt, piggybackedClock, epsilon, interval);
}

void
NodeRecvEvent(Ptr<ReplayClock> receiver, Ptr<LocalClock> receiverPt, Ptr<ReplayClock> piggybackedClock, int64_t epsilon, Time interval)
{
    receiverPt->SetLocalClock(Simulator::Now());
    receiver->Recv(piggybackedClock, epsilon, interval);

    NS_LOG_INFO(Simulator::Now().GetSeconds() << "s - Node " << receiver->GetNodeId() 
                << " received message. Merged Bitmap: " << receiver->GetBitmap());
    
    std::cout << "State of Node " << receiver->GetNodeId() << " clock: ";
    receiver->PrintClock(std::cout, epsilon);
    std::cout << std::endl;
}

int 
main(int argc, char* argv[])
{
    LogComponentEnable("ReplayClockExample", LOG_LEVEL_INFO);

    int64_t epsilon = 1000;
    Time interval = MicroSeconds(10);

    Ptr<SimpleLocalClock> pt0 = CreateObject<SimpleLocalClock>();
    Ptr<SimpleLocalClock> hlc0 = CreateObject<SimpleLocalClock>();
    Ptr<ReplayClock> rc0 = CreateObject<ReplayClock>();
    rc0->SetAttribute("NodeId", UintegerValue(0));
    rc0->SetAttribute("LocalClock", PointerValue(pt0));
    rc0->SetAttribute("HLC", PointerValue(hlc0));

    Ptr<SimpleLocalClock> pt1 = CreateObject<SimpleLocalClock>();
    Ptr<SimpleLocalClock> hlc1 = CreateObject<SimpleLocalClock>();
    Ptr<ReplayClock> rc1 = CreateObject<ReplayClock>();
    rc1->SetAttribute("NodeId", UintegerValue(1));
    rc1->SetAttribute("LocalClock", PointerValue(pt1));
    rc1->SetAttribute("HLC", PointerValue(hlc1));

    Simulator::Schedule(Seconds(1.0), &NodeSendEvent, rc0, rc1, pt0, pt1, epsilon, interval, Seconds(0.1));
    Simulator::Schedule(Seconds(2.0), &NodeSendEvent, rc1, rc0, pt1, pt0, epsilon, interval, Seconds(0.1));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}