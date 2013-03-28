package com.sysu.cloudgaminghub;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sysu.cloudgaminghub.hub.nodenetwork.NodeNetwork;


/**
 * Hello world!
 *
 */
public class App 
{
   
    	private static Logger logger = LoggerFactory.getLogger(App.class);
        public static void main( String[] args )
        {
        	logger.info("System Start");
        	NodeNetwork instanceNetwork=new NodeNetwork();
        	instanceNetwork.setupInstanceNetwork();
        }
    
}
