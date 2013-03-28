package com.sysu.cloudgaming.node;


import java.util.Map;

import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecuteResultHandler;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.ExecuteException;
import org.apache.commons.exec.ExecuteWatchdog;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class NodeManager {
	private static NodeManager manager=null;
	private static Logger logger = LoggerFactory.getLogger(NodeManager.class);
	private Map<String,ProgramBean> programMap=null;
	private boolean runningFlag=true;
	private ProgramBean runningbean=null;
	private int errorCode=0;
	private ExecuteWatchdog watchdog = null;
	private DefaultExecutor executor =null;
	private DefaultExecuteResultHandler resultHandler = new DefaultExecuteResultHandler()
	{
		@Override
		public void onProcessComplete(int exitValue)
		{
			logger.info("Application exit normally in {}",exitValue);
			runningFlag=false;
		}
		@Override
		public void onProcessFailed(ExecuteException e)
		{
			logger.warn("Application exit error for {}",e.getMessage());
			runningFlag=false;
			errorCode=e.getExitValue();
		}
	};
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
	public boolean initNodeManager()
	{
		programMap=ProgramUtils.searchLocalProgram();
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
	public boolean executeApplication(String programId)
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
			return this.executorProgram(runningbean.getProgramPath());
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
			try
			{
				watchdog.destroyProcess();
				if(!watchdog.killedProcess())
				{
					logger.warn("Process try to kill! But failed");
					return false;
				}
				return true;
			}
			catch(Exception e)
			{
				logger.error(e.getMessage(),e);
				return false;
			}
		}
	}
	
	private  boolean executorProgram(String pathname)
	{
		CommandLine cmdLine = CommandLine.parse(pathname);
		executor = new DefaultExecutor();
		watchdog = new ExecuteWatchdog(ExecuteWatchdog.INFINITE_TIMEOUT);
		executor.setWatchdog(watchdog);
		//executor.setExitValue(1);
		try {
			executor.execute(cmdLine,resultHandler);
			this.runningFlag=true;
			return true;
		} 
		catch (Exception e)
		{
			logger.warn(e.getMessage(),e);
			return false;
		}
		
	}
}
