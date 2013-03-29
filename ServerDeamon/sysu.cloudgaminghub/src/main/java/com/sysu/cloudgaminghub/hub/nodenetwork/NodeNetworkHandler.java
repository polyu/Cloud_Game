package com.sysu.cloudgaminghub.hub.nodenetwork;

import org.apache.mina.core.service.IoHandlerAdapter;

import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.parser.Feature;
import com.sysu.cloudgaminghub.config.Config;
import com.sysu.cloudgaminghub.hub.HubManager;
import com.sysu.cloudgaminghub.hub.nodenetwork.NodeNetworkHandler;


public class NodeNetworkHandler extends IoHandlerAdapter{
	private static Logger logger = LoggerFactory.getLogger(NodeNetworkHandler.class);
	 @Override
	    public void exceptionCaught( IoSession session, Throwable cause ) throws Exception
	    {
		 	logger.warn("Record A Exception:{}, Session will be closed",cause.getMessage());
	 		session.close(true);
	    }
	    @Override
	    public void messageReceived( IoSession session, Object message ) throws Exception
	    {
	    	NodeMessage msg=(NodeMessage)message;
	    	if(msg.getMessageType()==NodeMessage.INSTANCEREPORTMESSAGE)
	    	{
	    		logger.info("Recv a instance report");
	    		NodeReportBean b=JSON.parseObject(msg.getExtendedData(), NodeReportBean.class, Feature.AllowSingleQuotes);
	    		logger.info("Host:{}: Status:{}",b.getHostname(),b.isRunningFlag());
	    		session.setAttribute(Config.HOSTNAMEKEY, b.getHostname());
	    		HubManager manager=HubManager.getHubManager();
	    		if(manager.isNodeExisted(b.getHostname()))
	    		{
	    			manager.updateNodeStatus(b.getHostname(), b);
	    			
	    		}
	    		else
	    		{
	    			manager.insertNode(b.getHostname(), b, session);
	    		}
	    	}
	    	else if(msg.getMessageType()==NodeMessage.RUNRESPONSEMESSAGE)
	    	{
	    		
	    	}
	    	else if(msg.getMessageType()==NodeMessage.SHUTDOWNRESPONSEMESSAGE)
	    	{
	    		
	    	}
	    }
	    @Override
	    public void sessionIdle( IoSession session, IdleStatus status ) throws Exception
	    {
	    	
	    }
	    @Override
	    public void sessionCreated(IoSession session)
	    {
	    	
	    }
	    @Override
	    public void sessionClosed(IoSession session)
	    {
	    	String hostName=(String)session.getAttribute(Config.HOSTNAMEKEY);
	    	if(hostName!=null)
	    	{
	    		HubManager.getHubManager().removeNode(hostName);
	    	}
	    	session.removeAttribute(Config.HOSTNAMEKEY);
	    	session.close(true);
	    }
	    
	
}
