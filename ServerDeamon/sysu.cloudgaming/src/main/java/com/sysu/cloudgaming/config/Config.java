package com.sysu.cloudgaming.config;

import java.io.FileInputStream;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class Config {
	public static String VERSION="1.00 Alpha";
	public static String LOCALPROGRAMPATH="e:/150w/CloudGaming/Games";
	public static String DEAMONPATH="e:/150w/Server/Release/Server.exe";
	public static String LOCALPROGRAMXMLNAME="info.xml";
	public static String HUBSERVERADDR="127.0.0.1";
	public static int HUBSERVERPORT=10060;
	public static int CONNECT_TIMEOUT=3000;
	public static String HOSTNAME="GameHost1";
	public static int REFRESHINTEVAL=30;
	private static Logger logger = LoggerFactory.getLogger(Config.class);
	public static boolean initConfig()
	{
		try
		{
			Properties p = new Properties();
		    p.load(new FileInputStream("agentd.conf"));
		    logger.info("Server address {}",p.getProperty("ServerActive"));
		    HUBSERVERADDR=p.getProperty("ServerActive");
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
