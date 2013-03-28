package com.sysu.cloudgaming.node.network;

public class NodeMessage {
	public final static int INSTANCEREPORTMESSAGE=1;
	public final static int RUNRESPONSEMESSAGE=2;
	public final static int SHUTDOWNRESPONSEMESSAGE=3;
	private int messageLength;
	private int messageType=1;
	private int machineID=0;
	private boolean success=false;
	private int errorCode=0;
	private byte extendedData[]=null;
	public int getMessageLength() {
		return messageLength;
	}
	public void setMessageLength(int messageLength) {
		this.messageLength = messageLength;
	}
	public int getMessageType() {
		return messageType;
	}
	public void setMessageType(int messageType) {
		this.messageType = messageType;
	}
	public int getMachineID() {
		return machineID;
	}
	public void setMachineID(int machineID) {
		this.machineID = machineID;
	}
	public boolean isSuccess() {
		return success;
	}
	public void setSuccess(boolean success) {
		this.success = success;
	}
	public int getErrorCode() {
		return errorCode;
	}
	public void setErrorCode(int errorCode) {
		this.errorCode = errorCode;
	}
	public byte[] getExtendedData() {
		return extendedData;
	}
	public void setExtendedData(byte[] extendedData) {
		this.extendedData = extendedData;
	}
	
}
