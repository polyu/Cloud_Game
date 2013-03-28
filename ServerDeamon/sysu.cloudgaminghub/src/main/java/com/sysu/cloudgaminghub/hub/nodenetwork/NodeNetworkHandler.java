package com.sysu.cloudgaminghub.hub.nodenetwork;

import org.apache.mina.core.service.IoHandlerAdapter;
import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

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
	       
	    }
	    @Override
	    public void sessionIdle( IoSession session, IdleStatus status ) throws Exception
	    {
	    	
	    }
	    @Override
	    public void sessionCreated(IoSession session)
	    {
	    	logger.info("{} Session Init! Try to get node report from remote!",session.getId());
	    	
	    }
	    @Override
	    public void sessionClosed(IoSession session)
	    {
	    	session.close(true);
	    }
	    
	    private HubMessage generateNodeReportRequestMessage()
	    {
	    	
	    }
}
