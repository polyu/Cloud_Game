package com.sysu.cloudgaming.node.network;

import java.net.InetSocketAddress;
import org.apache.mina.core.future.ConnectFuture;
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
	public boolean setupNodeNetwork()
	{
		connector = new NioSocketConnector();
	    connector.setConnectTimeoutMillis(Config.CONNECT_TIMEOUT);
	    connector.setHandler(new NodeNetworkHandler());
	    connector.getFilterChain().addLast("protocol", new ProtocolCodecFilter(new NetworkCodecFactory()));
	    try
	    {
	    	logger.info("Connecting hub server");
	    	ConnectFuture future = connector.connect(new InetSocketAddress(Config.HUBSERVERADDR,Config.HUBSERVERPORT));
            future.awaitUninterruptibly();
            if(future.isConnected())
            {
            	session=future.getSession();
	            logger.info("Got hub server");
	            return true;
            }
            else
            {
            	logger.info("Connected hub Failed\n");
            	connector.dispose();
            	return false;
            }
	    }
	    catch(Exception e)
	    {
	    	logger.info("Connected hub Failed\n",e);
	    	connector.dispose();
	    	return false;
	    }
	}
}
