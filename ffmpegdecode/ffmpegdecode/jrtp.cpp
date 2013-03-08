#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpmemorymanager.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
using namespace jrtplib;


void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}
int main(void)
{

	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
	RTPSession session;
	session.SetDefaultPayloadType(96);
	session.SetDefaultMark(false);
	session.SetDefaultTimestampIncrement(160);
	RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0/8000.0);
	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(1234);
	int status = session.Create(sessionparams,&transparams);
	if (status < 0)
	{
		std::cerr << RTPGetErrorString(status) << std::endl;
		exit(-1);
	}
	printf("Create");
	while(true)
	{
		
		Sleep(10);
		
		session.BeginDataAccess();
		if (session.GotoFirstSource())
		 {
			do
			{
				
			    RTPPacket *packet;
                while ((packet = session.GetNextPacket()) != 0)
                {
					
                        std::cout << "Got packet with extended sequence number " 
                                  << packet->GetExtendedSequenceNumber() 
                                          << " from SSRC " << packet->GetSSRC() 
                                          << std::endl;
                        session.DeletePacket(packet);
                }
			} while (session.GotoNextSource());
		}
		session.EndDataAccess();
		
	}
}