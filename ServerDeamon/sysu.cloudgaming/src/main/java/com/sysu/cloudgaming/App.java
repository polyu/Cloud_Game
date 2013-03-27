package com.sysu.cloudgaming;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;

import org.json.JSONObject;

import com.quigley.zabbixj.agent.ZabbixAgent;
import com.sysu.cloudgaming.config.Config;
import com.sysu.cloudgaming.node.ProgramExecutor;
import com.sysu.cloudgaming.node.ProgramManager;
import com.sysu.cloudgaming.node.ZabbixClient;



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
	
    public static void main(String[] args) throws Exception 
    {
            
            
    	ZabbixClient client=new ZabbixClient();
    	client.startClient();
		
    }
    
}
