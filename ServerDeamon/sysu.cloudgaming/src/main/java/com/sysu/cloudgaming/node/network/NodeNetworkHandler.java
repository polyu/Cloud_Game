package com.sysu.cloudgaming.node.network;

import org.apache.mina.core.service.IoHandlerAdapter;
import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


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
	    	logger.info("Recv a message from hub in session {}\n",session.getId());
	    	HubMessage hubMessage=(HubMessage)message;
	    	switch(hubMessage.getMessageType())
	    	{
	    		case HubMessage.INSTANCEREPORTREQUESTMESSAGE:
	    			break;
	    		case HubMessage.RUNREQUESTMESSAGE:
	    			break;
	    		case HubMessage.SHUTDOWNREQUESTMESSAGE:
	    			break;
	    		default:
	    			logger.warn("Bad protocol found!");
	    			break;
	    	}
	    }
	    @Override
	    public void sessionIdle( IoSession session, IdleStatus status ) throws Exception
	    {
	    
	    }
	    @Override
	    public void sessionClosed(IoSession session)
	    {
	    	session.close(true);
	    }
	    
	    @Override 
	    public void sessionCreated(IoSession session)
	    {
	    	
	    }
}