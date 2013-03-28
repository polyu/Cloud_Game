package com.sysu.cloudgaminghub.hub;

import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import com.sysu.cloudgaminghub.hub.nodenetwork.NodeNetwork;

public class HubManager
{
	private static Logger logger = LoggerFactory.getLogger(HubManager.class);
	private static HubManager manager=null;
	private NodeNetwork nodeNetwork=new NodeNetwork();
	private Map<String,NodeBean> nodesSet=new HashMap<String,NodeBean>();
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
	public boolean initManager()
	{
		if(!nodeNetwork.setupNodeNetwork())
		{
			logger.warn("Unable to init node network");
			return false;
		}
		return true;
	}
	
};