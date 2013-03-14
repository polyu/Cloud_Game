#include "AACAudioRTPSource.h"
#include <string.h>

AACAudioRTPSource*
AACAudioRTPSource::createNew(UsageEnvironment& env,
			   Groupsock* RTPgs,
			   unsigned char rtpPayloadFormat,
			   unsigned rtpTimestampFrequency
			  ) 
{
  return new AACAudioRTPSource(env, RTPgs, rtpPayloadFormat,
			     rtpTimestampFrequency
			 );
}

AACAudioRTPSource
::AACAudioRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
		  unsigned char rtpPayloadFormat,
		  unsigned rtpTimestampFrequency)
  : MultiFramedRTPSource(env, RTPgs,
			 rtpPayloadFormat, rtpTimestampFrequency)
{
	fMIMEtypeString="audio/AAC";
}

AACAudioRTPSource::~AACAudioRTPSource() {
  delete[] (char*)fMIMEtypeString;
}

Boolean AACAudioRTPSource
::processSpecialHeader(BufferedPacket* packet,
		       unsigned& resultSpecialHeaderSize) {
  fCurrentPacketCompletesFrame
    = true;
  
  resultSpecialHeaderSize=0;
  return True;
}

char const* AACAudioRTPSource::MIMEtype() const {
  if (fMIMEtypeString == NULL) return MultiFramedRTPSource::MIMEtype();

  return fMIMEtypeString;
}
