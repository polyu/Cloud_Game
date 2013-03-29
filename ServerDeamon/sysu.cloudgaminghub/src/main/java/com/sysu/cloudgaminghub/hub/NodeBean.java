package com.sysu.cloudgaminghub.hub;

import org.apache.mina.core.session.IoSession;

import com.sysu.cloudgaminghub.hub.nodenetwork.NodeReportBean;

public class NodeBean {
	private String Hostname=null;
	private IoSession session=null;
	private NodeReportBean reportBean=null;
	public String getHostname() {
		return Hostname;
	}
	public void setHostname(String hostname) {
		Hostname = hostname;
	}
	public IoSession getSession() {
		return session;
	}
	public void setSession(IoSession session) {
		this.session = session;
	}
	public NodeReportBean getReportBean() {
		return reportBean;
	}
	public void setReportBean(NodeReportBean reportBean) {
		this.reportBean = reportBean;
	}

}
