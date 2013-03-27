package com.sysu.cloudgaming.config;

import java.io.FileInputStream;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class Config {
	public static String VERSION="1.00 Alpha";
	public static String LOCALPROGRAMPATH="e:/150w/CloudGaming/Games";
	public static String LOCALPROGRAMXMLNAME="info.xml";
	public static String ZABBIXSERVERADDR="222.200.182.75";
	public static int ZABBIXSERVERPORT=10051;
	public static String HOSTNAME="GameHost1";
	public static int REFRESHINTEVAL=10;
	private static Logger logger = LoggerFactory.getLogger(Config.class);
	public static boolean initConfig()
	{
		try
		{
			Properties p = new Properties();
		    p.load(new FileInputStream("agentd.conf"));
		    logger.info("Server address {}",p.getProperty("ServerActive"));
		    ZABBIXSERVERADDR=p.getProperty("ServerActive");
		    logger.info("HostName {}",p.getProperty("Hostname"));
		    HOSTNAME=p.getProperty("Hostname");
		    return true;
		}
		catch(Exception e)
		{
			logger.warn("Failed to read config file");
			return false;
		}
	}
}
