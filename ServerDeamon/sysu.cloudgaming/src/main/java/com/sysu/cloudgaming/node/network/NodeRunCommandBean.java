package com.sysu.cloudgaming.node.network;

public class NodeRunCommandBean {
	public static final int HD1024_768_6M=1;
	public static final int SD800_600_4M=2;
	public static final int CD640_480_2M=3;
	public static final int LD320_240_1M=4;
	private int quality;
	private int programId;
	public int getQuality() {
		return quality;
	}
	public void setQuality(int quality) {
		this.quality = quality;
	}
	public int getProgramId() {
		return programId;
	}
	public void setProgramId(int programId) {
		this.programId = programId;
	}
}
