package com.sysu.cloudgaminghub.hub.nodenetwork;

import org.apache.mina.core.service.IoHandlerAdapter;

import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.alibaba.fastjson.JSON;

import com.sysu.cloudgaminghub.config.Config;
import com.sysu.cloudgaminghub.hub.HubManager;
import com.sysu.cloudgaminghub.hub.NodeBean;
import com.sysu.cloudgaminghub.hub.nodenetwork.NodeNetworkHandler;
import com.sysu.cloudgaminghub.hub.nodenetwork.bean.NodeReportBean;


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
	    		NodeReportBean b=JSON.parseObject(msg.getExtendedData(), NodeReportBean.class);
	    		logger.info("Host:{}: Status:{}",b.getHostname(),b.isRunningFlag());
	    		session.setAttribute(Config.HOSTNAMEKEY, b.getHostname());
	    		HubManager manager=HubManager.getHubManager();
	    		NodeBean nb=new NodeBean();
	    		nb.setHostname(b.getHostname());
	    		nb.setReportBean(b);
	    		nb.setRunningFlag(b.isRunningFlag());
	    		nb.setSession(session);
	    		if(manager.isNodeExisted(b.getHostname()))
	    		{
	    			manager.updateNodeStatus(b.getHostname(), nb);
	    		}
	    		else
	    		{
	    			manager.insertNode(b.getHostname(), nb);
	    		}
	    	}
	    	else if(msg.getMessageType()==NodeMessage.RUNRESPONSEMESSAGE)
	    	{
	    		String hostName=(String)session.getAttribute(Config.HOSTNAMEKEY);
	    		NodeBean b=HubManager.getHubManager().getNodeBean(hostName);
	    		if(msg.isSuccess())
	    		{
	    			
	    		}
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
