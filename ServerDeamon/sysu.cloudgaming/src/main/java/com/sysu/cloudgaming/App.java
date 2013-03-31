package com.sysu.cloudgaming;


import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;

import org.slf4j.Logger;

import org.slf4j.LoggerFactory;

import com.sysu.cloudgaming.config.Config;
import com.sysu.cloudgaming.node.NodeManager;







public class App {
	private static Logger logger = LoggerFactory.getLogger(App.class);
    public static void main(String[] args) throws Exception 
    {
    
    	NodeManager manager=NodeManager.getNodeManager();
    	if(!manager.initNodeManager())
    	{
    		logger.warn("Unable to init node manager");
    	}
    	else
    	{
    	
    		logger.info("System ready to start");
    	}
    	
    	
        
		
    }
    
}