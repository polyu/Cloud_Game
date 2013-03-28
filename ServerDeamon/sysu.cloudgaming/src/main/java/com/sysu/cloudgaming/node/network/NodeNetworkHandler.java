package com.sysu.cloudgaming.node.network;

import org.apache.mina.core.service.IoHandlerAdapter;

import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.parser.Feature;
import com.alibaba.fastjson.serializer.SerializerFeature;
import com.sysu.cloudgaming.config.Config;
import com.sysu.cloudgaming.node.NodeManager;




public class NodeNetworkHandler extends IoHandlerAdapter{
		private static Logger logger = LoggerFactory.getLogger(NodeNetworkHandler.class);
	 	@Override
	    public void exceptionCaught( IoSession session, Throwable cause ) throws Exception
	    {
	 		logger.warn("Record A Exception:{}, Session will be closed",cause.getMessage());
	 		//cause.printStackTrace();
	 		session.close(true);
	    }
	    @Override
	    public void messageReceived( IoSession session, Object message ) throws Exception
	    {
	    	logger.info("Recv a message from hub in session {}",session.getId());
	    	HubMessage hubMessage=(HubMessage)message;
	    	switch(hubMessage.getMessageType())
	    	{
	    		case HubMessage.INSTANCEREPORTREQUESTMESSAGE:
	    		{
	    			logger.info("Instance Report Request From Server");
	    			session.write(generateInstanceReportMessage());
	    		}
	    		break;
	    		case HubMessage.RUNREQUESTMESSAGE:
	    		{
	    			 logger.info("Run Command Request From Server");
	    			 NodeRunCommandBean b=JSON.parseObject(hubMessage.getExtendedData(), NodeRunCommandBean.class, Feature.AllowSingleQuotes);
	    			 boolean result=NodeManager.getNodeManager().startApplication(b.getProgramId(), b.getQuality());
	    			 session.write(generateRunCommandResponseMessage(result,NodeManager.getNodeManager().getLastError()));
	    		}
	    			break;
	    		case HubMessage.SHUTDOWNREQUESTMESSAGE:
	    		{
	    			logger.info("Shutdown Request From Server");
	    			boolean result=NodeManager.getNodeManager().shutdownApplication();
	    			session.write(generateShutdownCommandResponseMessage(result,NodeManager.getNodeManager().getLastError()));
	    		}
	    			break;
	    		default:
	    			logger.warn("Bad protocol found!");
	    			break;
	    	}
	    }
	    @Override
	    public void sessionIdle( IoSession session, IdleStatus status ) throws Exception
	    {
	    	logger.info("Timer to send report to the hub");
	    	session.write(generateInstanceReportMessage());
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
	    private NodeMessage generateRunCommandResponseMessage(boolean successful,int errorcode)
	    {
	    	NodeMessage msg=new NodeMessage();
	    	msg.setMessageType(NodeMessage.RUNRESPONSEMESSAGE);
	    	msg.setErrorCode(errorcode);
	    	msg.setSuccess(successful);
	    	return msg;
	    }
	    private NodeMessage generateShutdownCommandResponseMessage(boolean successful,int errorcode)
	    {
	    	NodeMessage msg=new NodeMessage();
	    	msg.setMessageType(NodeMessage.SHUTDOWNRESPONSEMESSAGE);
	    	msg.setErrorCode(errorcode);
	    	msg.setSuccess(successful);
	    	return msg;
	    }
	    private NodeMessage generateInstanceReportMessage() 
	    {
	    	NodeMessage message=new NodeMessage();
	    	message.setErrorCode(0);
	    	message.setSuccess(true);
	    	message.setMessageType(NodeMessage.INSTANCEREPORTMESSAGE);
	    	NodeReportBean b=new NodeReportBean();
	    	NodeManager manager=NodeManager.getNodeManager();
	    	b.setHostname(Config.HOSTNAME);
	    	b.setRunningFlag(manager.isNodeRunningApplication());
	    	if(manager.isNodeRunningApplication())
	    	{
	
	    		b.setRunningApplication(manager.getRunningApplicationProgramBean().getProgramVersion());
	    		b.setRunningApplication(manager.getRunningApplicationProgramBean().getProgramName());
	    		b.setRunningApplicationPath(manager.getRunningApplicationProgramBean().getProgramPath());
	    	}
	    	byte []extendData=null;
	    	try
	    	{
	    		extendData=JSON.toJSONBytes(b, SerializerFeature.UseSingleQuotes);
	    	}
	    	catch(Exception e)
	    	{
	    		logger.error(e.getMessage(),e);
	    		return null;
	    	}
	    	message.setMessageLength(extendData.length);
	    	message.setExtendedData(extendData);
	    	return message;
	    }
}