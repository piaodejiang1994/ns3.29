#include <iostream>
#include <sstream>
#include <fstream>

#include <string>
#include <cassert>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/ipv4-flow-classifier.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestMeshScript");

class MeshTest
{
public:
  MeshTest ();
  void Configure (int argc, char ** argv);
  int Run ();
private:
  int       m_xSize; ///< X size
  int       m_ySize; ///< Y size
  double    m_step; ///< step
  double    m_randomStart; ///< random start
  double    m_totalTime; ///< total time
  double    m_packetInterval; ///< packet interval
  uint16_t  m_packetSize; ///< packet size
  uint32_t  m_nIfaces; ///< number interfaces
  bool      m_chan; ///< channel
  bool      m_pcap; ///< PCAP
  bool      m_ascii; ///< ASCII
  std::string m_stack; ///< stack
  std::string m_root; ///< root
  NodeContainer nodes;
  NetDeviceContainer meshDevices;
  Ipv4InterfaceContainer interfaces_nodes;
  MeshHelper mesh;
private:
  void CreateNodes ();
  void InstallInternetStack ();
  void InstallApplication ();
  void Report ();
};
MeshTest::MeshTest () :
  m_xSize (5),
  m_ySize (5),
  m_step (10.0),
  m_randomStart (0.1),
  m_totalTime (5.0),
  m_packetInterval (0.0005),
  m_packetSize (256),
  m_nIfaces (1),
  m_chan (false),
  m_pcap (true),
  m_ascii (false),
  m_stack ("ns3::Dot11sStack"),
  m_root ("ff:ff:ff:ff:ff:ff")
{
}
void
MeshTest::Configure (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("x-size", "Number of nodes in a row grid", m_xSize);
  cmd.AddValue ("y-size", "Number of rows in a grid", m_ySize);
  cmd.AddValue ("step",   "Size of edge in our grid (meters)", m_step);
  cmd.AddValue ("start",  "Maximum random start delay for beacon jitter (sec)", m_randomStart);
  cmd.AddValue ("time",  "Simulation time (sec)", m_totalTime);
  cmd.AddValue ("packet-interval",  "Interval between packets in UDP ping (sec)", m_packetInterval);
  cmd.AddValue ("packet-size",  "Size of packets in UDP ping (bytes)", m_packetSize);
  cmd.AddValue ("interfaces", "Number of radio interfaces used by each mesh point", m_nIfaces);
  cmd.AddValue ("channels",   "Use different frequency channels for different interfaces", m_chan);
  cmd.AddValue ("pcap",   "Enable PCAP traces on interfaces", m_pcap);
  cmd.AddValue ("ascii",   "Enable Ascii traces on interfaces", m_ascii);
  cmd.AddValue ("stack",  "Type of protocol stack. ns3::Dot11sStack by default", m_stack);
  cmd.AddValue ("root", "Mac address of root mesh point in HWMP", m_root);
  cmd.Parse (argc, argv);
  NS_LOG_DEBUG ("Grid:" << m_xSize << "*" << m_ySize);
  NS_LOG_DEBUG ("Simulation time: " << m_totalTime << " s");
  if (m_ascii)
    {
      PacketMetadata::Enable ();
    }
}
void
MeshTest::CreateNodes ()
{
  nodes.Create (m_ySize*m_xSize);
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  mesh = MeshHelper::Default ();
  if (!Mac48Address (m_root.c_str ()).IsBroadcast ())
    {
      mesh.SetStackInstaller (m_stack, "Root", Mac48AddressValue (Mac48Address (m_root.c_str ())));
    }
  else
    {
      //If root is not set, we do not use "Root" attribute, because it
      //is specified only for 11s
      mesh.SetStackInstaller (m_stack);
    }
  if (m_chan)
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
    }
  else
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
    }
  mesh.SetMacType ("RandomStart", TimeValue (Seconds (m_randomStart)));
  // Set number of interfaces - default is single-interface mesh point
  mesh.SetNumberOfInterfaces (m_nIfaces);
  // Install protocols and return container if MeshPointDevices
  meshDevices = mesh.Install (wifiPhy, nodes);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
		  	  	  	  	  	  	  	  	   "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
		                                   "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               	   	   	   "Mode", StringValue ("Time"),//(Time|Distance)The mode indicates the condition used to change the current speed and direction
										   "Time", StringValue ("2s"),//Change current direction and speed after moving for this delay.
										   "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
										   "Bounds", StringValue ("0|100|0|100"));//Bounds of the area to cruise.
  mobility.Install (nodes);

  if (m_pcap)
  	{
	  wifiPhy.EnablePcapAll (std::string ("mp-"));
  	}
  if (m_ascii)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("mesh.tr"));
    }
}
void
MeshTest::InstallInternetStack ()
{
  AodvHelper aodv;
  InternetStackHelper internetStack_nodes,internetStack_usernodes;
  internetStack_nodes.SetRoutingHelper (aodv);
  internetStack_nodes.Install (nodes);
  Ipv4AddressHelper address_nodes;
  address_nodes.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces_nodes = address_nodes.Assign (meshDevices);
}
void
MeshTest::InstallApplication ()
{
	int port_QQ = 4000;
  UdpEchoServerHelper echoServer (port_QQ);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (12));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (m_totalTime));

  UdpEchoClientHelper echoClient (interfaces_nodes.GetAddress (12), port_QQ);
  echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_totalTime*(1/m_packetInterval))));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));

  ApplicationContainer usernodesAppsArray[25];
  for(int i = 0;i < 25;i++)
  {
	  usernodesAppsArray[i] = echoClient.Install (nodes.Get(i));
	  usernodesAppsArray[i].Start (Seconds (0.0 + i*0.001));
	  usernodesAppsArray[i].Stop (Seconds (m_totalTime));
  }

/*
    int port = 9;
  	OnOffHelper clientOnOffHelper ("ns3::TcpSocketFactory", Address ());
  	clientOnOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  	clientOnOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  	clientOnOffHelper.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  	clientOnOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("50Mb/s")));

  	ApplicationContainer clientOnOffApps[25];
  	AddressValue remoteAddress (InetSocketAddress (interfaces_nodes.GetAddress (12), port));
  	clientOnOffHelper.SetAttribute ("Remote", remoteAddress);
  	for(int i = 0;i < 25;i++)
  	{
  		if(i!=12)
  		{
  			clientOnOffApps[i].Add (clientOnOffHelper.Install (nodes.Get (i)));
  			clientOnOffApps[i].Start (Seconds (0.1+i*0.001));
  			clientOnOffApps[i].Stop (Seconds (m_totalTime));
  		}
  	}
*/


  //ftp
  int port_ftp = 20;
   BulkSendHelper ftpApps ("ns3::TcpSocketFactory", InetSocketAddress (interfaces_nodes.GetAddress (1), port_ftp));
   // Set the amount of data to send in bytes.  Zero is unlimited.
   int maxBytes = 0;
   ftpApps.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

   ApplicationContainer sourceApps = ftpApps.Install (nodes.Get (0));
   sourceApps.Start (Seconds (0.1));
   sourceApps.Stop (Seconds (m_totalTime));


}

int
MeshTest::Run ()
{
  CreateNodes ();
  InstallInternetStack ();
  InstallApplication ();
  Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds (m_totalTime));

  AnimationInterface anim ("anim_test.xml");

  Simulator::Run ();

  flowMonitor->SerializeToXmlFile("flowMonitor_test.xml", true, true);

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
      {
        // first 2 FlowIds are for ECHO apps, we don't want to display them
        //
        // Duration for throughput measurement is 9.0 seconds, since
        //   StartTime of the OnOffApplication is at about "second 1"
        // and
        //   Simulator::Stops at "second 10".


            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
            std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
            std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
            std::cout << "  Packet average Delay:      " << i->second.delaySum / (i->second.rxPackets + 1)  << " ns\n";

      }

  Simulator::Destroy ();
  return 0;
}
void
MeshTest::Report ()
{
  unsigned n (0);
  for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i, ++n)
    {
      std::ostringstream os;
      os << "mp-report-" << n << ".xml";
      std::cerr << "Printing mesh point device #" << n << " diagnostics to " << os.str () << "\n";
      std::ofstream of;
      of.open (os.str ().c_str ());
      if (!of.is_open ())
        {
          std::cerr << "Error: Can't open file " << os.str () << "\n";
          return;
        }
      mesh.Report (*i, of);
      of.close ();
    }
}
int
main (int argc, char *argv[])
{
  MeshTest t;
  t.Configure (argc, argv);
  return t.Run ();
}
