#pragma once

/*
* This file contains all the D3D9 offsets for XP, Vista, Windows 7 (x86, x64)
*/

//x86 Versions for XP
const long XP_x86_OFFSET_DEVICE_PRESENT = 0x00040EA0;
const long XP_x86_OFFSET_SWAP_CHAIN_PRESENT = 0x00039230;
const long XP_x86_OFFSET_CLEAR = 0x00085720;
const long XP_x86_OFFSET_DEVICE_RESET = 0x000436B0;

//x64 Versions for XP
const long XP_x64_OFFSET_DEVICE_PRESENT = 0x000410A0;
const long XP_x64_OFFSET_SWAP_CHAIN_PRESENT = 0x00039300;
const long XP_x64_OFFSET_CLEAR = 0x00085A40;
const long XP_x64_OFFSET_DEVICE_RESET = 0x000438B0;

//x86 Versions for Win7
const long WIN7_x86_OFFSET_DEVICE_PRESENT = 0x0003DBA5;
const long WIN7_x86_OFFSET_SWAP_CHAIN_PRESENT = 0x00007AD5;
const long WIN7_x86_OFFSET_CLEAR = 0x0001F206;
const long WIN7_x86_OFFSET_DEVICE_RESET = 0x00051B4C;

//x64 bit Versions for Win7
const long WIN7_x64_OFFSET_DEVICE_PRESENT = 0x000410c3;
const long WIN7_x64_OFFSET_SWAP_CHAIN_PRESENT = 0x0000780E; 
const long WIN7_x64_OFFSET_CLEAR = 0x0000F244;
const long WIN7_x64_OFFSET_DEVICE_RESET = 0x00058DDA;

//x86 bit Versions for Vista
const long VISTA_x86_OFFSET_DEVICE_PRESENT = 0x0005589E;
const long VISTA_x86_OFFSET_SWAP_CHAIN_PRESENT = 0x00005200;
const long VISTA_x86_OFFSET_CLEAR = 0x0000E00D;
const long VISTA_x86_OFFSET_DEVICE_RESET = 0x000C7345;

//x64 bit Versions for Vista
const long VISTA_x64_OFFSET_DEVICE_PRESENT = 0x0005589E;
const long VISTA_x64_OFFSET_SWAP_CHAIN_PRESENT = 0x00005200;
const long VISTA_x64_OFFSET_CLEAR = 0x0000E00D;
const long VISTA_x64_OFFSET_DEVICE_RESET = 0x000C7345;

//opt codes for all 
unsigned char EXPECTED_OPCODES_DEVICE_PRESENT[5] = {0x8B, 0xFF, 0x55, 0x8B, 0xEC};
unsigned char EXPECTED_OPCODES_SWAP_CHAIN_PRESENT[5] = {0x8B, 0xFF, 0x55, 0x8B, 0xEC};
unsigned char EXPECTED_OPCODES_CLEAR[5] = {0x8B, 0xFF, 0x55, 0x8B, 0xEC};

void* address_DevicePresent = NULL;
void* address_SwapChainPresent = NULL;
void* address_Clear = NULL;
void* address_Reset = NULL;

unsigned char backup_Reset[5];
unsigned char patch_Reset[5];
unsigned char backup_DevicePresent[5];
unsigned char patch_DevicePresent[5];
unsigned char backup_SwapChainPresent[5];
unsigned char patch_SwapChainPresent[5];
unsigned char backup_Clear[5];
unsigned char patch_Clear[5];