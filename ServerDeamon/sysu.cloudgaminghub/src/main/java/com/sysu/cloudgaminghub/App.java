package com.sysu.cloudgaminghub;

import org.slf4j.Logger;

import org.slf4j.LoggerFactory;

import com.sysu.cloudgaminghub.hub.HubManager;


/**
 * Hello world!
 *
 */
public class App 
{
   
    	private static Logger logger = LoggerFactory.getLogger(App.class);
        public static void main( String[] args )
        {
        	
        	HubManager manager=HubManager.getHubManager();
        	if(manager.initManager())
        	{
        		logger.info("Hub System Start");
        	}
        	else
        	{
        		logger.warn("Hub System Failed");
        	}
        	
        }
    
}
