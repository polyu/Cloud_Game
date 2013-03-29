package com.sysu.cloudgaming.node.network;

import java.net.InetSocketAddress;
import org.apache.mina.core.future.ConnectFuture;
import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolCodecFilter;
import org.apache.mina.transport.socket.nio.NioSocketConnector;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sysu.cloudgaming.config.Config;

public class NodeNetwork {
	NioSocketConnector connector=null;
	private static Logger logger = LoggerFactory.getLogger(NodeNetwork.class);
	private IoSession session=null;
	public IoSession getNetworkSession()
	{
		return this.session;
	}
	public boolean sendRunningFinishMessage(boolean successful,int errorcode)
	{
		NodeMessage msg=new NodeMessage();
		msg.setMessageType(NodeMessage.RUNNINGFINISHMESSAGE);
		msg.setSuccess(successful);
		msg.setErrorCode(errorcode);
		session.write(msg);
		return true;
	}
	public boolean setupNodeNetwork()
	{
		connector = new NioSocketConnector();
		
	    connector.setConnectTimeoutMillis(Config.CONNECT_TIMEOUT);
	    connector.setHandler(new NodeNetworkHandler());
	    connector.getSessionConfig().setIdleTime( IdleStatus.BOTH_IDLE, Config.REFRESHINTEVAL );
	    connector.getFilterChain().addLast("protocol", new ProtocolCodecFilter(new NetworkCodecFactory()));
	    try
	    {
	    	logger.info("Connecting hub server");
	    	ConnectFuture future = connector.connect(new InetSocketAddress(Config.HUBSERVERADDR,Config.HUBSERVERPORT));
            future.awaitUninterruptibly();
            session=future.getSession();
            logger.info("Got hub server");
            return true;
	    }
	    catch(Exception e)
	    {
	    	logger.info("Connected hub Failed",e);
	    	connector.dispose();
	    	return false;
	    }
	}
}
