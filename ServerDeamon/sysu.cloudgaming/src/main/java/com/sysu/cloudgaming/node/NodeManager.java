package com.sysu.cloudgaming.node;


import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


import org.jdom2.Document;
import org.jdom2.Element;
import org.jdom2.input.SAXBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sysu.cloudgaming.config.Config;
import com.sysu.cloudgaming.node.network.NodeNetwork;

public class NodeManager {
	private static NodeManager manager=null;
	private static Logger logger = LoggerFactory.getLogger(NodeManager.class);
	private Map<String,ProgramBean> programMap=new HashMap<String,ProgramBean>();
	private boolean runningFlag=false;
	private ProgramBean runningbean=null;
	private int errorCode=0;
	private Process gameProcess=null;
	private Process deamonProcess=null;
	private NodeNetwork nodeNetwork=new NodeNetwork();
	
	private class WatchDogThread extends Thread
	{
		public void run()
		{
			
			while(true)
			{
				try
				{
					sleep(5);
				}
				catch(Exception e)
				{
					
				}
				try
				{
					gameProcess.exitValue();
					logger.info("Game Process Exited");
					break;
				}
				catch(IllegalThreadStateException e)
				{
					
				}
				try
				{
					deamonProcess.exitValue();
					logger.info("Daemon Process Exited");
					break;
				}
				catch(IllegalThreadStateException e)
				{
					
				}
			}
			killAllProcess();
		}
	}
	WatchDogThread watchThread=null;
	private void killAllProcess()
	{
		try
		{
			gameProcess.destroy();
			logger.info("Game Process was killed");
		}
		catch(Exception e)
		{
			logger.warn("Try to terminate game process But Failed");
		}
		try
		{
			deamonProcess.destroy();
			logger.info("Daemon Process was killed");
		}
		catch(Exception e)
		{
			logger.warn("Try to terminate deamon process But Failed");
		}
		runningFlag=false;
		nodeNetwork.sendRunningFinishMessage(true, 0);

	}
	public static NodeManager getNodeManager()
	{
		
		if(manager==null)
		{
			manager=new NodeManager();
		}
		return manager;
	}
	private NodeManager()
	{
		
		
	}
	public ProgramBean getRunningApplicationProgramBean()
	{
		return this.runningbean;
	}
	public boolean initNodeManager()
	{
		if(!searchLocalProgram())
		{
			logger.warn("Failed to search local program!");
        	return false;
		}
		if(!nodeNetwork.setupNodeNetwork())
        {
        	logger.warn("Init Node Network Failed!");
        	return false;
        }
		return true;
	}
	public boolean isNodeRunningApplication()
	{
		return this.runningFlag;
	}
	public int getLastError()
	{
		return this.errorCode;
	}
	public boolean startApplication(String programId,int quality)
	{
		if(!executeDeamonApplication(quality))
		{
			logger.warn("Unable to init server deamon");
			return false;
		}
		if(!executeGameApplication(programId))
		{
			logger.warn("Unable to init game application");
			return false;
		}
		watchThread=new WatchDogThread();
		watchThread.start();
		return true;
	}
	private boolean executeDeamonApplication(int quality)
	{
		try
		{
			ProcessBuilder builder=new ProcessBuilder(Config.DEAMONPATH,"-q "+quality);
			gameProcess=builder.start();
			return true;
		}
		catch(Exception e)
		{
			logger.warn(e.getMessage(),e);
			return false;
		}
		
	}
	private boolean executeGameApplication(String programId)
	{
		if(!programMap.containsKey(programId))
		{
			logger.warn("Local disk don't have such game!");
			return false;
		}
		else
		{
			this.runningbean=programMap.get(programId);
			logger.info("Try to execute {} in {}",runningbean.getProgramName(),runningbean.getProgramPath());
			try
			{
				ProcessBuilder builder=new ProcessBuilder(runningbean.getProgramPath());
				deamonProcess=builder.start();
				return true;
			}
			catch(Exception e)
			{
				logger.warn(e.getMessage(),e);
				return false;
			}
		}
	}
	
	public boolean shutdownApplication()
	{
		if(!runningFlag)
		{
			logger.warn("Application is not running yet");
			return false;
		}
		else
		{
			killAllProcess();
			return true;
		}
	}
	

	
	public  boolean searchLocalProgram()
	{
		
		try
		{
			programMap.clear();
			File infoFile=new File(Config.LOCALPROGRAMPATH+"/"+Config.LOCALPROGRAMXMLNAME);
			if(infoFile.exists())
			{
				SAXBuilder builder=new SAXBuilder();
				
					Document doc=builder.build(infoFile);
					Element info=doc.getRootElement();
					List<Element> games=info.getChildren("program");
					logger.info("Local disk have {} game",games.size());
					for(Element g: games)
					{
						ProgramBean b=new ProgramBean();
						b.setProgramID(g.getChildText("id"));
						b.setProgramName(g.getChildText("name"));
						b.setProgramVersion(g.getChildText("ver"));
						b.setProgramPath(Config.LOCALPROGRAMPATH+'/'+g.getChildText("path"));
						logger.info("Add Game to Map Id:{}, Name:{}",b.getProgramID(),b.getProgramName());
						programMap.put(b.getProgramID(), b);
					}
					return true;
				
				
			}
			else
			{
				logger.warn("Info File Not Existed!");
			}
			return false;
		}
		
		catch(Exception e)
		{
			logger.warn(e.getMessage(),e);
			return false;
		}
	}
	
}
