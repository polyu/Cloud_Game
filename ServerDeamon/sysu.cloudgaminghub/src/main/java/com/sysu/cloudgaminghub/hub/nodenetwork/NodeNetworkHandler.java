package com.sysu.cloudgaminghub.hub.nodenetwork;

import org.apache.mina.core.service.IoHandlerAdapter;

import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.parser.Feature;
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
	    		logger.debug("{}:{}",b.getHostname(),b.isRunningFlag());
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
	    	logger.info("{} Session Init! Try to get node report from remote (Not reliable in Udp)!",session.getId());
	    	session.write(generateNodeReportRequestMessage());
	    }
	    @Override
	    public void sessionClosed(IoSession session)
	    {
	    	session.close(true);
	    }
	    
	    private HubMessage generateNodeReportRequestMessage()
	    {
	    	HubMessage message=new HubMessage();
	    	message.setMessageLength(0);
	    	message.setMessageType(HubMessage.INSTANCEREPORTREQUESTMESSAGE);
	    	return message;
	    }
}
