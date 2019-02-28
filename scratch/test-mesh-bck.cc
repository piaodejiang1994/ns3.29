#include <iostream>
#include <sstream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestMeshScript");

/**
 * \ingroup mesh
 * \brief MeshTest class
 */
class MeshTest
{
public:
  /// Init test
  MeshTest ();
  /**
   * Configure test from command line arguments
   *
   * \param argc command line argument count
   * \param argv command line arguments
   */
  void Configure (int argc, char ** argv);
  /**
   * Run test
   * \returns the test status
   */
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
  /// List of network nodes
  NodeContainer nodes, usernodes;
  NodeContainer allnodes = NodeContainer(nodes,usernodes);
  /// List of all mesh point devices
  NetDeviceContainer meshDevices, userDevices[16];
  /// Addresses of interfaces:
  Ipv4InterfaceContainer interfaces_nodes,interfaces_usernodes[16];
  /// MeshHelper. Report is not static methods
  MeshHelper mesh;
private:
  /// Create nodes and setup their mobility
  void CreateNodes ();
  /// Install internet m_stack on nodes
  void InstallInternetStack ();
  /// Install applications
  void InstallApplication ();
  /// Print mesh devices diagnostics
  void Report ();
};
MeshTest::MeshTest () :
  m_xSize (5),
  m_ySize (5),
  m_step (100.0),
  m_randomStart (0.1),
  m_totalTime (100.0),
  m_packetInterval (0.005),
  m_packetSize (1024),
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
  // Avoid starting all mesh nodes at the same time (beacons may collide)
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
  /*
   * Create m_ySize*m_xSize stations to form a grid topology
   */
  nodes.Create (m_ySize*m_xSize);
  usernodes.Create (16);

  // Configure YansWifiChannel
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  /*
   * Create mesh helper and set stack installer to it
   * Stack installer creates all needed protocols and install them to
   * mesh point device
   */
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




    std::vector<NodeContainer> nodeAdjacencyList(16);
    nodeAdjacencyList[0]=NodeContainer(usernodes.Get(0),nodes.Get(0));
    nodeAdjacencyList[1]=NodeContainer(usernodes.Get(1),nodes.Get(1));
    nodeAdjacencyList[2]=NodeContainer(usernodes.Get(2),nodes.Get(2));
    nodeAdjacencyList[3]=NodeContainer(usernodes.Get(3),nodes.Get(3));
    nodeAdjacencyList[4]=NodeContainer(usernodes.Get(4),nodes.Get(4));
    nodeAdjacencyList[5]=NodeContainer(usernodes.Get(5),nodes.Get(5));
    nodeAdjacencyList[6]=NodeContainer(usernodes.Get(6),nodes.Get(9));
    nodeAdjacencyList[7]=NodeContainer(usernodes.Get(7),nodes.Get(10));
    nodeAdjacencyList[8]=NodeContainer(usernodes.Get(8),nodes.Get(14));
    nodeAdjacencyList[9]=NodeContainer(usernodes.Get(9),nodes.Get(15));
    nodeAdjacencyList[10]=NodeContainer(usernodes.Get(10),nodes.Get(19));
    nodeAdjacencyList[11]=NodeContainer(usernodes.Get(11),nodes.Get(20));
    nodeAdjacencyList[12]=NodeContainer(usernodes.Get(12),nodes.Get(21));
    nodeAdjacencyList[13]=NodeContainer(usernodes.Get(13),nodes.Get(22));
    nodeAdjacencyList[14]=NodeContainer(usernodes.Get(14),nodes.Get(23));
    nodeAdjacencyList[15]=NodeContainer(usernodes.Get(15),nodes.Get(24));

    std::vector<PointToPointHelper> p2p(16);
    for(int i = 0; i < 16; i ++){
      p2p[i].SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));//设置带宽
      p2p[i].SetChannelAttribute ("Delay", StringValue ("5ms"));  //设置时延
    }
    std::vector<NetDeviceContainer> userDevices(16);
    for(int i = 0; i < 16; i ++){
    	userDevices[i] = p2p[i].Install(nodeAdjacencyList[i]);
    }





  // Setup mobility - static grid topology
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (m_step),
                                 "DeltaY", DoubleValue (m_step),
                                 "GridWidth", UintegerValue (m_xSize),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  if (m_pcap)
    wifiPhy.EnablePcapAll (std::string ("mp-"));
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
  internetStack_usernodes.Install (usernodes);

  Ipv4AddressHelper address_nodes;
  address_nodes.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces_nodes = address_nodes.Assign (meshDevices);


  Ipv4AddressHelper address_usernodes;
    for(uint32_t i=101; i<=116; i++)
    {
      std::ostringstream subset;
      subset<<"10.1.1."<<i+1;
      address_usernodes.SetBase(subset.str().c_str (),"255.255.255.0");
      interfaces_usernodes[i]=address_usernodes.Assign(userDevices[i]);
    }


}
void
MeshTest::InstallApplication ()
{
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (12));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (m_totalTime));

  UdpEchoClientHelper echoClient (interfaces_nodes.GetAddress (12), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_totalTime*(1/m_packetInterval))));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  //echoClient.SetAttribute ("DataRate", StringValue ("6Mb/s"));

  int trafficNodeList[] = {0,1,2,3,4,5,9,10,14,15,19,20,21,22,23,24};
  int trafficNodeListSize = sizeof(trafficNodeList)/sizeof(trafficNodeList[0]);
  ApplicationContainer clientAppsArray[trafficNodeListSize];
  for(int i = 0;i < trafficNodeListSize;i++)
  {
	  int trafficNode = trafficNodeList[i];
	  clientAppsArray[i] = echoClient.Install (nodes.Get (trafficNode));
	  clientAppsArray[i].Start (Seconds (0.0 + i*0.001));
	  clientAppsArray[i].Stop (Seconds (m_totalTime));
  }
  ApplicationContainer usernodesAppsArray;
  usernodesAppsArray = echoClient.Install (usernodes.Get (0));
  usernodesAppsArray.Start (Seconds (50));
  usernodesAppsArray.Stop (Seconds (m_totalTime));

/*
  ApplicationContainer clientApps1 = echoClient.Install (nodes.Get (0));
  clientApps1.Start (Seconds (0.0));
  clientApps1.Stop (Seconds (m_totalTime));
  ApplicationContainer clientApps2 = echoClient.Install (nodes.Get (24));
  clientApps2.Start (Seconds (0.0 + 0.001));
  clientApps2.Stop (Seconds (m_totalTime));
*/


/*
  //it uses OnOffApp to generate traffic
  int port = 9;
  OnOffHelper clientHelper1 ("ns3::UdpSocketFactory", Address ());
  clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper1.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  clientHelper1.SetAttribute ("DataRate", DataRateValue (DataRate ("6Mb/s")));

  AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress (12), port));

  ApplicationContainer clientApps1;
  clientHelper1.SetAttribute ("Remote", remoteAddress);
  clientApps1.Add (clientHelper1.Install (nodes.Get (0)));
  clientApps1.Start (Seconds (0.0));
  clientApps1.Stop (Seconds (m_totalTime));

  ApplicationContainer clientApps2;
  clientApps2.Add (clientHelper1.Install (nodes.Get (24)));
  clientApps2.Start (Seconds (10.0));
  clientApps2.Stop (Seconds (m_totalTime));
*/



}
int
MeshTest::Run ()
{
  CreateNodes ();
  InstallInternetStack ();
  InstallApplication ();
  Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
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
