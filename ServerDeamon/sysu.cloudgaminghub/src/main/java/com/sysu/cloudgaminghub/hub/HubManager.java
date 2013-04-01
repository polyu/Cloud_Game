package com.sysu.cloudgaminghub.hub;

import java.util.HashMap;
import java.util.Iterator;

import java.util.Map;

import org.apache.mina.core.session.IoSession;
import org.eclipse.jetty.continuation.Continuation;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import com.alibaba.fastjson.JSON;
import com.sysu.cloudgaminghub.hub.nodenetwork.HubMessage;
import com.sysu.cloudgaminghub.hub.nodenetwork.NodeNetwork;
import com.sysu.cloudgaminghub.hub.nodenetwork.bean.NodeReportBean;
import com.sysu.cloudgaminghub.hub.nodenetwork.bean.NodeRunRequestBean;
import com.sysu.cloudgaminghub.hub.portalnetwork.PortalNetwork;
import com.sysu.cloudgaminghub.stun.StunServer;

public class HubManager
{
	private static Logger logger = LoggerFactory.getLogger(HubManager.class);
	private static HubManager manager=null;
	private NodeNetwork nodeNetwork=new NodeNetwork();
	private PortalNetwork portalNetwork=new PortalNetwork();
	private StunServer stunServer =new StunServer();
	private Map<String,NodeBean> freeNodesSet=new HashMap<String,NodeBean>();
	private Map<String,NodeBean> busyNodesSet=new HashMap<String,NodeBean>();
	public static HubManager getHubManager()
	{
		if(manager==null)
		{
			manager=new HubManager();
		}
		return manager;
	}
	private HubManager()
	{
		
	}
	/*
	 * Free Status Report
	 */
	public synchronized int getFreeNodesNum()
	{
		return freeNodesSet.size();
	}
	/*
	 * Busy Status Report
	 */
	public synchronized int getBusyNodesNum()
	{
		return busyNodesSet.size();
	}
	/*
	 * Find A Free Node And Send Request!
	 */
	public synchronized boolean sendPlayRequest(NodeRunRequestBean rb,Continuation continuation)
	{
		NodeBean b=findFreeNode();
		if(b==null)
		{
			logger.warn("Cluster Busying");
			return false;
		}
		freeNodesSet.remove(b.getHostname());
		busyNodesSet.put(b.getHostname(), b);
		b.setRunningFlag(true);
		b.setContinuation(continuation);
		IoSession session=b.getSession();
		HubMessage message=new HubMessage();
		message.setMessageType(HubMessage.RUNREQUESTMESSAGE);
		byte extendedData[]=JSON.toJSONBytes(rb);
		message.setExtendedData(extendedData);
		message.setMessageLength(extendedData.length);
		session.write(message);
		return true;
		
	}
	/*
	 * Most Easy Way
	 */
	private NodeBean findFreeNode()
	{
		if(freeNodesSet.size()==0)
		{
			logger.warn("Not more free nodes");
			return null;
		}
		Iterator<NodeBean> it=freeNodesSet.values().iterator();
		return it.next();
	}
	public synchronized boolean removeNode(String hostName)
	{
		boolean inBusySet=busyNodesSet.containsKey(hostName);
		if(inBusySet)
		{
			busyNodesSet.remove(hostName);
		}
		else
		{
			freeNodesSet.remove(hostName);
		}
		logger.info("Remove node {} from sets",hostName);
		return true;
	}
	public synchronized boolean updateNodeStatus(String hostName,NodeBean b)
	{
		boolean isRunning=b.isRunningFlag();
		boolean inBusySet=busyNodesSet.containsKey(hostName);
		if(isRunning)
		{
			if(inBusySet)
			{
				logger.info("Update A running Node in busy set {}",hostName);
				busyNodesSet.put(hostName, b);
			}
			else
			{
				logger.info("Remove a node from free set and put it into busy set {}",hostName);
				NodeBean tmpBean=freeNodesSet.remove(hostName);
				busyNodesSet.put(hostName,tmpBean);
			}
		}
		else
		{
			if(inBusySet)
			{
				logger.info("Remove a node from busy set and put it into free set {}",hostName);
				NodeBean tmpBean=busyNodesSet.remove(hostName);
				
				freeNodesSet.put(hostName,tmpBean);
			}
			else
			{
				logger.info("Update A free Node in free set {}",hostName);
				freeNodesSet.put(hostName, b);
			}
		}
		return true;
		
		
	}
	public synchronized boolean insertNode(String hostName,NodeBean b)
	{
		
		if(b.isRunningFlag())
		{
			logger.info("Insert A Node into busy set {}",b.getHostname());
			busyNodesSet.put(hostName, b);
		}
		else
		{
			logger.info("Insert A Node into free set {}",b.getHostname());
			freeNodesSet.put(hostName, b);
		}
		return true;
	}
	public synchronized boolean isNodeExisted(String hostName)
	{
		if(freeNodesSet.containsKey(hostName))
		{
			return true;
		}
		if(busyNodesSet.containsKey(hostName))
		{
			return true;
		}
		return false;
	}
	public synchronized NodeBean getNodeBean(String hostName)
	{
		if(freeNodesSet.containsKey(hostName))
		{
			return freeNodesSet.get(hostName);
		}
		return busyNodesSet.get(hostName);
	}
	public boolean initManager()
	{
		/*if(!stunServer.startStunServer())
		{
			logger.warn("Unable to init stun network");
			return false;
		}*/
		if(!nodeNetwork.setupNodeNetwork())
		{
			logger.warn("Unable to init node network");
			return false;
		}
		if(!portalNetwork.startPortalNetwork())
		{
			logger.warn("Unable to start portal network");
			return false;
		}
		return true;
	}
	
};