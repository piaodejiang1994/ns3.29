/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/on-off-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/bulk-send-helper.h"

//<test library>
//#include "Eigen/Dense"
//<test library\end>

using namespace ns3;
//using namespace Eigen;


struct OneResult{
	  uint64_t m_rxBytes = 0;
	  uint64_t m_rxPackets = 0;
	  Time     m_delaySum = Seconds (0.0);
	  uint64_t m_lostPackets = 0;
	  uint64_t m_txBytes = 0;
};

double	m_totalTime = 20.0;

struct OneResult experiment (bool enableCtsRts, std::string wifiManager, uint16_t numAllNodes )
{
	/*/test
    MatrixXd m(2,2);
    m(0,0) = 3;
    m(1,0) = 2.5;
    m(0,1) = -1;
    m(1,1) = m(1,0) + m(0,1);

    std::cout << m << "\n";




	//test*/


	struct OneResult result;
  //uint16_t	numAllNodes = 2;

  // 0. Enable or disable CTS/RTS
  UintegerValue ctsThr = (enableCtsRts ? UintegerValue (100) : UintegerValue (2200));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

  // 1. Create 3 nodes
  NodeContainer nodes;
  nodes.Create (numAllNodes);

  // 2. Place nodes somehow, this is required by every wireless simulation
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
/*
  // 3. Create propagation loss matrix
  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
  lossModel->SetLoss (nodes.Get (0)->GetObject<MobilityModel> (), nodes.Get (1)->GetObject<MobilityModel> (), 50); // set symmetric loss 0 <-> 1 to 50 dB
  lossModel->SetLoss (nodes.Get (2)->GetObject<MobilityModel> (), nodes.Get (1)->GetObject<MobilityModel> (), 50); // set symmetric loss 2 <-> 1 to 50 dB

  // 4. Create & setup wifi channel
  Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
  wifiChannel->SetPropagationLossModel (lossModel);
  wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
*/
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  //wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
  // 5. Install wireless devices
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::" + wifiManager + "WifiManager");
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  //wifiPhy.SetChannel (wifiChannel);
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // uncomment the following to have athstats output
  // AthstatsHelper athstats;
  // athstats.EnableAthstats(enableCtsRts ? "rtscts-athstats-node" : "basic-athstats-node" , nodes);

  // uncomment the following to have pcap output
   wifiPhy.EnablePcap (enableCtsRts ? "rtscts-pcap-node" : "basic-pcap-node" , nodes);


  // 6. Install TCP/IP stack & assign IP addresses
  InternetStackHelper internet;
  //AodvHelper aodv;
  InternetStackHelper internetStack_nodes;
  //internetStack_nodes.SetRoutingHelper (aodv);
  internet.Install (nodes);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer interfaces;
  interfaces = ipv4.Assign (devices);

  // 7. Install applications
  //double start_time = 0.1;
  //double stop_time = m_totalTime;
/*
  uint16_t  m_packetSize = 512; ///< packet size
  double    m_packetInterval = 0.00131857;
  uint16_t port_QQ = 4000;

  //UdpEchoServerHelper echoServer (port_QQ);
  // ApplicationContainer serverApps[numAllNodes/2];

 UdpEchoClientHelper echoClient (interfaces.GetAddress (12), port_QQ);
 echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_totalTime*(1/m_packetInterval))));
 echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
 echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));

 ApplicationContainer usernodesAppsArray[numAllNodes/2];
 for(int i = 0;i < numAllNodes/2;i++)
 {
	 //serverApps[i] = echoServer.Install (nodes.Get (i));
	  //serverApps[i].Start (Seconds (0.01 + i*0.0001));
	  //serverApps[i].Stop (Seconds (m_totalTime));

	 echoClient.SetAttribute ("RemoteAddress", AddressValue (interfaces.GetAddress (i)));
	  usernodesAppsArray[i] = echoClient.Install (nodes.Get(i+numAllNodes/2));
	  usernodesAppsArray[i].Start (Seconds (0.0 + i*0.0001));
	  usernodesAppsArray[i].Stop (Seconds (m_totalTime));
 }
*/
/*
   uint64_t port_ftp = 20;
      BulkSendHelper ftpApps ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (0), port_ftp));
      // Set the amount of data to send in bytes.  Zero is unlimited.
      //uint64_t maxBytes = 0;
      //uint64_t sendSize = 256;
      //ftpApps.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
      //ftpApps.SetAttribute ("SendSize", UintegerValue (sendSize));
      ApplicationContainer sourceApps = ftpApps.Install (nodes.Get (1));
      sourceApps.Start (Seconds (0.012345));
      sourceApps.Stop (Seconds (m_totalTime));
*/

  int port = 666;
	OnOffHelper clientOnOffHelper ("ns3::UdpSocketFactory", Address ());
	clientOnOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	clientOnOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	clientOnOffHelper.SetAttribute ("PacketSize", UintegerValue (512));
	clientOnOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("2.2Mb/s")));


	ApplicationContainer clientOnOffApps[numAllNodes/2];
	//AddressValue remoteAddress;

	for(int i = 0;i < numAllNodes/2;i++)
	{
		AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress (i+numAllNodes/2), port));
		clientOnOffHelper.SetAttribute ("Remote", remoteAddress);
		clientOnOffApps[i].Add (clientOnOffHelper.Install (nodes.Get (i)));
		clientOnOffApps[i].Start (Seconds (0.01+i*0.0001));
		clientOnOffApps[i].Stop (Seconds (m_totalTime));
	}

  // 8. Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // 9. Run simulation for m_totalTime
  Simulator::Stop (Seconds (m_totalTime));
  //AnimationInterface anim ("anim_test.xml");
  Simulator::Run ();

  // 10. Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
/*
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
          std::cout << "  Delay:      " << i->second.delaySum / (i->second.rxPackets)  << " per packet\n";
          std::cout << "  LostPackets:" << i->second.lostPackets  << "\n";
          std::cout << "  LostRate:   " << i->second.lostPackets/(i->second.lostPackets+i->second.rxPackets)  << "\n";
    }
*/
  	result.m_rxBytes = 0;
  	result.m_rxPackets = 0;
  	result.m_delaySum = Seconds (0.0);
  	result.m_lostPackets = 0;
  	result.m_txBytes = 0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
      {
	  	  result.m_rxBytes += i->second.rxBytes;
	  	  result.m_rxPackets += i->second.rxPackets;
	  	  result.m_delaySum += i->second.delaySum;
	  	  result.m_lostPackets += i->second.lostPackets;
	  	  result. m_txBytes += i->second.txBytes;
      }
  /*
  	  std::cout << "------Average quantity each pair------\n";
  	  std::cout << "  Tx Bytes:   " << result.m_txBytes/numAllNodes*2 << "\n";
      std::cout << "  TxOffered:  " << result.m_txBytes/numAllNodes*2 * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
      std::cout << "  Rx Packets: " << result.m_rxPackets/numAllNodes*2 << "\n";
      std::cout << "  Rx Bytes:   " << result.m_rxBytes/numAllNodes*2 << "\n";
      std::cout << "  LostPackets:" << result.m_lostPackets/numAllNodes*2  << "\n";
      std::cout << "--------Performance indicators--------\n";
      std::cout << "  Throughput: " << result.m_rxBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
      std::cout << "  Delay:      " << result.m_delaySum / result.m_rxPackets  << " per packet\n";
      std::cout << "  LostRate:   " << 1.0*result.m_lostPackets/(result.m_lostPackets+result.m_rxPackets)  << "\n";
      */
  // 11. Cleanup
  Simulator::Destroy ();
  return result;
}

int main (int argc, char **argv)
{
  std::string wifiManager ("Arf");
  CommandLine cmd;
  cmd.AddValue ("wifiManager", "Set wifi rate manager (Aarf, Aarfcd, Amrr, Arf, Cara, Ideal, Minstrel, Onoe, Rraa)", wifiManager);
  cmd.Parse (argc, argv);

  int numTrials = 10;

  for(int i=1;i<=7;i++)
  {
	  int numAllNodes = 4*i;
	  std::cout << "************************************************\n";
	  std::cout << "**                   "<< numAllNodes <<" nodes                 **\n";
	  std::cout << "************************************************\n";
	  std::cout << "Hidden station experiment with RTS/CTS disabled:\n" << std::flush;
	  struct OneResult result[numTrials],aver;
	  for(int j=0;j<numTrials;j++)
	  {
		  result[j]=experiment (false, wifiManager, numAllNodes);
	  }
	  aver.m_rxBytes = 0;
	  aver.m_rxPackets = 0;
	  aver.m_delaySum = Seconds (0.0);
	  aver.m_lostPackets = 0;
	  aver.m_txBytes = 0;
	  for(int j=0;j<numTrials;j++)
	  {
		  aver.m_rxBytes += result[j].m_rxBytes;
		  aver.m_rxPackets += result[j].m_rxPackets;
		  aver.m_delaySum += result[j].m_delaySum;
		  aver.m_lostPackets += result[j].m_lostPackets;
		  aver.m_txBytes += result[j].m_txBytes;
  	  }
	  aver.m_rxBytes /= numTrials;
	  aver.m_rxPackets /= numTrials;
	  aver.m_delaySum = aver.m_delaySum/numTrials;
	  aver.m_lostPackets /= numTrials;
	  aver.m_txBytes /= numTrials;
	  std::cout << "  Throughput: " << aver.m_rxBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
	  std::cout << "  Delay:      " << aver.m_delaySum / aver.m_rxPackets  << " per packet\n";
	  std::cout << "  LostRate:   " << 1.0*aver.m_lostPackets/(aver.m_lostPackets+aver.m_rxPackets)  << "\n";
      std::cout << "------------------------------------------------\n";
  }

  for(int i=1;i<=7;i++)
    {
  	  int numAllNodes = 4*i;
  	  std::cout << "************************************************\n";
  	  std::cout << "**                   "<< numAllNodes <<" nodes                 **\n";
  	  std::cout << "************************************************\n";
  	std::cout << "Hidden station experiment with RTS/CTS enabled:\n" << std::flush;

  	  struct OneResult result[numTrials],aver;
  	  for(int j=0;j<numTrials;j++)
  	  {
  		result[j]=experiment (true, wifiManager, numAllNodes);
  	  }
  	  aver.m_rxBytes = 0;
  	  aver.m_rxPackets = 0;
  	  aver.m_delaySum = Seconds (0.0);
  	  aver.m_lostPackets = 0;
  	  aver.m_txBytes = 0;
  	  for(int j=0;j<numTrials;j++)
  	  {
  		  aver.m_rxBytes += result[j].m_rxBytes;
  		  aver.m_rxPackets += result[j].m_rxPackets;
  		  aver.m_delaySum += result[j].m_delaySum;
  		  aver.m_lostPackets += result[j].m_lostPackets;
  		  aver.m_txBytes += result[j].m_txBytes;
    	  }
  	  aver.m_rxBytes /= numTrials;
  	  aver.m_rxPackets /= numTrials;
  	  aver.m_delaySum = aver.m_delaySum/numTrials;
  	  aver.m_lostPackets /= numTrials;
  	  aver.m_txBytes /= numTrials;
  	  std::cout << "  Throughput: " << aver.m_rxBytes * 8.0 / m_totalTime / 1000 / 1000  << " Mbps\n";
  	  std::cout << "  Delay:      " << aver.m_delaySum / aver.m_rxPackets  << " per packet\n";
  	  std::cout << "  LostRate:   " << 1.0*aver.m_lostPackets/(aver.m_lostPackets+aver.m_rxPackets)  << "\n";
      std::cout << "------------------------------------------------\n";
    }
  return 0;
}
