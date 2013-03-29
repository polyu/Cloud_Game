package com.sysu.cloudgaminghub.hub;

import java.util.HashMap;
import java.util.Iterator;

import java.util.Map;

import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import com.sysu.cloudgaminghub.hub.nodenetwork.NodeNetwork;
import com.sysu.cloudgaminghub.hub.nodenetwork.NodeReportBean;
import com.sysu.cloudgaminghub.hub.portalnetwork.PortalNetwork;

public class HubManager
{
	private static Logger logger = LoggerFactory.getLogger(HubManager.class);
	private static HubManager manager=null;
	private NodeNetwork nodeNetwork=new NodeNetwork();
	private PortalNetwork portalNetwork=new PortalNetwork();
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
	 * Find A Free Node And Send Request!
	 */
	public synchronized boolean sendPlayRequest()
	{
		NodeBean b=findFreeNode();
		if(b==null)
		{
			logger.warn("Cluster Busying");
			return false;
		}
		return false;
		
		
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
	public synchronized boolean updateNodeStatus(String hostName,NodeReportBean b)
	{
		boolean isRunning=b.isRunningFlag();
		boolean inBusySet=busyNodesSet.containsKey(hostName);
		if(isRunning)
		{
			if(inBusySet)
			{
				logger.info("Update A running Node in busy set {}",hostName);
				busyNodesSet.get(hostName).setReportBean(b);
			}
			else
			{
				logger.info("Remove a node from free set and put it into busy set {}",hostName);
				NodeBean tmpBean=freeNodesSet.remove(hostName);
				tmpBean.setReportBean(b);
				busyNodesSet.put(hostName,tmpBean);
			}
		}
		else
		{
			if(inBusySet)
			{
				logger.info("Remove a node from busy set and put it into free set {}",hostName);
				NodeBean tmpBean=busyNodesSet.remove(hostName);
				tmpBean.setReportBean(b);
				freeNodesSet.put(hostName,tmpBean);
			}
			else
			{
				logger.info("Update A free Node in free set {}",hostName);
				freeNodesSet.get(hostName).setReportBean(b);
			}
		}
		return true;
		
		
	}
	public synchronized boolean insertNode(String hostName,NodeReportBean b,IoSession session)
	{
		NodeBean bean=new NodeBean();
		bean.setSession(session);
		bean.setHostname(hostName);
		bean.setReportBean(b);
		if(b.isRunningFlag())
		{
			logger.info("Insert A Node into busy set {}",bean.getHostname());
			busyNodesSet.put(hostName, bean);
		}
		else
		{
			logger.info("Insert A Node into free set {}",bean.getHostname());
			freeNodesSet.put(hostName, bean);
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
	public boolean initManager()
	{
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