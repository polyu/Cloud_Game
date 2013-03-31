package com.sysu.cloudgaming.node.network.upnp;

import java.net.InetAddress;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;



public class UPNPManager {
	private static Logger logger = LoggerFactory.getLogger(UPNPManager.class);
	GatewayDiscover discover =null;
	GatewayDevice d=null;
	InetAddress localAddress=null;
	String externalIPAddress=null;
	public boolean initManager()
	{
		try
		{
			discover = new GatewayDiscover();
	        logger.info("Looking for Gateway Devices");
	        discover.discover();
	        d = discover.getValidGateway();
	
	        if (null != d) {
	            logger.info("Gateway device found.{} ({})", new Object[]{d.getModelName(), d.getModelDescription()});
	        } else {
	            logger.info("No valid gateway device found.");
	            return false;
	        }
	        localAddress = d.getLocalAddress();
	        logger.info("Using local address: {}", localAddress);
	        String externalIPAddress = d.getExternalIPAddress();
	        logger.info("External address: {}", externalIPAddress);
	        return true;
		}
		catch(Exception e)
		{
			logger.warn(e.getMessage(),e);
			return false;
		}
	}
	private boolean setupUPNPMapping(int localPort,int outBoundPort)
	{
		try
		{
	        PortMappingEntry portMapping = new PortMappingEntry();
	        logger.info("Attempting to map port {}---{}", localPort,outBoundPort);
	        if (!d.getSpecificPortMappingEntry(outBoundPort,"UDP",portMapping)) {
	            logger.info("Sending port mapping request");
	            if (d.addPortMapping(outBoundPort,localPort,localAddress.getHostAddress(),"UDP","ForCloudGaming"))
	            {
	                logger.info("Mapping succesful");
	                return true;
	            } else 
	            {
	                logger.info("Port mapping failed");
	                return false;
	            }
	            
	        } 
	        else 
	        {
	            logger.info("Port was already mapped. Aborting test.");
	            return false;
	        }

		}
		catch(Exception e)
		{
			logger.warn("Failed to open upnp device",e);
			return false;
		}

	}
	/*
	 * UPNP UNHOLE FUNCTION
	 */
	private boolean removeUPNPPort(int outboundPort)
	{
		try
		{
	        d.deletePortMapping(outboundPort,"UDP");
	        return true;
		}
		catch(Exception e)
		{
			logger.info(e.getMessage(),e);
			return false;
		}
	}
}
