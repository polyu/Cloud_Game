package com.sysu.cloudgaming;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import com.sysu.cloudgaming.node.NodeManager;
import com.sysu.cloudgaming.node.ProgramUtils;
import com.sysu.cloudgaming.node.network.NodeNetwork;




/*
public class App 
{
	
    public static void main( String[] args )
    {
    	Config.initConfig();
    	ZabbixClient client=new ZabbixClient();
    	client.startClient();
    }
}*/
public class App {
	private static Logger logger = LoggerFactory.getLogger(App.class);
    public static void main(String[] args) throws Exception 
    {
    	NodeManager manager=NodeManager.getNodeManager();
    	if(!manager.initNodeManager())
    	{
    		logger.warn("Unable to init node manager");
    	}
    	NodeNetwork nodeNetwork=new NodeNetwork();
        if(nodeNetwork.setupNodeNetwork())
        {
        	logger.info("System Ready to start!");
        }
        else
        {
        	logger.warn("Init Node Network Failed!");
        }
		
    }
    
}
