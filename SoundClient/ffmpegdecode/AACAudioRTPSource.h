#pragma once
#ifndef _AACAUDIO_RTP_SOURCE_HH
#define _AACAUDIO_RTP_SOURCE_HH
#ifndef _MULTI_FRAMED_RTP_SOURCE_HH
#include "MultiFramedRTPSource.hh"
#endif

class AACAudioRTPSource: public MultiFramedRTPSource {
public:
  static AACAudioRTPSource* createNew(UsageEnvironment& env, Groupsock* RTPgs,
				    unsigned char rtpPayloadFormat,
				    unsigned rtpTimestampFrequency);
  // "doNormalMBitRule" means: If the medium is not audio, use the RTP "M"
  // bit on each incoming packet to indicate the last (or only) fragment
  // of a frame.  Otherwise (i.e., if "doNormalMBitRule" is False, or the medium is "audio"), the "M" bit is ignored.

protected:
  virtual ~AACAudioRTPSource();

protected:
  AACAudioRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
		  unsigned char rtpPayloadFormat,
		  unsigned rtpTimestampFrequency);
      // called only by createNew()

private:
  // redefined virtual functions:
  virtual Boolean processSpecialHeader(BufferedPacket* packet,
                                       unsigned& resultSpecialHeaderSize);
  virtual char const* MIMEtype() const;

private:
  char const* fMIMEtypeString;
};

#endif