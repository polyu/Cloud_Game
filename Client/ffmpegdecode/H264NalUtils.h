#pragma once
typedef struct  
{  
    unsigned char forbidden_bit;           //! Should always be FALSE  
    unsigned char nal_reference_idc;       //! NALU_PRIORITY_xxxx  
    unsigned char nal_unit_type;           //! NALU_TYPE_xxxx    
    unsigned int startcodeprefix_len;      //! 前缀字节数  
    unsigned int len;                      //! 包含nal 头的nal 长度，从第一个00000001到下一个000000001的长度  
    unsigned int max_size;                 //! 做多一个nal 的长度  
    unsigned char * buf;                   //! 包含nal 头的nal 数据  
    unsigned int lost_packets;             //! 预留  
} NALU_t;  
typedef struct   
{  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;  
    unsigned char F:1;          
} NALU_HEADER; // 1 BYTE 
typedef struct   
{  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;   
    unsigned char F:1;                
} FU_INDICATOR; // 1 BYTE  
typedef struct   
{  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char R:1;  
    unsigned char E:1;  
    unsigned char S:1;      
} FU_HEADER;   // 1 BYTES   


