
//参数：1.RTP包缓冲地址 2.RTP包数据大小 3.H264输出地址 4.输出数据大小
//返回：true:表示一帧结束  false:帧未结束 一般AAC音频包比较小，没有分片。
bool UnpackRTPAAC(void * bufIn, int recvLen, void** pBufOut,  int* pOutLen)
{
    unsigned char*  bufRecv = (unsigned char*)bufIn;
    //char strFileName[20];
    unsigned char ADTS[] = {0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0xFC}; 
    int audioSamprate = 44100;//音频采样率
    int audioChannel = 2;//音频声道 1或2
    int audioBit = 16;//16位 固定
    ADTS[2] = 0x50;
    ADTS[3] = 0x80;
    int len = recvLen - 16 + 7;
    len <<= 5;//8bit * 2 - 11 = 5(headerSize 11bit)
    len |= 0x1F;//5 bit    1            
    ADTS[4] = len>>8;
    ADTS[5] = len & 0xFF;
    *pBufOut = (char*)bufIn+16-7;
    memcpy(*pBufOut, ADTS, sizeof(ADTS));
    *pOutLen = recvLen - 16 + 7;
    unsigned char* bufTmp = (unsigned char*)bufIn;
    bool bFinishFrame = false;
    if (bufTmp[1] & 0x80)
    {
        //DebugTrace::D("Marker");
        bFinishFrame = true;
    }
    else
    {
        bFinishFrame = false;
    }
    return true;
}